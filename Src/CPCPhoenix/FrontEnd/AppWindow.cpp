//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AppWindow.h"
#include "Application.h"
#include "DisplayWindow.h"
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
    DWORD dwStyles;
    dwStyles = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

    bRet = Super::Init( "CPCPhoenix", dwStyles, 0/*x*/, 0/*y*/, 1/*width*/, 1/*height*/, NULL/*hParentOrOwner*/ );
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

  // DisplayWindow
  if (bRet)
  {
    RECT rWndRect;
    ::SetRect( &rWndRect, 0, 0, CPC::CDisplay::IMAGEBUFFER_WIDTH * 2, CPC::CDisplay::IMAGEBUFFER_HEIGHT * 2 );

    m_pDisplayWindow = new DisplayWindow;
    m_pDisplayWindow->Init( rWndRect, this );
  }

  // Status bar
  if (bRet)
  {
    m_pStatusBar = new StatusBar;
    m_pStatusBar->Init( this );
  }

  // Resize application window
  if (bRet)
  {
    // Calculate the window rectangle
    DWORD dwStyles;
    dwStyles = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

    RECT rWndRect;
    ::SetRect( &rWndRect, 0, 0, CPC::CDisplay::IMAGEBUFFER_WIDTH * 2, (CPC::CDisplay::IMAGEBUFFER_HEIGHT * 2) + m_pStatusBar->GetHeight() );
    ::AdjustWindowRect( &rWndRect, dwStyles, TRUE/*bMenu*/ );
    ::OffsetRect( &rWndRect, -rWndRect.left, -rWndRect.top );

    // Resize it
    SetRect( rWndRect );
  }

  // Key accelerators
  if (bRet)
  {
    m_hAccelerators = ::LoadAccelerators( ::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_APPWINDOWACCELERATORS) );
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
  m_hMainMenu      = NULL;
  m_hAccelerators  = NULL;
  m_pDisplayWindow = NULL;
  m_pStatusBar     = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void AppWindow::FreeVars()
{
  delete m_pStatusBar;
  delete m_pDisplayWindow;
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
  m_pDisplayWindow->UpdateDisplayImage();
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
/*virtual*/ LRESULT AppWindow::_OnSize(int iWidth, int iHeight)
{
  // Resize the status bar
  if (m_pStatusBar != NULL)
  {
    ::SendMessage( m_pStatusBar->GetHWnd(), WM_SIZE, 0, LOWORD(iWidth) | HIWORD(iHeight) );
    ::InvalidateRect( m_pStatusBar->GetHWnd(), NULL, FALSE );
  }

  // Resize the display window
  if (m_pDisplayWindow != NULL)
  {
    m_pDisplayWindow->SetSize( iWidth, iHeight - m_pStatusBar->GetHeight() );
    m_pDisplayWindow->InvalidateAll( FALSE );
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
