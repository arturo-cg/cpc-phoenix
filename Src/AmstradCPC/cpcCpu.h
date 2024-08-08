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
    ** different clock cycles. This could be a problem in situations where very precise timing is needed.
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
    */
    class CCpu : public CSubSystem
    {
    public:

        // Opcode prefixes.
        enum Prefix : int
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

        enum InstructionTimingType
        {
            // F - Fetch cycle
            // M - Memory cycle
            // I - Internal operation cycle
            // E - External device input/output cycle

            TIMING_F4 = 0,
            TIMING_F5,
            TIMING_F6,
            TIMING_F4M3,
            TIMING_F4M4,
            TIMING_F4E4,
            TIMING_F5M3,
            TIMING_F4M3M3,
            TIMING_F4M3E4,
            TIMING_F4M4M3,
            TIMING_F4M3I5,
            TIMING_F4I4I3,
            TIMING_F4M3M5,
            TIMING_F5M3M3,
            TIMING_F5M3E4,
            TIMING_F5E4M3,
            TIMING_F4M3M3M3,
            TIMING_F4M3I4M3,
            TIMING_F4M3I5M3,
            TIMING_F4M3I5M4,
            TIMING_F5M3E4I5,
            TIMING_F5E4M3I5,
            TIMING_F4M3M3M3M3,
            TIMING_F4M3M4M3M3,
            TIMING_F4M3M4M3M5,
            TIMING_F4M3I5M4M3,
            TIMING_F4M3M3I5M3,

            INSTRUCTION_TIMING_COUNT
        };

        enum class MnemonicFlags
        {
            DisplacementBeforeOpcode = 1 << 0,          // Instruction has a 1-byte displacement right before the opcode.
            DisplacementAfterOpcode = 1 << 1,           // Instruction has a 1-byte displacement right after the opcode.
            Immediate8 = 1 << 2,                        // Instruction has a 1-byte immediate data at the end of the byte sequence.
            Immediate16 = 1 << 3,                       // Instruction has a 2-byte immediate data at the end of the byte sequence.
        };

        struct OpcodeDisassemblyInfo
        {
            const char* mnemonicOperation;              // The operation part of the mnemonic for this instruction.
            const char* mnemonicOperands;               // The operands art of the mnemonic for this instruction. Empty string if it has no operands.
            MnemonicFlags flags;
            int displacementTagPos;                   // Index of the '%' character for the displacement tag in the mnemonicOperands string.
            int immediateTagPos;                      // Index of the '%' character for the immediate data tag in the mnemonicOperands string.
        };

        struct OpcodeInfo
        {
            using MicrocodeFn = void (CCpu::*)();

            // Emulation data.
            bool isInstruction;                           // Prefixes are also included in the instruction look-up table. This variable is true iif this entry is for an instruction, or false if it's a prefix byte.
            InstructionTimingType timingType;             // Index into the timing table.
            MicrocodeFn microcodeFn;                      // Pointer to the function that contains the microde (i.e. the emulation) for the instruction.
            // Disassembly data.
            OpcodeDisassemblyInfo disassemblyInfo;        // Disassembly info.
        };

        struct AssemblyInstruction
        {
            int sizeBytes;                  // Size of the instruction, in bytes. It includes everything: prefix, opcode and operands.
            std::string operation;          // The operation, e.g. "LD", "OUT", etc.
            std::string operands;           // The operands. Empty if instruction has no operands.
        };


        CCpu(CMachine *pMachine);
        virtual                ~CCpu() { FreeVars(); }

        void                    SetCpuInterface(CCpuInterface* cpuInterface) { m_cpuInterface = cpuInterface; }
        CCpuInterface*          GetCpuInterface() { return m_cpuInterface; }
        const CCpuInterface*    GetCpuInterface() const { return m_cpuInterface; }

        /** Resets the subsystem. */
        virtual void            Reset();

        /** Sets the active state of the /INT signal (maskable interrupt request). */
        void                    SetInterruptRequestActive(bool active) { m_interruptRequestActive = active; }
        bool                    IsInterruptRequestActive() const { return m_interruptRequestActive; }
        /** Sets the interrupt vector, used by the Z80 in interrupt modes 0 and 2. */
        void                    SetInterruptVector(cpcByte interruptVector) { m_interruptVector = interruptVector; }
        cpcByte                 GetInterruptVector() const { return m_interruptVector; }
        /** Requests an NMI (non-maskable interrupt. */
        void                    RequestNmi() { m_nmiRequested = true; }
        bool                    IsNmiRequested() const { return m_nmiRequested; }
        /** Sets the active state of the /WAIT signal. */
        void                    SetWaitActive(bool active) { m_waitActive = active; }
        bool                    IsWaitActive() const { return m_waitActive; }

        /** Runs the CPU for the given number of cycles (or T-states, in Z80 terminology). */
        void                    Run(unsigned nNumCycles);

        /** Returns true if the CPU is in the middle of the execution of an instruction, or if it just read a prefix but not yet the opcode.
        *** Returns false if it has just finished executing an instruction and is about to start the next one. */
        bool                    IsExecutingInstruction() const;
        /** Returns true if the CPU acknowledged an interrupt during the last call to Run, or false otherwise. */
        bool                    InterruptWasAcknowledged() const { return m_interruptWasAcknowledged; }

        /** Translates the instruction that starts at the specified address to assembly language.
        *   It provides the core functionality for a full-blown disassembler.
        */
        void                    DisassembleInstruction(cpcWord address, AssemblyInstruction* outResult) const;
        /** Translates the instruction that starts at the specified address to assembly language.
        *   It provides the core functionality for a full-blown disassembler.
        *   This version uses the specified function to read instruction bytes from memory.
        */
        static void             DisassembleInstruction(cpcWord address, std::function<cpcByte(cpcWord)> ReadByteFromMemory, AssemblyInstruction* outResult);

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

        // All of the Z80 registers.
        struct Registers
        {
            Reg16 AF;               // Active AF register.
            Reg16 BC;               // Active BC register.
            Reg16 DE;               // Active DE register.
            Reg16 HL;               // Active HL register.
            Reg16 altAF;            // Alternate (hidden) AF register.
            Reg16 altBC;            // Alternate (hidden) BC register.
            Reg16 altDE;            // Alternate (hidden) DE register.
            Reg16 altHL;            // Alternate (hidden) HL register.
            Reg16 IX;               // Index register IX.
            Reg16 IY;               // Index register IY.
            Reg16 PC;               // Program Counter.
            Reg16 SP;               // Stack Pointer.
            Reg16 IR;               // Interrupt Vector / Memory Refresh.
            bool IFF1;              // Interrupt flip-flop 1.
            bool IFF2;              // Interrupt flip-flop 2.
            int IM;                 // Interrupt Mode.

            // 8-bit register accessor functions.
            cpcByte& A() { return AF.b.h; }
            cpcByte& F() { return AF.b.l; }
            cpcByte& B() { return BC.b.h; }
            cpcByte& C() { return BC.b.l; }
            cpcByte& D() { return DE.b.h; }
            cpcByte& E() { return DE.b.l; }
            cpcByte& H() { return HL.b.h; }
            cpcByte& L() { return HL.b.l; }
            cpcByte& altA() { return altAF.b.h; }
            cpcByte& altF() { return altAF.b.l; }
            cpcByte& altB() { return altBC.b.h; }
            cpcByte& altC() { return altBC.b.l; }
            cpcByte& altD() { return altDE.b.h; }
            cpcByte& altE() { return altDE.b.l; }
            cpcByte& altH() { return altHL.b.h; }
            cpcByte& altL() { return altHL.b.l; }
            cpcByte& I() { return IR.b.h; }
            cpcByte& R() { return IR.b.l; }
            cpcByte& IXH() { return IX.b.h; }
            cpcByte& IXL() { return IX.b.l; }
            cpcByte& IYH() { return IY.b.h; }
            cpcByte& IYL() { return IY.b.l; }
            const cpcByte& A() const { return AF.b.h; }
            const cpcByte& F() const { return AF.b.l; }
            const cpcByte& B() const { return BC.b.h; }
            const cpcByte& C() const { return BC.b.l; }
            const cpcByte& D() const { return DE.b.h; }
            const cpcByte& E() const { return DE.b.l; }
            const cpcByte& H() const { return HL.b.h; }
            const cpcByte& L() const { return HL.b.l; }
            const cpcByte& altA() const { return altAF.b.h; }
            const cpcByte& altF() const { return altAF.b.l; }
            const cpcByte& altB() const { return altBC.b.h; }
            const cpcByte& altC() const { return altBC.b.l; }
            const cpcByte& altD() const { return altDE.b.h; }
            const cpcByte& altE() const { return altDE.b.l; }
            const cpcByte& altH() const { return altHL.b.h; }
            const cpcByte& altL() const { return altHL.b.l; }
            const cpcByte& I() const { return IR.b.h; }
            const cpcByte& R() const { return IR.b.l; }
            const cpcByte& IXH() const { return IX.b.h; }
            const cpcByte& IXL() const { return IX.b.l; }
            const cpcByte& IYH() const { return IY.b.h; }
            const cpcByte& IYL() const { return IY.b.l; }
            // Flag accessor functions.
            enum Flag
            {
                Flag_S = 7,     // Sign
                Flag_Z = 6,     // Zero
                Flag_5 = 5,     // Undocumented - Copy of bit 5
                Flag_H = 4,     // Half carry
                Flag_3 = 3,     // Undocumented - Copy of bit 3
                Flag_PV = 2,    // Parity/Overflow
                Flag_N = 1,     // Subtraction
                Flag_C = 0,     // Carry
            };
            void SetFlag(Flag flag, bool state) { AF.b.l = (state ? AF.b.l | (1 << flag) : AF.b.l & ~(1 << flag)); }
            bool GetFlag(Flag flag) const { return ((AF.b.l & (1 << flag)) != 0); }
        };

        /** Sets the Z80's internal registers. */
        void                    SetRegisters(const Registers& newRegisters) { m_registers = newRegisters; }
        /** Returns the Z80's internal registers. */
        const Registers&        GetRegisters() const { return m_registers; }

    private:

        typedef                 CSubSystem                inherited;

        enum MCycleType
        {
            MCYCLE_FETCH = 0,       // Opcode fetch.
            MCYCLE_MEM,             // Memory read/write.
            MCYCLE_IO,              // External device input/output.
            MCYCLE_INTERNAL         // Internal operation.
        };

        static const int MAX_MCYCLES = 5;
        struct InstructionTiming
        {
            int tstates[MAX_MCYCLES];               // T states per M cycle. 0 indicates that the instruction doesn't use this M cycle.
            MCycleType mcycleTypes[MAX_MCYCLES];    // Type of each M cycle (fetch, memory r/w, I/O or internal operation).
        };

        class StaticInitializer
        {
        public:
            StaticInitializer();
        };

        static StaticInitializer s_staticInitializer;
        static bool s_parity[256];      // True = even, false = odd.
        static InstructionTiming CCpu::s_instructionTimings[INSTRUCTION_TIMING_COUNT];


        void                    ResetVars();
        void                    FreeVars();

        void                    Step();
        void                    StepOpcode(cpcByte opcode);
        cpcByte                 FetchByte();
        void                    IncrementR();
        void                    AcceptNmi();
        void                    AcceptInterrupt();

        void                    DoInstructionTiming(const InstructionTiming& instructionTiming);
        void                    DoMCycleTiming(int tstates, CCpu::MCycleType mcycleType);
        void                    ConsumeTStates(int tstates);
        void                    ConsumeTStatesWithWait(int tstates, int when);

        void                    ProcessPrefixByte(cpcByte prefixByte);
        void                    HandleInvalidInstruction();

        cpcByte                 ReadByteFromMemory(cpcWord address);
        void                    WriteByteToMemory(cpcWord address, cpcByte value);
        cpcByte                 ReadByteFromPort(cpcWord address);
        void                    WriteByteToPort(cpcWord address, cpcByte value);

        void                    Push(const Reg16& value);
        void                    Pop(Reg16* value);

        static cpcWord          ConvertSignedByteToWord(cpcByte value);

        void                    LD8_reg_reg(cpcByte* dest, cpcByte value);
        void                    LD8_reg_n(cpcByte* byte);
        void                    LD8_reg_mem(cpcByte* byte, const Reg16& addressReg);
        void                    LD8_addrreg_valuereg(const Reg16& addressReg, cpcByte value);
        void                    LD8_addrreg_n(const Reg16& addressReg);
        void                    LD8_addrnn_reg(cpcByte value);
        void                    LD8_reg_addrnn(cpcByte* dest);
        void                    LD8_reg_addrreg(cpcByte* dest, const Reg16& addressReg);
        void                    LD8_addrreg_offset_n(const Reg16& addressReg);
        void                    LD8_addrreg_offset_valuereg(const Reg16& addressReg, cpcByte value);
        void                    LD8_reg_addrreg_offset(cpcByte* dest, const Reg16& addressReg);
        void                    LD8_IorR(cpcByte value);
        void                    LDI();
        void                    LDIR();
        void                    LDD();
        void                    LDDR();
        void                    LD16_reg_reg(Reg16* dest, const Reg16& src);
        void                    LD16_reg_nn(Reg16* reg);
        void                    LD16_addrnn_reg(const Reg16& value);
        void                    LD16_reg_addrnn(Reg16* reg);
        void                    ADD8_reg_reg(cpcByte* a, cpcByte b, cpcByte carry);
        void                    ADD8_reg_n(cpcByte* a, cpcByte carry);
        void                    ADD8_reg_addrreg(cpcByte* a, const Reg16& addressReg, cpcByte carry);
        void                    ADD8_reg_addrreg_offset(cpcByte* a, const Reg16& addressReg, cpcByte carry);
        void                    ADD16_reg_reg(Reg16* a, Reg16 b);
        void                    ADC16_reg_reg(Reg16* a, Reg16 b);
        void                    SUB8_reg(cpcByte b, cpcByte borrow);
        void                    SUB8_n(cpcByte borrow);
        void                    SUB8_addrreg(const Reg16& addressReg, cpcByte borrow);
        void                    SUB8_addrreg_offset(const Reg16& addressReg, cpcByte borrow);
        void                    SBC16_reg(Reg16 b);
        void                    NEG();
        void                    INC8_reg(cpcByte* byte);
        void                    DEC8_reg(cpcByte* byte);
        void                    INC8_addrreg(const Reg16& addressReg);
        void                    DEC8_addrreg(const Reg16& addressReg);
        void                    INC8_addrreg_offset(const Reg16& addressReg);
        void                    DEC8_addrreg_offset(const Reg16& addressReg);
        void                    INC16_reg(Reg16* reg);
        void                    DEC16_reg(Reg16* reg);
        void                    AND_reg(cpcByte b);
        void                    AND_n();
        void                    AND_addrreg(const Reg16& addressReg);
        void                    AND_addrreg_offset(const Reg16& addressReg);
        void                    OR_reg(cpcByte b);
        void                    OR_n();
        void                    OR_addrreg(const Reg16& addressReg);
        void                    OR_addrreg_offset(const Reg16& addressReg);
        void                    XOR_reg(cpcByte b);
        void                    XOR_n();
        void                    XOR_addrreg(const Reg16& addressReg);
        void                    XOR_addrreg_offset(const Reg16& addressReg);
        void                    CPL();
        void                    DAA();
        void                    RL_result_value(cpcByte* result, cpcByte value);
        void                    RL_reg(cpcByte* byte);
        void                    RL_addrreg(const Reg16& addressReg);
        void                    RL_addrreg_offset(const Reg16& addressReg);
        void                    RL_addrreg_offset_reg(const Reg16& addressReg, cpcByte* result);
        void                    RLC_result_value(cpcByte* result, cpcByte value);
        void                    RLC_reg(cpcByte* byte);
        void                    RLC_addrreg(const Reg16& addressReg);
        void                    RLC_addrreg_offset(const Reg16& addressReg);
        void                    RLC_addrreg_offset_reg(const Reg16& addressReg, cpcByte* result);
        void                    RLA();
        void                    RLCA();
        void                    RLD();
        void                    RR_result_value(cpcByte* result, cpcByte value);
        void                    RR_reg(cpcByte* byte);
        void                    RR_addrreg(const Reg16& addressReg);
        void                    RR_addrreg_offset(const Reg16& addressReg);
        void                    RR_addrreg_offset_reg(const Reg16& addressReg, cpcByte* result);
        void                    RRC_result_value(cpcByte* result, cpcByte value);
        void                    RRC_reg(cpcByte* byte);
        void                    RRC_addrreg(const Reg16& addressReg);
        void                    RRC_addrreg_offset(const Reg16& addressReg);
        void                    RRC_addrreg_offset_reg(const Reg16& addressReg, cpcByte* result);
        void                    RRA();
        void                    RRCA();
        void                    RRD();
        void                    SL_result_value(cpcByte* result, cpcByte value, bool bit0);
        void                    SL_reg(cpcByte* byte, bool bit0);
        void                    SL_addrreg(const Reg16& addressReg, bool bit0);
        void                    SL_addrreg_offset(const Reg16& addressReg, bool bit0);
        void                    SL_addrreg_offset_reg(const Reg16& addressReg, cpcByte* result, bool bit0);
        void                    SR_result_value(cpcByte* result, cpcByte value, bool bit7);
        void                    SR_reg(cpcByte* byte, bool bit7);
        void                    SR_addrreg_offset(const Reg16& addressReg, bool bit7);
        void                    SR_addrreg_offset_reg(const Reg16& addressReg, cpcByte* result, bool bit7);
        void                    SRA_addrreg(const Reg16& addressReg);
        void                    SRA_addrreg_offset(const Reg16& addressReg);
        void                    SRL_addrreg(const Reg16& addressReg);
        void                    SRL_addrreg_offset(const Reg16& addressReg);
        void                    EX_reg_reg(Reg16* a, Reg16* b);
        void                    EX_addrreg_reg(Reg16 addressReg, Reg16* b);
        void                    EXX();
        void                    SCF();
        void                    CCF();
        void                    CP_reg(cpcByte b);
        void                    CP_n();
        void                    CP_addrreg(const Reg16& addressReg);
        void                    CP_addrreg_offset(const Reg16& addressReg);
        void                    CPI();
        void                    CPIR();
        void                    CPD();
        void                    CPDR();
        void                    BIT_reg(int bit, cpcByte value);
        void                    BIT_addr(int bit, cpcWord address);
        void                    BIT_addr_offset(int bit, cpcWord address);
        void                    RES_reg(int bit, cpcByte* value);
        void                    RES_addr(int bit, cpcWord address);
        void                    RES_addr_offset(int bit, cpcWord address);
        void                    RES_addr_offset_reg(int bit, cpcWord address, cpcByte* result);
        void                    SET_reg(int bit, cpcByte* value);
        void                    SET_addr(int bit, cpcWord address);
        void                    SET_addr_offset(int bit, cpcWord address);
        void                    SET_addr_offset_reg(int bit, cpcWord address, cpcByte* result);
        void                    PUSH(const Reg16& value);
        void                    POP(Reg16* value);
        void                    CALL_nn();
        void                    CALL_condition_nn(bool condition);
        void                    RST_p(cpcByte p);
        void                    RET();
        void                    RET_condition(bool condition);
        void                    RETN();
        void                    JP_reg(const Reg16& address);
        void                    JP_nn();
        void                    JP_condition_nn(bool condition);
        void                    JR_n();
        void                    JR_condition_n(bool condition);
        void                    DJNZ_n();
        void                    IN_value_address(cpcByte* value, const Reg16& addressReg);
        void                    IN_n();
        void                    INI();
        void                    INIR();
        void                    IND();
        void                    INDR();
        void                    OUT_address_value(const Reg16& addressReg, cpcByte value);
        void                    OUT_n();
        void                    OUTI();
        void                    OTIR();
        void                    OUTD();
        void                    OTDR();
        void                    EI();
        void                    DI();
        void                    IM(int mode);
        void                    HALT();

        static void             FillOpcodeDisassemblyInfo(OpcodeInfo* opcodeTable, bool twoBytePrefixInstructions);

        Registers m_registers;
        bool m_inHalt;
        bool m_delayInterruptEnable;
        bool m_interruptRequestActive;
        bool m_interruptWasAcknowledged;
        cpcByte m_interruptVector;
        bool m_nmiRequested;
        bool m_waitActive;
        int m_numCyclesAhead;      // How many clock cycles the Z80 emulation is ahead with respect to the rest of the emulator.
                                   // When an instruction is fetched and executed, this counter is incremented by the number of cycles the instruction actually takes.
                                   // If that number of cycles is higher than the number of cycles the Z80 emulation was asked to execute, it will sit idle until the emulator catches up with it.
        Prefix m_prefix;
        cpcWord m_signedDisplacement;
        CCpuInterface* m_cpuInterface;

        static OpcodeInfo* m_opcodes[Prefix::Count];     // One InstructionTable per prefix.

        static OpcodeInfo m_opcodesMain[256];
        static OpcodeInfo m_opcodesED[256];
        static OpcodeInfo m_opcodesCB[256];
        static OpcodeInfo m_opcodesDD[256];
        static OpcodeInfo m_opcodesDDCB[256];
        static OpcodeInfo m_opcodesFD[256];
        static OpcodeInfo m_opcodesFDCB[256];

        // Define all the opcodes' micro-code functions.
#define Z80_OPCODE(_num, _isPrefix, _mnemonicOperation, _mnemonicOperands, _timingType, _microCode) \
    void Execute_##_num() \
        _microCode
#include "cpcCpu_MainOpcodes.h"
#undef Z80_OPCODE

#define Z80_OPCODE(_num, _isPrefix, _mnemonicOperation, _mnemonicOperands, _timingType, _microCode) \
    void Execute_ED##_num() \
        _microCode
#include "cpcCpu_OpcodesED.h"
#undef Z80_OPCODE

#define Z80_OPCODE(_num, _isPrefix, _mnemonicOperation, _mnemonicOperands, _timingType, _microCode) \
    void Execute_CB##_num() \
        _microCode
#include "cpcCpu_OpcodesCB.h"
#undef Z80_OPCODE

#define Z80_OPCODE(_num, _isPrefix, _mnemonicOperation, _mnemonicOperands, _timingType, _microCode) \
    void Execute_DD##_num() \
        _microCode
#include "cpcCpu_OpcodesDD.h"
#undef Z80_OPCODE

#define Z80_OPCODE(_num, _isPrefix, _mnemonicOperation, _mnemonicOperands, _timingType, _microCode) \
    void Execute_DDCB##_num() \
        _microCode
#include "cpcCpu_OpcodesDDCB.h"
#undef Z80_OPCODE

#define Z80_OPCODE(_num, _isPrefix, _mnemonicOperation, _mnemonicOperands, _timingType, _microCode) \
    void Execute_FD##_num() \
        _microCode
#include "cpcCpu_OpcodesFD.h"
#undef Z80_OPCODE

#define Z80_OPCODE(_num, _isPrefix, _mnemonicOperation, _mnemonicOperands, _timingType, _microCode) \
    void Execute_FDCB##_num() \
        _microCode
#include "cpcCpu_OpcodesFDCB.h"
#undef Z80_OPCODE

    };


} //namespace CPC

#endif // _CPCCPU_H_
