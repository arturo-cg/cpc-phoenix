//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "WindowsKeyStateProvider.h"
#include "Application.h"
#include "AppWindow.h"



//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ CPC::ECpcKeyState WindowsKeyStateProvider::GetKeyState(CPC::ECpcKey eCpcKey)
{
  CPC::ECpcKeyState eRet = CPC::CPCKEYSTATE_RELEASED;

  // Look whether the application has the input focus
  if (::GetFocus() == Application::Singleton()->GetAppWindow()->GetHWnd())
  {
    // Get the Windows key the CPC key is mapped to
    int nWindowsKey;
    nWindowsKey = Application::Singleton()->GetSettings()->GetCpcKeyMapping( eCpcKey );

    if (nWindowsKey != 0)  // If it is a valid key...
    {
      //
      // TODO - Take Num Lock ON/OFF state into account for certain keys.
      //

      // Get the current state of the Windows key
      if (::GetAsyncKeyState(nWindowsKey) & 0x8000)      // If the key is pressed...
      {
        eRet = CPC::CPCKEYSTATE_PRESSED;
      }
      else
      {
        eRet = CPC::CPCKEYSTATE_RELEASED;
      }
    }
  }

  return eRet;
}
