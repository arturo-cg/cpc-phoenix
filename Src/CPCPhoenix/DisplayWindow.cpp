//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "DisplayWindow.h"
#include "Application.h"
#include "cpcMachine.h"
#include "cpcDisplay.h"

#include <Windows.h>
#include "resource.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool DisplayWindow::Init(const RECT& placement, kmbWindow* pParent)
{
  bool bRet = true;

  End( false );
  ResetVars();

  // Initialize base class
  if (bRet)
  {
    DWORD dwStyles;
    dwStyles = (WS_CHILD | WS_VISIBLE);

    bRet = Super::Init( "", dwStyles, placement.left, placement.top, placement.right, placement.bottom, pParent->GetHWnd() );
  }

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Back-buffer
  if (bRet)
  {
    // Create the back-buffer
    unsigned nBackBufferLength;
    nBackBufferLength = CPC::CDisplay::IMAGEBUFFER_WIDTH * CPC::CDisplay::IMAGEBUFFER_HEIGHT * 4/*bytes-per-pixel*/;

    m_pBackBufferDibBits = new unsigned char [nBackBufferLength];

    // Fill back-buffer DIB info
    memset( &m_backBufferDibInfo, 0, sizeof(m_backBufferDibInfo) );
    m_backBufferDibInfo.bmiHeader.biSize = sizeof(m_backBufferDibInfo);
    m_backBufferDibInfo.bmiHeader.biWidth = (LONG) CPC::CDisplay::IMAGEBUFFER_WIDTH;
    m_backBufferDibInfo.bmiHeader.biHeight = -(LONG) CPC::CDisplay::IMAGEBUFFER_HEIGHT;    // Note: Negative height for a top-down DIB, with its origin at the upper-left corner.
    m_backBufferDibInfo.bmiHeader.biPlanes = 1;
    m_backBufferDibInfo.bmiHeader.biBitCount = 32;
    m_backBufferDibInfo.bmiHeader.biCompression = BI_RGB;
    m_backBufferDibInfo.bmiHeader.biSizeImage = 0;
    m_backBufferDibInfo.bmiHeader.biClrUsed = 0;
    m_backBufferDibInfo.bmiHeader.biClrImportant = 0;
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
  if ( IsOk() )
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
  memset( &m_backBufferDibInfo, 0, sizeof(m_backBufferDibInfo) );
  m_pBackBufferDibBits = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void DisplayWindow::FreeVars()
{
  delete m_pBackBufferDibBits;
}

//----------------------------------------------------------------------------
/**
** 
*/
void DisplayWindow::UpdateDisplayImage()
{
  // Tell the emulated machine to decode the current frame
  Application::Singleton()->GetEmulatedMachine()->GetDisplay()->DecodeImage_B8G8R8X8( m_pBackBufferDibBits );
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ LRESULT DisplayWindow::_OnPaint(HDC hDc)
{
  // Copy the back-buffer DIB to the window DC
  RECT rClientArea;
  GetClientRect( &rClientArea );

  ::StretchDIBits( hDc, 0, 0, rClientArea.right - rClientArea.left, rClientArea.bottom - rClientArea.top,
                   0, 0, CPC::CDisplay::IMAGEBUFFER_WIDTH, CPC::CDisplay::IMAGEBUFFER_HEIGHT,
                   m_pBackBufferDibBits, &m_backBufferDibInfo, DIB_RGB_COLORS, SRCCOPY );

  return 0;
}
