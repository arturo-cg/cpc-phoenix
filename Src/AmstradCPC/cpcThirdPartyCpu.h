//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCCPU_H_
#define _CPCCPU_H_


#include "cpcSubSystem.h"
#include "z80ex.h"


namespace CPC {


  /**
  ** 
  */
  class CThirdPartyCpu : public CSubSystem
  {
    friend Z80EX_BYTE CB_ReadByteFromMemory(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, int m1_state, void *user_data);
    friend void CB_WriteByteToMemory(Z80EX_CONTEXT *cpu, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data);
    friend Z80EX_BYTE CB_ReadByteFromPort(Z80EX_CONTEXT *cpu, Z80EX_WORD port, void *user_data);
    friend void CB_WriteByteToPort(Z80EX_CONTEXT *cpu, Z80EX_WORD port, Z80EX_BYTE value, void *user_data);
    friend Z80EX_BYTE CB_ReadByteFromInterruptVector(Z80EX_CONTEXT *cpu, void *user_data);

  public:

                            CThirdPartyCpu            (CMachine *pMachine);
    virtual                ~CThirdPartyCpu            ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Requests a maskable interrupt. It returns true if it was accepted, or false otherwise (i.e. interrupts are disabled). */
    bool                    RequestInterrupt          ();

    /** Runs the CPU for the given number of cycles. */
    void                    Run                       (unsigned nMinNumCycles);


  private:

    typedef                 CSubSystem                inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    /** Returns true if the last call to CThirdPartyCpu::Step executed a complete instruction,
    *** or false if only its prefix was executed. */
    bool                    WasCompleteInstructionLastStep () const  { return m_bCompleteInstructionLastStep; }

    cpcByte                 ReadByteFromMemory        (cpcWord nAddr) const;
    void                    WriteByteToMemory         (cpcWord nAddr, cpcByte nValue);
    cpcByte                 ReadByteFromPort          (cpcWord nPort);
    void                    WriteByteToPort           (cpcWord nPort, cpcByte nValue);
    cpcByte                 ReadByteFromInterruptVector () const;


    /** Internal CPU state (registers, interrupt mode, etc.). */
    Z80EX_CONTEXT          *m_pZ80State;

    bool                    m_bCompleteInstructionLastStep;
    unsigned                m_nNumSpareCycles;

  };


} //namespace CPC

#endif // _CPCCPU_H_
