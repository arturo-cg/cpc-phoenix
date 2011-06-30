//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _WINDOWSKEYSTATEPROVIDER_H_
#define _WINDOWSKEYSTATEPROVIDER_H_


#include "cpcKeyStateProvider.h"


/**
** Concrete implementation of CPC::CKeyStateProvider that reads key states from the Windows keyboard.
*/
class WindowsKeyStateProvider : public CPC::CKeyStateProvider
{
public:

  /** Called by the emulator to know the current released/pressed state of the given CPC key. */
  virtual CPC::ECpcKeyState  GetKeyState               (CPC::ECpcKey eCpcKey);

};

#endif // _WINDOWSKEYSTATEPROVIDER_H_
