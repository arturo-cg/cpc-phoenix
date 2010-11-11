//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCKEYBOARD_H_
#define _CPCKEYBOARD_H_


#include "cpcSubSystem.h"
#include "cpcKeyboardTypes.h"


namespace CPC {


  /**
  ** This class represents the Amstrad CPC keyboard. It keeps track of the state of all keys and joysticks.
  */
  class CKeyboard : public CSubSystem
  {
  public:


                            CKeyboard                 (CMachine *pMachine);
    virtual                ~CKeyboard                 ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Selects the specified keyboard line. This is called by the emulator whenever PPI port C changes. */
    void                    SetSelectedLine           (int nLine);

    /** Sets the released/pressed state of a specific key.
    *** This must be called by the emulator front end when a key in the host keyboard is released or pressed. */
    void                    SetKeyState               (ECpcKey eKey, ECpcKeyState eState);


  private:

    typedef                 CSubSystem                inherited;

    static const unsigned   LINE_COUNT    = 10;
    static const unsigned   KEYS_PER_LINE = 8;
    static const ECpcKey    s_aeKeyboardMatrix[KEYS_PER_LINE][LINE_COUNT];
    static unsigned         s_anKeyToLineLookUp[CPCKEY_LAST];


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    static void             InitKeyToLineLookUpTable  ();

    cpcByte                 GetSelectedLineStatus     () const;
    void                    WriteSelectedLineStatusToPsg ();

    ECpcKeyState            m_aeKeyStates[CPCKEY_LAST];
    unsigned                m_nSelectedLine;

  };


} //namespace CPC

#endif // _CPCKEYBOARD_H_
