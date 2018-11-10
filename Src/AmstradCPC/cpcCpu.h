//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCCPU_H_
#define _CPCCPU_H_


#include "cpcSubSystem.h"


namespace CPC {

  class CCpuInterface;


  /**
  ** It emulates the Zilog Z80A CPU.
  **
  ** Caveat about this implementation:
  **
  ** From the point of view of the other devices in the machine, instructions are executed atomically.
  ** All the bytes that make up an instruction (prefix, opcode, operand) are read from memory on the same clock cycle
  ** and the result is written to memory on that same cycle, whereas a real Z80 CPU reads and writes each byte sequentially on
  ** different clock cycles. This could be a problem in which extremely precise timing is needed.
  **
  ** On an Amstrad CPC emulator it's *mostly* OK because the CPU is the only device that can write to memory, so once the CPU
  ** starts reading the bytes of the next instruction it's guaranteed that all of them are going to remain unchanged. Therefore,
  ** it's OK for the emulator to read all these bytes at once.
  **
  ** However, writing the result to memory too early could be a problem (not tested but it would nice to do so). For example,
  ** imagine a piece of code that modifies the byte at the same address as the Gate-Array is reading pixel data from. On a real CPC
  ** the Gate-Array would read the old byte value and then the CPU would write the new value, because by the time the Gate-Array reads
  ** the memory the CPU is still fetching the opcode; when the CPU is done fetching the opcode and is about to write the new byte value,
  ** the Gate-Array has already moved on to a higher address. On this emulator on the other hand it would be the other way around; the CPU
  ** would write the new byte value first and then the Gate-Array would read the byte, which has already been changed.
  **
  */
  class CCpu : public CSubSystem
  {
  public:

    struct OpcodeInfo
    {
      using MicrocodeFn = void (CCpu::*)();

      MicrocodeFn microcodeFn;                      // Pointer to the function that contains the microde (i.e. the emulation) for the instruction.
      bool isPrefix;                                // Prefixes are also included in the instruction look-up table. This variable is true iif this entry is for a prefix byte rather than an instruction.
      //int numTStates;                               // The T states that this instruction takes.
      const char* mnemonic;                         // The mnemonic for this instruction.
    };

                            CCpu                      (CMachine *pMachine);
    virtual                ~CCpu                      ()  { FreeVars(); }

    void                    SetCpuInterface           (CCpuInterface* cpuInterface)  { m_cpuInterface = cpuInterface; }
    CCpuInterface*          GetCpuInterface           ()        { return m_cpuInterface; }
    const CCpuInterface*    GetCpuInterface           () const  { return m_cpuInterface; }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Requests a maskable interrupt. It returns true if it was accepted, or false otherwise (i.e. interrupts are disabled). */
    bool                    RequestInterrupt          ();
    /** Sets the active state of the /WAIT signal. */
    void                    SetWaitSignalActive       (bool active)  { m_waitActive = active; }
    bool                    IsWaitSignalActive        () const       { return m_waitActive; }

    /** Runs the CPU for the given number of cycles (or T-states, in Z80 terminology). */
    void                    Run                       (unsigned nNumCycles);


  private:

    typedef                 CSubSystem                inherited;

    // Type for 16-bit registers whose 8-bit components can be accessed individually as well.
    union Reg16
    {
      cpcWord w;
      struct
      {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        cpcByte l;
        cpcByte h;
#else
        cpcByte h;
        cpcByte l;
#endif
      } b;
    };

    // General-purpose registers.
    struct GPR
    {
        Reg16 AF;
        Reg16 BC;
        Reg16 DE;
        Reg16 HL;
    };

    // All of the Z80 registers.
    struct Registers
    {
        GPR gpr[2];     // Two sets of general-purpose registers.
        Reg16 IX;       // Index register IX.
        Reg16 IY;       // Index register IY.
        Reg16 PC;       // Program Counter.
        Reg16 SP;       // Stack Pointer.
        Reg16 IR;       // Interrupt Vector / Memory Refresh.
        bool IFF1;      // Interrupt flip-flop 1.
        bool IFF2;      // Interrupt flip-flop 2.
        int IM;         // Interrupt Mode.

        // Handy methods to access general-purpose registers.
        Reg16& AF(int activeSet) { return gpr[activeSet].AF; }
        const Reg16& AF(int activeSet) const { return gpr[activeSet].AF; }
        Reg16& BC(int activeSet) { return gpr[activeSet].BC; }
        const Reg16& BC(int activeSet) const { return gpr[activeSet].BC; }
        Reg16& DE(int activeSet) { return gpr[activeSet].DE; }
        const Reg16& DE(int activeSet) const { return gpr[activeSet].DE; }
        Reg16& HL(int activeSet) { return gpr[activeSet].HL; }
        const Reg16& HL(int activeSet) const { return gpr[activeSet].HL; }

        cpcByte& A(int activeSet) { return gpr[activeSet].AF.b.h; }
        const cpcByte& A(int activeSet) const { return gpr[activeSet].AF.b.h; }
        cpcByte& F(int activeSet) { return gpr[activeSet].AF.b.l; }
        const cpcByte& F(int activeSet) const { return gpr[activeSet].AF.b.l; }
        cpcByte& B(int activeSet) { return gpr[activeSet].BC.b.h; }
        const cpcByte& B(int activeSet) const { return gpr[activeSet].BC.b.h; }
        cpcByte& C(int activeSet) { return gpr[activeSet].BC.b.l; }
        const cpcByte& C(int activeSet) const { return gpr[activeSet].BC.b.l; }
        cpcByte& D(int activeSet) { return gpr[activeSet].DE.b.h; }
        const cpcByte& D(int activeSet) const { return gpr[activeSet].DE.b.h; }
        cpcByte& E(int activeSet) { return gpr[activeSet].DE.b.l; }
        const cpcByte& E(int activeSet) const { return gpr[activeSet].DE.b.l; }
        cpcByte& H(int activeSet) { return gpr[activeSet].HL.b.h; }
        const cpcByte& H(int activeSet) const { return gpr[activeSet].HL.b.h; }
        cpcByte& L(int activeSet) { return gpr[activeSet].HL.b.l; }
        const cpcByte& L(int activeSet) const { return gpr[activeSet].HL.b.l; }
    };

    // Opcode prefixes.
    enum Prefix
    {
      None = 0,         // No prefix: main instructions.
      ED,               // Extended instructions.
      CB,               // Bit instructions.
      DD,               // IX instructions.
      DDCB,             // IX bit instructions.
      FD,               // IY instructions.
      FDCB,             // IY bit instructions.

      Count
    };

    void                    ResetVars                 ();
    void                    FreeVars                  ();

    void                    Step                      ();
    cpcByte                 FetchByte                 ();
    //void                    AdvanceTStates            (int numTStates);
    //void                    SyncToWaitSignal          ();

    cpcByte                 ReadByteFromMemory        (cpcWord address);
    void                    WriteByteToMemory         (cpcWord address, cpcByte value);

    void                    LD8_addrreg_valuereg      (const Reg16& addressReg, cpcByte value);
    void                    LD16_reg_nn               (Reg16* reg);

    void                    Execute_00                ();
    void                    Execute_01                ();
    void                    Execute_02                ();
    void                    Execute_03                ();
    void                    Execute_04                ();
    void                    Execute_05                ();
    void                    Execute_06                ();
    void                    Execute_07                ();
    void                    Execute_08                ();
    void                    Execute_09                ();

    Registers m_registers;
    int m_activeGprSet;             // Index into the 'm_regs.gpr' array. Either 0 or 1.
    bool m_waitActive;
    unsigned m_numCyclesAhead;      // How many clock cycles the Z80 emulation is ahead with respect to the rest of the emulator.
                                    // When an instruction is fetched and executed, this counter is incremented by the number of cycles the instruction actually takes.
                                    // If that number of cycles is higher than the number of cycles the Z80 emulation was asked to execute, it will sit idle until the emulator catches up with it.
    Prefix m_prefix;
    OpcodeInfo* m_opcodes[Prefix::Count];     // One InstructionTable per prefix.
    CCpuInterface* m_cpuInterface;

    static OpcodeInfo m_opcodesMain[256];
    static OpcodeInfo m_opcodesED[256];
    static OpcodeInfo m_opcodesCB[256];
    static OpcodeInfo m_opcodesDD[256];
    static OpcodeInfo m_opcodesDDCB[256];
    static OpcodeInfo m_opcodesFD[256];
    static OpcodeInfo m_opcodesFDCB[256];

  };


} //namespace CPC

#endif // _CPCCPU_H_
