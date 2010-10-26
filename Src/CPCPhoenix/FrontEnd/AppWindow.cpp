//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AppWindow.h"
#include "Application.h"
#include "cpcMachine.h"
#include "cpcDisplay.h"

#include <Windows.h>



//----------------------------------------------------------------------------
/**
** Init
*/
bool AppWindow::Init()
{
  bool bRet = true;

  End( false );
  ResetVars();

  // Initialize base class
  if (bRet)
  {
    // Calculate the window rectangle based on the desired client rectangle
    DWORD dwStyles;
    dwStyles = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

    RECT rWndRect;
    ::SetRect( &rWndRect, 0, 0, 1280, 800 );
    ::AdjustWindowRect( &rWndRect, dwStyles, FALSE );
    ::OffsetRect( &rWndRect, -rWndRect.left, -rWndRect.top );

    bRet = Super::Init( "CPCPhoenix", dwStyles, rWndRect.left, rWndRect.top, rWndRect.right, rWndRect.bottom, NULL/*hParentOrOwner*/ );
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

    m_pBackBuffer = new unsigned char [nBackBufferLength];

    // Create a CBitmap to hold the back-buffer and tell it to grab pixels from m_pBackBuffer
    SIZE bitmapSize;
    bitmapSize.cx = CPC::CDisplay::IMAGEBUFFER_WIDTH;
    bitmapSize.cy = CPC::CDisplay::IMAGEBUFFER_HEIGHT;

    ////////m_BackBufferBitmap.CreateBitmap( bitmapSize.cx, bitmapSize.cy, 1/*nPlanes*/, 32/*nBitCount = bpp*/, m_pBackBuffer/*lpBits*/ );
    m_BackBufferBitmap = ::CreateCompatibleBitmap( this->GetDc(), bitmapSize.cx, bitmapSize.cy );

//*************************************** TODO - TODO - TODO *********************************************
//*************************************** TODO - TODO - TODO *********************************************
    //// Check device context format and tell CPC::CDisplay to decode the image in the same format.
    //BITMAP bitmapInfo;
    //m_BackBufferBitmap.GetBitmap( &bitmapInfo );
//*************************************** TODO - TODO - TODO *********************************************
//*************************************** TODO - TODO - TODO *********************************************

    // Create the memory DC that will be used as the back-buffer
    m_BackBufferDC = ::CreateCompatibleDC( this->GetDc() );
    ::SelectObject( m_BackBufferDC, m_BackBufferBitmap );
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
void AppWindow::ResetVars()
{
  //...
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
void AppWindow::UpdateDisplayImage()
{
  // Tell the emulated machine to decode the current frame
  Application::Singleton()->GetEmulatedMachine()->GetDisplay()->DecodeImage_B8G8R8X8( m_pBackBuffer );
  ::SetBitmapBits( m_BackBufferBitmap, CPC::CDisplay::IMAGEBUFFER_WIDTH * CPC::CDisplay::IMAGEBUFFER_HEIGHT * 4/*bytes-per-pixel*/, m_pBackBuffer );
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ LRESULT AppWindow::_OnClose()
{
  Application::Singleton()->_OnAppWindowCloseRequest( this );
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ LRESULT AppWindow::_OnPaint(HDC hDc)
{
  // Copy the back-buffer to the window DC
  RECT rClientArea;
  GetClientRect( &rClientArea );

  ::StretchBlt( hDc, 0, 0, rClientArea.right - rClientArea.left, rClientArea.bottom - rClientArea.top,
                m_BackBufferDC, 0, 0, CPC::CDisplay::IMAGEBUFFER_WIDTH, CPC::CDisplay::IMAGEBUFFER_HEIGHT,
                SRCCOPY );
  //     dc.BitBlt( 0, 0, rClientArea.Width(), rClientArea.Height(),
  //                &m_BackBufferDC, 0, 0, SRCCOPY );

  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ LRESULT AppWindow::_OnKeyDown(unsigned nVirtualKey)
{
  CPC::CMachine* pEmulatedMachine;
  pEmulatedMachine = Application::Singleton()->GetEmulatedMachine();

  if (nVirtualKey == VK_F1)     // F1 key --> Toggle scan line effect
  {
    pEmulatedMachine->GetDisplay()->SetScanLineEffectActivated( !pEmulatedMachine->GetDisplay()->IsScanLineEffectActivated() );
  }
  else
  if (nVirtualKey == VK_F2)     // F2 key --> Reset machine
  {
    pEmulatedMachine->Reset();
  }

  return 0;
}
