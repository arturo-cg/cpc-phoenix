//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCKEYBOARD_H_
#define _CPCKEYBOARD_H_


#include "cpcSubSystem.h"
#include "cpcKeyboardTypes.h"


namespace CPC {

  class CKeyStateProvider;


  /**
  ** This class represents the Amstrad CPC keyboard. It keeps track of the state of all keys and joysticks.
  */
  class CKeyboard : public CSubSystem
  {
  public:


                            CKeyboard                 (CMachine *pMachine, CKeyStateProvider* pKeyStateProvider);
    virtual                ~CKeyboard                 ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Selects the specified keyboard line. This is called by the emulator whenever PPI port C changes. */
    void                    SetSelectedLine           (int nLine);


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


    unsigned                m_nSelectedLine;
    CKeyStateProvider*      m_pKeyStateProvider;

  };


} //namespace CPC

#endif // _CPCKEYBOARD_H_
