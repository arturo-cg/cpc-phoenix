//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AppWindow.h"
#include "Application.h"
#include "StatusBar.h"
#include "WinVideoOutput.h"
#include "RenderingApi.h"
#include "cpcMachine.h"
#include "cpcKeyboard.h"

#include <Windows.h>
#include "resource.h"


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



//----------------------------------------------------------------------------
/**
** Init
*/
bool AppWindow::Init()
{
    bool bRet = true;

    End(false);
    ResetVars();

    // Initialize base class
    if (bRet)
    {
        DWORD dwStyles;
        dwStyles = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

        bRet = Super::Init("CPCPhoenix", dwStyles, 0/*x*/, 0/*y*/, 1/*width*/, 1/*height*/, NULL/*hParentOrOwner*/);
    }

    // Check parameters
    if (bRet)
    {
        //...
    }

    // Resize application window
    if (bRet)
    {
        ResizeToScale(Application::Singleton()->GetSettings()->GetScale());
    }

    // Key accelerators
    if (bRet)
    {
        m_hAccelerators = ::LoadAccelerators(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_APPWINDOWACCELERATORS));
    }


    if (!bRet)
    {
        FreeVars();
        m_bOk = false;
    }

    return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void AppWindow::End(bool bIncludedSuper/* = true*/)
{
    if (IsOk())
    {
        FreeVars();

        if (bIncludedSuper)
        {
            Super::End();
        }
    }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void AppWindow::ResetVars()
{
    m_hAccelerators = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void AppWindow::FreeVars()
{
    //...
}

//----------------------------------------------------------------------------
/**
**
*/
void AppWindow::ResizeToScale(float scale)
{
    // Calculate the window rectangle
    DWORD dwStyles;
    dwStyles = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

    RECT rWndRect;
    int nNewWidth = int(CPC::CVideoOutput::VIEWPORT_WIDTH * scale);
    int nNewHeight = int(CPC::CVideoOutput::VIEWPORT_HEIGHT * 2 * scale);

    ::SetRect(&rWndRect, 0, 0, nNewWidth, nNewHeight);
    ::AdjustWindowRect(&rWndRect, dwStyles, FALSE/*bMenu*/);
    ::OffsetRect(&rWndRect, -rWndRect.left, -rWndRect.top);

    // Resize it
    SetRect(rWndRect);
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ bool AppWindow::_OnWindowProcedureProlog(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Let Dear ImGui process every message.
    bool skipMessage = (ImGui_ImplWin32_WndProcHandler(GetHWnd(), uMsg, wParam, lParam) != 0);
    return skipMessage;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnClose()
{
    Application::Singleton()->_OnAppWindowCloseRequest(this);
    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnSize(int iWidth, int iHeight)
{
    // Resize rendering buffers.
    if ((Application::Singleton()->GetRenderingApi() != NULL)/* && (wParam != SIZE_MINIMIZED)*/)
    {
        Application::Singleton()->GetRenderingApi()->ResizeRenderTarget(iWidth, iHeight);
    }

    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
LRESULT AppWindow::_OnMenuCommand(WORD nItemId, bool bFromAccelerator)
{
    Application* pApplication;
    pApplication = Application::Singleton();

    CPC::CMachine* pEmulatedMachine;
    pEmulatedMachine = pApplication->GetEmulatedMachine();

    // This used to have all the entries of the old native menu.
    // Now, only the key accelerators are present.
    // Dear ImGui doens't have support for key accelerators so we have to do it the old, native way.
    switch (nItemId)
    {
        //
        // File Menu
        //

        case ID_FILE_EXIT:  RequestClose(); break;


        //
        // Settings Menu
        //

        case ID_SETTINGS_EMULATIONSPEED_25:         pApplication->ChangeEmulationSpeedSetting(0.25f); break;
        case ID_SETTINGS_EMULATIONSPEED_50:         pApplication->ChangeEmulationSpeedSetting(0.5f); break;
        case ID_SETTINGS_EMULATIONSPEED_100:        pApplication->ChangeEmulationSpeedSetting(1.f); break;
        case ID_SETTINGS_EMULATIONSPEED_120:        pApplication->ChangeEmulationSpeedSetting(1.19808f); break;     // = 19968.0 (50.08 fps) / 16666.6667 (60 fps)  <- It speeds up the emulation so that it completes a new frame at a 60 Hz rate.
        case ID_SETTINGS_EMULATIONSPEED_UNLIMITED:  pApplication->ChangeEmulationSpeedSetting(-1.f); break;

        case ID_SETTINGS_RESET:  pEmulatedMachine->Reset(); break;
    }

    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnKeyDown(unsigned nVirtualKey)
{
    return 0;
}
