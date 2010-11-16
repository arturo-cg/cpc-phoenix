//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCKEYSTATEPROVIDER_H_
#define _CPCKEYSTATEPROVIDER_H_


#include "cpcKeyboardTypes.h"


namespace CPC {


  /**
  ** This is the base class for providers of key states. The front end must inherit from this class and pass it to the
  ** cpcKeyboard class constructor. Once done, the emulator will make calls to the method CKeyStateProvider::GetKeyState
  ** each time the emulated machine needs to know the state of a particular key in the emulated keyboard.
  */
  class CKeyStateProvider
  {
  public:

    /** Called by the emulator to know the current released/pressed state of the given CPC key. */
    virtual ECpcKeyState    GetKeyState               (ECpcKey eCpcKey) = 0;

  };


} //namespace CPC

#endif // _CPCKEYSTATEPROVIDER_H_
