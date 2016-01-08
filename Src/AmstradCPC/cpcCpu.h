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


    void                    ResetVars                 ();
    void                    FreeVars                  ();

  };


} //namespace CPC

#endif // _CPCCPU_H_
