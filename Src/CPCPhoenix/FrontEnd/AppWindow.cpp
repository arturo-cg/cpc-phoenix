//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AppWindow.h"
#include "Application.h"
#include "StatusBar.h"
#include "cpcMachine.h"
#include "cpcDisplay.h"
#include "cpcKeyboard.h"

#include <Windows.h>
#include "resource.h"



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
    ::AdjustWindowRect( &rWndRect, dwStyles, TRUE/*bMenu*/ );
    ::OffsetRect( &rWndRect, -rWndRect.left, -rWndRect.top );

    bRet = Super::Init( "CPCPhoenix", dwStyles, rWndRect.left, rWndRect.top, rWndRect.right, rWndRect.bottom, NULL/*hParentOrOwner*/ );
  }

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Main menu
  if (bRet)
  {
    m_hMainMenu = ::LoadMenu( ::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINMENU) );
    KMASSERTM( m_hMainMenu != NULL, ("Could not create the main menu. GetLastError() == %d", ::GetLastError()) );
    if (m_hMainMenu != NULL)
    {
      ::SetMenu( this->GetHWnd(), m_hMainMenu );
      OnApplicationSettingsChanged();
    }
  }

  // Status bar
  if (bRet)
  {
    m_pStatusBar = new StatusBar;
    m_pStatusBar->Init( this );
  }

  // Key accelerators
  if (bRet)
  {
    m_hAccelerators = ::LoadAccelerators( ::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_APPWINDOWACCELERATORS) );
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
  m_hMainMenu        = NULL;
  m_hAccelerators    = NULL;
  m_pStatusBar       = NULL;
  m_BackBufferDC     = NULL;
  m_BackBufferBitmap = NULL;
  m_pBackBuffer      = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void AppWindow::FreeVars()
{
  delete m_pStatusBar;
  delete m_pBackBuffer;
}

//----------------------------------------------------------------------------
/**
** 
*/
void AppWindow::OnApplicationSettingsChanged()
{
  const Settings* pSettings;
  pSettings = Application::Singleton()->GetSettings();

  // Update the menu
  UINT nItem;
  switch ( pSettings->GetCpcModel() )
  {
    case CPC::CMachine::MODEL_464:         nItem = ID_SETTINGS_CHANGECPCMODEL_CPC464; break;
    case CPC::CMachine::MODEL_664:         nItem = ID_SETTINGS_CHANGECPCMODEL_CPC664; break;
    case CPC::CMachine::MODEL_6128:        nItem = ID_SETTINGS_CHANGECPCMODEL_CPC6128; break;
    case CPC::CMachine::MODEL_6128_MAXAM:  nItem = ID_SETTINGS_CHANGECPCMODEL_CPC6128_MAXAM; break;
    default:                               KMASSERT(false); nItem = ID_SETTINGS_CHANGECPCMODEL_CPC464; break;
  }
  ::CheckMenuRadioItem( m_hMainMenu, ID_SETTINGS_CHANGECPCMODEL_CPC464, ID_SETTINGS_CHANGECPCMODEL_CPC6128_MAXAM, nItem, MF_BYCOMMAND );
  ::CheckMenuItem( m_hMainMenu, ID_SETTINGS_DRAWSCANLINES, /*MF_BYCOMMAND | */ pSettings->GetDrawScanLines() ? MF_CHECKED : MF_UNCHECKED );

  if (pSettings->GetEmulationSpeed() < 0.f)
  {
    nItem = ID_SETTINGS_EMULATIONSPEED_UNLIMITED;
  }
  else
  if (pSettings->GetEmulationSpeed() <= 0.251f)
  {
    nItem = ID_SETTINGS_EMULATIONSPEED_25;
  }
  else
  if (pSettings->GetEmulationSpeed() <= 0.51f)
  {
    nItem = ID_SETTINGS_EMULATIONSPEED_50;
  }
  else
  if (pSettings->GetEmulationSpeed() <= 1.1f)
  {
    nItem = ID_SETTINGS_EMULATIONSPEED_100;
  }
  else
  {
    nItem = ID_SETTINGS_EMULATIONSPEED_UNLIMITED;
  }
  ::CheckMenuRadioItem( m_hMainMenu, ID_SETTINGS_EMULATIONSPEED_25, ID_SETTINGS_EMULATIONSPEED_UNLIMITED, nItem, MF_BYCOMMAND );
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
/*virtual*/ LRESULT AppWindow::_OnSize(int iWidth, int iHeight)
{
  LRESULT nRet = 0;

  if (m_pStatusBar != NULL)
  {
    // Resize the status bar
    nRet = ::SendMessage( m_pStatusBar->GetHWnd(), WM_SIZE, 0, LOWORD(iWidth) | HIWORD(iHeight) );
  }

  return nRet;
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

  switch (nItemId)
  {
    //
    // File Menu
    //

    case ID_FILE_EXIT:  RequestClose(); break;


    //
    // Settings Menu
    //

    case ID_SETTINGS_CHANGECPCMODEL_CPC464:         pApplication->ChangeCpcModelSetting( CPC::CMachine::MODEL_464 ); break;
    case ID_SETTINGS_CHANGECPCMODEL_CPC664:         pApplication->ChangeCpcModelSetting( CPC::CMachine::MODEL_664 ); break;
    case ID_SETTINGS_CHANGECPCMODEL_CPC6128:        pApplication->ChangeCpcModelSetting( CPC::CMachine::MODEL_6128 ); break;
    case ID_SETTINGS_CHANGECPCMODEL_CPC6128_MAXAM:  pApplication->ChangeCpcModelSetting( CPC::CMachine::MODEL_6128_MAXAM ); break;

    case ID_SETTINGS_DRAWSCANLINES:  pApplication->ChangeDrawScanLinesSetting( !pApplication->GetSettings()->GetDrawScanLines() ); break;

    case ID_SETTINGS_EMULATIONSPEED_25:         pApplication->ChangeEmulationSpeedSetting( 0.25 ); break;
    case ID_SETTINGS_EMULATIONSPEED_50:         pApplication->ChangeEmulationSpeedSetting( 0.5 ); break;
    case ID_SETTINGS_EMULATIONSPEED_100:        pApplication->ChangeEmulationSpeedSetting( 1.f ); break;
    case ID_SETTINGS_EMULATIONSPEED_UNLIMITED:  pApplication->ChangeEmulationSpeedSetting( -1.f ); break;

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
