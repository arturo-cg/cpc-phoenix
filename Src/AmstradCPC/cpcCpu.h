//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCCPU_H_
#define _CPCCPU_H_


#include "cpcSubSystem.h"


namespace CPC {


  /**
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

    /** Runs the CPU for the given number of cycles. */
    void                    Run                       (unsigned nMinNumCycles);


  private:

    typedef                 CSubSystem                inherited;

    // Type for 16-bit registers whose 8-bit components can be accessed as well.
    union Reg16
    {
      cpcWord word;
      struct
      {
        union
        {
          cpcByte A;
          cpcByte B;
          cpcByte D;
          cpcByte H;
          cpcByte I;
        };

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


    Registers m_registers;
    int m_activeGprSet;     // Index into the 'm_regs.gpr' array. Either 0 or 1.
  };


} //namespace CPC

#endif // _CPCCPU_H_
