//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCCPU_H_
#define _CPCCPU_H_


#include "cpcSubSystem.h"


namespace CPC {


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

                            CCpu                      (CMachine *pMachine);
    virtual                ~CCpu                      ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Requests a maskable interrupt. It returns true if it was accepted, or false otherwise (i.e. interrupts are disabled). */
    bool                    RequestInterrupt          ();

    /** Runs the CPU for the given number of cycles (or T-states, in Z80 terminology). */
    void                    Run                       (unsigned nNumCycles);


  private:

    typedef                 CSubSystem                inherited;

    // Type for 16-bit registers whose 8-bit components can be accessed as well.
    union Reg16
    {
      cpcWord word;
      struct
      {
        // Note: This is correct if the host is a low-endian machine.
        //       If the host is big-endian, the order of the low and high bytes should be reversed.

        // Low byte.
        union
        {
          cpcByte F;
          cpcByte C;
          cpcByte E;
          cpcByte L;
          cpcByte X;
          cpcByte Y;
          cpcByte R;
        };
        // High byte.
        union
        {
          cpcByte A;
          cpcByte B;
          cpcByte D;
          cpcByte H;
          cpcByte I;
        };
      } byte;
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
    };

    void                    ResetVars                 ();
    void                    FreeVars                  ();

    unsigned                FetchAndExecuteInstruction();
    void                    FetchOpcode               ();

    Registers m_registers;
    int m_activeGprSet;             // Index into the 'm_regs.gpr' array. Either 0 or 1.
    unsigned m_numCyclesAhead;      // How many clock cycles the Z80 emulation is ahead with respect to the rest of the emulator.
                                    // Instructions are fetched and executed at once, and this counter is incremented by the number of cycles the instruction should actually take.
                                    // Then the Z80 emulation sits idle until the emulator catches up with it.

  };


} //namespace CPC

#endif // _CPCCPU_H_
