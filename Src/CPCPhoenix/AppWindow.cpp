//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AppWindow.h"
#include "Application.h"
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

        static constexpr int ExtraWidth = 60;
        static constexpr int ExtraHeight = 140;
        int width = int(CPC::CVideoOutput::VIEWPORT_WIDTH) + ExtraWidth;
        int height = int(CPC::CVideoOutput::VIEWPORT_HEIGHT * 2) + ExtraHeight;

        bRet = Super::Init("CPCPhoenix", dwStyles, 0/*x*/, 0/*y*/, width, height, NULL/*hParentOrOwner*/);
    }

    // Check parameters
    if (bRet)
    {
        //...
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
void AppWindow::ComputeDisplayWindowSize(int* out_width, int* out_height)
{
    KMASSERT((out_width != nullptr) && (out_height != nullptr));

    RECT appWindowClientRect;
    GetClientRect(&appWindowClientRect);
    *out_width = appWindowClientRect.right;
    *out_height = appWindowClientRect.bottom;
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
LRESULT AppWindow::_OnSizing(LPRECT prRect)
{
    Application::Singleton()->_OnAppWindowSizing();
    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnSize(int iWidth, int iHeight)
{
    int displayWindowWidth;
    int displayWindowHeight;
    ComputeDisplayWindowSize(&displayWindowWidth, &displayWindowHeight);

    // Resize rendering buffers.
    if ((Application::Singleton()->GetRenderingApi() != NULL)/* && (wParam != SIZE_MINIMIZED)*/)
    {
        Application::Singleton()->GetRenderingApi()->ResizeRenderTarget(displayWindowWidth, displayWindowHeight);
    }

    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnKeyDown(unsigned virtualKey)
{
    // Get state of SHIFT, CTRL and ALT keys.
    bool shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    bool altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
    // Notify the application.
    Application::Singleton()->_OnAppWindowKeyDown(virtualKey, shiftPressed, ctrlPressed, altPressed);

    return 0;
}
