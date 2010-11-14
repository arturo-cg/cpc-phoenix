//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AppWindow.h"
#include "Application.h"
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
    case CPC::CMachine::MODEL_464:   nItem = ID_SETTINGS_CHANGECPCMODEL_CPC464; break;
    case CPC::CMachine::MODEL_664:   nItem = ID_SETTINGS_CHANGECPCMODEL_CPC664; break;
    case CPC::CMachine::MODEL_6128:  nItem = ID_SETTINGS_CHANGECPCMODEL_CPC6128; break;
    default:                         KMASSERT(false); nItem = ID_SETTINGS_CHANGECPCMODEL_CPC464; break;
  }
  ::CheckMenuRadioItem( m_hMainMenu, ID_SETTINGS_CHANGECPCMODEL_CPC464, ID_SETTINGS_CHANGECPCMODEL_CPC6128, nItem, MF_BYCOMMAND );
  ::CheckMenuItem( m_hMainMenu, ID_SETTINGS_DRAWSCANLINES, /*MF_BYCOMMAND | */ pSettings->GetDrawScanLines() ? MF_CHECKED : MF_UNCHECKED );
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
CPC::ECpcKey AppWindow::MapKeyHostToCpc(unsigned nVirtualKey)
{
  CPC::ECpcKey eRet = CPC::CPCKEY_INVALID;

  switch (nVirtualKey)
  {
    case VK_ESCAPE:  eRet = CPC::CPCKEY_ESC; break;
    case '1':  eRet = CPC::CPCKEY_1; break;
    case '2':  eRet = CPC::CPCKEY_2; break;
    case '3':  eRet = CPC::CPCKEY_3; break;
    case '4':  eRet = CPC::CPCKEY_4; break;
    case '5':  eRet = CPC::CPCKEY_5; break;
    case '6':  eRet = CPC::CPCKEY_6; break;
    case '7':  eRet = CPC::CPCKEY_7; break;
    case '8':  eRet = CPC::CPCKEY_8; break;
    case '9':  eRet = CPC::CPCKEY_9; break;
    case '0':  eRet = CPC::CPCKEY_0; break;
    //case VK_:  eRet = CPC::CPCKEY_MINUS; break;
    //case VK_:  eRet = CPC::CPCKEY_ARROW; break;
    //case VK_:  eRet = CPC::CPCKEY_CLR; break;
    case VK_BACK:  eRet = CPC::CPCKEY_DEL; break;
    case VK_TAB:  eRet = CPC::CPCKEY_TAB; break;
    case 'Q':  eRet = CPC::CPCKEY_Q; break;
    case 'W':  eRet = CPC::CPCKEY_W; break;
    case 'E':  eRet = CPC::CPCKEY_E; break;
    case 'R':  eRet = CPC::CPCKEY_R; break;
    case 'T':  eRet = CPC::CPCKEY_T; break;
    case 'Y':  eRet = CPC::CPCKEY_Y; break;
    case 'U':  eRet = CPC::CPCKEY_U; break;
    case 'I':  eRet = CPC::CPCKEY_I; break;
    case 'O':  eRet = CPC::CPCKEY_O; break;
    case 'P':  eRet = CPC::CPCKEY_P; break;
    //case VK_:  eRet = CPC::CPCKEY_AT; break;
    //case VK_:  eRet = CPC::CPCKEY_BRACKET_OPEN; break;
    case VK_RETURN:  eRet = CPC::CPCKEY_RETURN; break;
    //case VK_:  eRet = CPC::CPCKEY_CAPS_LOCK; break;
    case 'A':  eRet = CPC::CPCKEY_A; break;
    case 'S':  eRet = CPC::CPCKEY_S; break;
    case 'D':  eRet = CPC::CPCKEY_D; break;
    case 'F':  eRet = CPC::CPCKEY_F; break;
    case 'G':  eRet = CPC::CPCKEY_G; break;
    case 'H':  eRet = CPC::CPCKEY_H; break;
    case 'J':  eRet = CPC::CPCKEY_J; break;
    case 'K':  eRet = CPC::CPCKEY_K; break;
    case 'L':  eRet = CPC::CPCKEY_L; break;
    //case VK_:  eRet = CPC::CPCKEY_COLON; break;
    //case VK_:  eRet = CPC::CPCKEY_SEMICOLON; break;
    //case VK_:  eRet = CPC::CPCKEY_BRACKET_CLOSE; break;
    case VK_SHIFT:  eRet = CPC::CPCKEY_SHIFT; break;
    case 'Z':  eRet = CPC::CPCKEY_Z; break;
    case 'X':  eRet = CPC::CPCKEY_X; break;
    case 'C':  eRet = CPC::CPCKEY_C; break;
    case 'V':  eRet = CPC::CPCKEY_V; break;
    case 'B':  eRet = CPC::CPCKEY_B; break;
    case 'N':  eRet = CPC::CPCKEY_N; break;
    case 'M':  eRet = CPC::CPCKEY_M; break;
    case ',':  eRet = CPC::CPCKEY_COMMA; break;
    case '.':  eRet = CPC::CPCKEY_PERIOD; break;
    case '/':  eRet = CPC::CPCKEY_SLASH; break;
    case '\\':  eRet = CPC::CPCKEY_INVERTED_SLASH; break;
    case VK_SPACE:  eRet = CPC::CPCKEY_SPACE; break;
    case VK_CLEAR:  eRet = CPC::CPCKEY_CTRL; break;
    case VK_UP:  eRet = CPC::CPCKEY_CURSOR_UP; break;
    case VK_DOWN:  eRet = CPC::CPCKEY_CURSOR_DOWN; break;
    case VK_LEFT:  eRet = CPC::CPCKEY_CURSOR_LEFT; break;
    case VK_RIGHT:  eRet = CPC::CPCKEY_CURSOR_RIGHT; break;
    case VK_INSERT:  eRet = CPC::CPCKEY_COPY; break;
    case VK_NUMPAD0:  eRet = CPC::CPCKEY_NUM_0; break;
    case VK_NUMPAD1:  eRet = CPC::CPCKEY_NUM_1; break;
    case VK_NUMPAD2:  eRet = CPC::CPCKEY_NUM_2; break;
    case VK_NUMPAD3:  eRet = CPC::CPCKEY_NUM_3; break;
    case VK_NUMPAD4:  eRet = CPC::CPCKEY_NUM_4; break;
    case VK_NUMPAD5:  eRet = CPC::CPCKEY_NUM_5; break;
    case VK_NUMPAD6:  eRet = CPC::CPCKEY_NUM_6; break;
    case VK_NUMPAD7:  eRet = CPC::CPCKEY_NUM_7; break;
    case VK_NUMPAD8:  eRet = CPC::CPCKEY_NUM_8; break;
    case VK_NUMPAD9:  eRet = CPC::CPCKEY_NUM_9; break;
    case VK_DECIMAL:  eRet = CPC::CPCKEY_NUM_PERIOD; break;
    case VK_SEPARATOR:  eRet = CPC::CPCKEY_NUM_ENTER; break;
    //case VK_:  eRet = CPC::CPCKEY_JOY_UP; break;
    //case VK_:  eRet = CPC::CPCKEY_JOY_DOWN; break;
    //case VK_:  eRet = CPC::CPCKEY_JOY_LEFT; break;
    //case VK_:  eRet = CPC::CPCKEY_JOY_RIGHT; break;
    //case VK_:  eRet = CPC::CPCKEY_JOY_FIRE1; break;
    //case VK_:  eRet = CPC::CPCKEY_JOY_FIRE2; break;
    default: eRet = CPC::CPCKEY_INVALID; break;
  }

  return eRet;
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

    case ID_SETTINGS_CHANGECPCMODEL_CPC464:   pApplication->ChangeCpcModelSetting( CPC::CMachine::MODEL_464 ); break;
    case ID_SETTINGS_CHANGECPCMODEL_CPC664:   pApplication->ChangeCpcModelSetting( CPC::CMachine::MODEL_664 ); break;
    case ID_SETTINGS_CHANGECPCMODEL_CPC6128:  pApplication->ChangeCpcModelSetting( CPC::CMachine::MODEL_6128 ); break;

    case ID_SETTINGS_DRAWSCANLINES:  pApplication->ChangeDrawScanLinesSetting( !pApplication->GetSettings()->GetDrawScanLines() ); break;

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
  // Try to map the host key to a CPC key
  CPC::ECpcKey eCpcKey;
  eCpcKey = MapKeyHostToCpc( nVirtualKey );

  if (eCpcKey != CPC::CPCKEY_INVALID)
  {
    // Press the corresponding CPC key
    Application::Singleton()->GetEmulatedMachine()->GetKeyboard()->SetKeyState( eCpcKey, CPC::CPCKEYSTATE_PRESSED );
  }

  return 0;
}

/**
** 
*/
/*virtual*/ LRESULT AppWindow::_OnKeyUp(unsigned nVirtualKey)
{
  // Try to map the host key to a CPC key
  CPC::ECpcKey eCpcKey;
  eCpcKey = MapKeyHostToCpc( nVirtualKey );

  if (eCpcKey != CPC::CPCKEY_INVALID)
  {
    // Release the corresponding CPC key
    Application::Singleton()->GetEmulatedMachine()->GetKeyboard()->SetKeyState( eCpcKey, CPC::CPCKEYSTATE_RELEASED );
  }

  return 0;
}
