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

    if ((::GetFocus() == Application::Singleton()->GetAppWindow()->GetHWnd()) &&        // If our app window has the input focus...
        !ImGui::GetIO().WantCaptureKeyboard)                                            // If Dear ImGui does *not* want the keyboard for itself...
    {
        // Get the Windows key the CPC key is mapped to
        const Settings::SMappedKey& mappedKey = Application::Singleton()->GetSettings()->GetCpcKeyMapping(eCpcKey);

        // Get the current state of the Windows key
        if (::GetAsyncKeyState(mappedKey.nWindowsKey) & 0x8000)      // If the key is pressed...
        {
            // Check NUM LOCK key state
            Settings::EModifierKeyState eNumLockState;
            if (::GetKeyState(VK_NUMLOCK) & 0x0001)
            {
                eNumLockState = Settings::MODIFIERKEY_ON;
            }
            else
            {
                eNumLockState = Settings::MODIFIERKEY_OFF;
            }

            if (mappedKey.eNumLock & eNumLockState)
            {
                eRet = CPC::CPCKEYSTATE_PRESSED;
            }
        }
    }

    return eRet;
}
