//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "DisplayWindow.h"
#include "Application.h"
#include "WinVideoOutput.h"
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

    ::StretchDIBits(hDc, 0, 0, rClientArea.right - rClientArea.left, rClientArea.bottom - rClientArea.top,
                    0, 0, output.nWidth, output.nHeight, output.pDibBits, output.pDibInfo, DIB_RGB_COLORS, SRCCOPY);

    return 0;
}
