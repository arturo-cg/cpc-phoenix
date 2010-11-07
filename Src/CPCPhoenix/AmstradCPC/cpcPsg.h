//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCPSG_H_
#define _CPCPSG_H_


#include "cpcSubSystem.h"


namespace CPC {


  /**
  ** The AY-3-8912 Programmable Sound Generator (PSG), made by General Instruments, is the responsible for generating sound in the Amstrad CPC.
  ** In addition, the CPC uses the PSG to scan the keyboard matrix to read the state of the keyboard and joysticks.
  **
  ** In the Amstrad CPC, The AY-3-8912 PSG is accessed through the 8255 PPI.
  **
  ** For now, the emulation provided by this class is limited to keyboard-related features; no sound-related features are emulated in any way.
  */
  class CPsg : public CSubSystem
  {
  public:

    enum EFunction
    {
      FUNCTION_INACTIVE        = 0x00,
      FUNCTION_READ_REGISTER   = 0x01,    // Read from the selected PSG register. The register content is then readable from PPI port A.
      FUNCTION_WRITE_REGISTER  = 0x02,    // Write to the selected PSG register. The register content is taken from PPI port A.
      FUNCTION_SELECT_REGISTER = 0x03,    // Selects the specified PSG register. The register to select is taken from PPI port A.

      FUNCTION_LAST,
      FUNCTION_INVALID = 0x7FFFFFFF
    };

    static const unsigned   IO_PORT_A_REGISTER = 14;     // PSG register index that holds data about the PSG I/O Port A (in the CPC it is connected to the selected keyboard matrix).
                                                         // Note: According to PSG datasheet, I/O Port A uses register 16. Probably the CPC has some logic
                                                         //       that makes that, when a program selects register 14, register 16 is actually selected (and yes,
                                                         //       this would have the side effect of hiding the actual register 14 (Envelope Coarse Tune) to the CPU.


                            CPsg                      (CMachine *pMachine);
    virtual                ~CPsg                      ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Selects the PSG function to perform. */
    void                    SelectFunction            (EFunction eFunction);

    /** Writes a new value into the currently selected register. */
    void                    SetSelectedRegisterValue  (cpcByte nValue)                 { m_anRegisters[m_nSelectedRegister] = nValue; }
    /** Writes a new value into the specified register.
    *** This is used by CKeyboard each time a new keyboard matrix line is selected or a key is pressed or released. */
    void                    SetSelectedRegisterValue  (int nRegister, cpcByte nValue)  { m_anRegisters[nRegister] = nValue; }
    /** Returns the value of the currently selected value. */
    cpcByte                 GetSelectedRegisterValue  () const                         { return m_anRegisters[m_nSelectedRegister]; }


  private:

    typedef                 CSubSystem                inherited;

    static const unsigned   REGISTER_COUNT     = 16;     // (Read note about I/O Port A above)

    void                    ResetVars                 ();
    void                    FreeVars                  ();


    unsigned                m_nSelectedRegister;
    cpcByte                 m_anRegisters[REGISTER_COUNT];

  };


} //namespace CPC

#endif // _CPCPSG_H_
