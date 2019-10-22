//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "DisplayWindow.h"
#include "Application.h"
#include "WinVideoOutput.h"
#include "cpcVideoOutput.h"
#include "cpcMachine.h"

#include <Windows.h>
#include "resource.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool DisplayWindow::Init(const RECT& placement, kmbWindow* pParent)
{
    bool bRet = true;

    End(false);
    ResetVars();

    // Initialize base class
    if (bRet)
    {
        DWORD dwStyles;
        dwStyles = (WS_CHILD | WS_VISIBLE);

        bRet = Super::Init("", dwStyles, placement.left, placement.top, placement.right, placement.bottom, pParent->GetHWnd());
    }

    // Check parameters
    if (bRet)
    {
        //...
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
/*virtual*/ void DisplayWindow::End(bool bIncludedSuper/* = true*/)
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
void DisplayWindow::ResetVars()
{
    //...
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void DisplayWindow::FreeVars()
{
    //...
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT DisplayWindow::_OnPaint(HDC hDc)
{
    // Get current video output.
    CWinVideoOutput* pVideoOutput = Application::Singleton()->GetWinVideoOutput();
    CWinVideoOutput::SOutput output;
    pVideoOutput->GetOutput(&output);

    // Copy the back-buffer DIB to the window DC
    RECT rClientArea;
    GetClientRect(&rClientArea);

    // Use these values to display the entire buffer.
    //int srcX = 0;
    //int srcY = 0;
    //int srcWidth = output.nWidth;
    //int srcHeight = output.nHeight;

    // Use these values to display the area that a CPC monitor would actually display.
    int srcX = CPC::CVideoOutput::VIEWPORT_LEFT;
    int srcY = CPC::CVideoOutput::BUFFER_HEIGHT - CPC::CVideoOutput::VIEWPORT_TOP - CPC::CVideoOutput::VIEWPORT_HEIGHT;
    int srcWidth = CPC::CVideoOutput::VIEWPORT_WIDTH;
    int srcHeight = CPC::CVideoOutput::VIEWPORT_HEIGHT;

    ::StretchDIBits(hDc, 0, 0, rClientArea.right - rClientArea.left, rClientArea.bottom - rClientArea.top,
                    srcX, srcY, srcWidth, srcHeight, output.pDibBits, output.pDibInfo, DIB_RGB_COLORS, SRCCOPY);

    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT DisplayWindow::_OnEraseBkgnd()
{
    return 1;   // Prevent default window procedure from filling the window with the background color.
}
