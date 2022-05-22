//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbWindow.h"
#include <Windows.h>


/*static*/ bool kmbWindow::ms_bWindowClassRegistered = false;


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#define EXTRACT_APPWINDOW_PTR(hWnd)   (reinterpret_cast<kmbWindow*>( ::GetWindowLongPtr(hWnd, GWLP_USERDATA) ))

//----------------------------------------------------------------------------
/**
** The WindowProc used by all kmbWindows.
*/
LRESULT CALLBACK kmbWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  bool       bCallDefWindowProc = false;
  kmbWindow* pWindow            = NULL;

  // Derived class-defined prolog.
  pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
  if (pWindow != nullptr)
  {
      bool skipMessage;
      skipMessage = pWindow->_OnWindowProcedureProlog(uMsg, wParam, lParam);
      if (skipMessage)
      {
          return 1;
      }
  }

  switch (uMsg)
  {
  case WM_CREATE:
    {
      CREATESTRUCT* pCreateStruct;

      // Here the pointer to the kmbWindow is not in the userdata field yet,
      // we have to get it from the parameter in the CreateWindowEx call.
      pCreateStruct = reinterpret_cast<CREATESTRUCT*>( lParam );
      pWindow       = reinterpret_cast<kmbWindow*>( pCreateStruct->lpCreateParams );
      pWindow->_SetHWnd( hWnd );
      return pWindow->_OnCreate( pCreateStruct );
    }
    break;

  case WM_CLOSE:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnClose();
    }
    break;

  case WM_DESTROY:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnDestroy();
    }
    break;

  case WM_COMMAND:
    {
      if (lParam != 0)   // It is 0 when it comes from a menu or from an accelerator, but not from a control.
      {
        pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
        return pWindow->_OnControlCommand( HIWORD(wParam), LOWORD(wParam), (HWND)lParam );
      }
      else
      {
        pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
        return pWindow->_OnMenuCommand( LOWORD(wParam), HIWORD(wParam)==1 );
      }
    }
    break;

  case WM_PAINT:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );

      HDC         hDc;
      PAINTSTRUCT paintStruct;
      hDc = BeginPaint( hWnd, &paintStruct );
      pWindow->_OnPaint( hDc );
      EndPaint( hWnd, &paintStruct );
    }
    break;

  case WM_ERASEBKGND:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnEraseBkgnd();
    }
  break;

  case WM_MOVE:
  {
      pWindow = EXTRACT_APPWINDOW_PTR(hWnd);
      return pWindow->_OnMove(LOWORD(lParam), HIWORD(lParam));
  }
  break;

  case WM_SIZING:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnSizing( (LPRECT)lParam );
    }
    break;

  case WM_SIZE:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnSize( LOWORD(lParam), HIWORD(lParam) );
    }
    break;

  case WM_ACTIVATE:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnActivate( LOWORD(wParam), HIWORD(wParam)!=0, (HWND)lParam );
    }
    break;

  case WM_ACTIVATEAPP:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnActivateApp( wParam != FALSE );
    }
    break;

  case WM_SETFOCUS:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnSetFocus( (HWND) wParam );
    }
    break;

  case WM_KEYDOWN:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnKeyDown( wParam );
    }
    break;

  case WM_KEYUP:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnKeyUp( wParam );
    }
    break;

  case WM_CHAR:
    {
      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnCharKey( wParam, (lParam & 0x1000)!=0/*bit 24*/ );
    }
    break;

  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MOUSEMOVE:
    {
      int nModifierKeys;
      nModifierKeys  = 0;
      if ((wParam & MK_SHIFT) != 0)
      {
        nModifierKeys |= kmbWindow::MODIFIERKEY_SHIFT;
      }
      if ((wParam & MK_CONTROL) != 0)
      {
        nModifierKeys |= kmbWindow::MODIFIERKEY_CTRL;
      }
      if (GetKeyState(VK_MENU) < 0)
      {
        nModifierKeys |= kmbWindow::MODIFIERKEY_ALT;
      }

      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      switch (uMsg)
      {
        case WM_LBUTTONDOWN:  return pWindow->_OnLButtonDown( LOWORD(lParam), HIWORD(lParam), nModifierKeys ); break;
        case WM_LBUTTONUP:    return pWindow->_OnLButtonUp( LOWORD(lParam), HIWORD(lParam), nModifierKeys ); break;
        case WM_RBUTTONDOWN:  return pWindow->_OnRButtonDown( LOWORD(lParam), HIWORD(lParam), nModifierKeys ); break;
        case WM_RBUTTONUP:    return pWindow->_OnRButtonUp( LOWORD(lParam), HIWORD(lParam), nModifierKeys ); break;
        case WM_MBUTTONDOWN:  return pWindow->_OnMButtonDown( LOWORD(lParam), HIWORD(lParam), nModifierKeys ); break;
        case WM_MBUTTONUP:    return pWindow->_OnMButtonUp( LOWORD(lParam), HIWORD(lParam), nModifierKeys ); break;
        case WM_MOUSEMOVE:    return pWindow->_OnMouseMove( LOWORD(lParam), HIWORD(lParam), nModifierKeys ); break;
      }
    }
    break;

  case WM_MOUSEWHEEL:
    {
      int nModifierKeys;
      nModifierKeys  = 0;
      if ((wParam & MK_SHIFT) != 0)
      {
        nModifierKeys |= kmbWindow::MODIFIERKEY_SHIFT;
      }
      if ((wParam & MK_CONTROL) != 0)
      {
        nModifierKeys |= kmbWindow::MODIFIERKEY_CTRL;
      }
      if (GetKeyState(VK_MENU) < 0)
      {
        nModifierKeys |= kmbWindow::MODIFIERKEY_ALT;
      }

      pWindow = EXTRACT_APPWINDOW_PTR( hWnd );
      return pWindow->_OnMouseWheel( GET_WHEEL_DELTA_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam), nModifierKeys );
    }
    break;

  default:
    bCallDefWindowProc = true;
  }

  if (bCallDefWindowProc)
  {
    return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
  }
  else
  {
    return 0;
  }
}

#undef EXTRACT_APPWINDOW_PTR


//----------------------------------------------------------------------------
/**
** 
*/
/*static*/ void kmbWindow::RegisterWindowClass()
{
  KMASSERTM( !ms_bWindowClassRegistered, ("The static function kmbWindow::RegisterWindowClass must be called just once.") );
  if (!ms_bWindowClassRegistered)
  {
    WNDCLASSEX WndClass;

    // Register the window class
    WndClass.cbSize = sizeof( WNDCLASSEX );
    WndClass.style  = 0/*CS_DBLCLKS*/;
    WndClass.lpfnWndProc = kmbWindowProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = GetModuleHandle(NULL);
    WndClass.hIcon = NULL;
    WndClass.hCursor = NULL;
    WndClass.hbrBackground = NULL; //(HBRUSH)GetStockObject(LTGRAY_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = "KMBWINDOW_WINDOW_CLASS";
    WndClass.hIconSm = NULL;

    ::RegisterClassEx( &WndClass );

    ms_bWindowClassRegistered = true;
  }
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbWindow::Init(const string& sWindowText, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hParentOrOwner)
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Create the window
  if (bRet)
  {
    bRet = CreateWnd( sWindowText, dwStyle, x, y, nWidth, nHeight, hParentOrOwner );
  }

  // Get the default mouse cursor
  if (bRet)
  {
    m_hMouseCursor = (HCURSOR) ::LoadImage( NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED );
  }


  if (bRet)
  {
    m_bOk = true;
  }
  else
  {
    FreeVars();
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** InitDialog
*/
bool kmbWindow::InitDialog()
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Get the default mouse cursor
  if (bRet)
  {
    m_hMouseCursor = (HCURSOR) ::LoadImage( NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED );
  }


  if (bRet)
  {
    m_bOk = true;
  }
  else
  {
    FreeVars();
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void kmbWindow::End()
{
  if( IsOk() )
  {
    FreeVars();
    m_bOk = false;
  }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void kmbWindow::ResetVars()
{
  m_hWnd         = NULL;
  m_hMouseCursor = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbWindow::FreeVars()
{
  DestroyWnd();
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbWindow::CreateWnd(const string& sWindowText, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hParentOrOwner)
{
  KMASSERTM( ms_bWindowClassRegistered, ("The static function kmbWindow::RegisterWindowClass must be called once before creating any kmbWindow.") );

  m_hWnd = ::CreateWindow( "KMBWINDOW_WINDOW_CLASS", sWindowText.c_str(), dwStyle,
                           x, y, nWidth, nHeight,
                           hParentOrOwner, NULL/*hMenu*/, GetModuleHandle(NULL), (LPVOID)this );

  return (m_hWnd != NULL);
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::DestroyWnd()
{
  if (m_hWnd != NULL)
  {
    ::DestroyWindow( m_hWnd );
    m_hWnd = NULL;
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::RequestClose()
{
  ::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::SetText(const string& sText)
{
  ::SetWindowText( m_hWnd, sText.c_str() );
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::SetPosition(long x, long y)
{
  ::SetWindowPos( GetHWnd(), NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::SetSize(long width, long height)
{
  ::SetWindowPos( GetHWnd(), NULL, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::SetRect(const RECT& newRect)
{
  ::SetWindowPos( GetHWnd(), NULL, newRect.left, newRect.top, newRect.right - newRect.left, newRect.bottom - newRect.top, SWP_NOACTIVATE | SWP_NOZORDER );
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::GrabInputFocus()
{
  ::SetFocus( GetHWnd() );
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::CaptureMouse(bool bCapture)
{
  if (bCapture)
  {
    ::SetCapture( GetHWnd() );
  }
  else
  {
    ::ReleaseCapture();
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
HCURSOR kmbWindow::SetMouseCursor(HCURSOR hNewCursor)
{
  HCURSOR hRet = m_hMouseCursor;
  m_hMouseCursor = hNewCursor;
  return hRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::InvalidateRect(const RECT& rRect, bool bEraseBackground)
{
  ::InvalidateRect( GetHWnd(), &rRect, bEraseBackground ? TRUE : FALSE );
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbWindow::InvalidateAll(bool bEraseBackground)
{
  ::InvalidateRect( GetHWnd(), NULL, bEraseBackground ? TRUE : FALSE );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbWindow::_OnWindowProcedureProlog(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return false;       // Continue with the normal message handler.
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnCreate(CREATESTRUCT* pCreateStruct)
{
  KMASSERT( pCreateStruct->lpCreateParams == (LPVOID) this );

  // Store the 'this' pointer in the window's user data field
  ::SetLastError( ERROR_SUCCESS );

  if ( ::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) ) == 0 )
  {
    KMASSERT( ::GetLastError() == ERROR_SUCCESS );
  }

  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnClose()
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnDestroy()
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnControlCommand(WORD /*nCommand*/, WORD /*nCtrlId*/, HWND /*hCtrl*/)
{
  //    switch(nCtrlId)
  //    {
  //        case EDIT_COMMANDLINE_CTRLID:
  //            switch(nCommand)
  //            {
  //                case EN_CHANGE:
  //                    ::MessageBox(m_hConsoleWnd, "EN_CHANGE recibido", "PRUEBA", MB_OK);
  //                    break;
  //            }
  //            break;
  //    }

  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnMenuCommand(WORD nItemId, bool bFromAccelerator)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnPaint(HDC hDc)
{
  return 1; ////0
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnEraseBkgnd()
{
    return 0;  // Default message behavior.
}

//----------------------------------------------------------------------------
/**
**
*/
LRESULT kmbWindow::_OnMove(int clientAreaX, int clientAreaY)
{
    return TRUE;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnSizing(LPRECT /*prRect*/)
{
  return TRUE;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnSize(int iWidth, int iHeight)
{
  return TRUE;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnActivate(int iType, bool bMinimized, HWND /*hPreviousWnd*/)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnActivateApp(bool bActivated)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnSetFocus(HWND hPrevWnd)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnKeyDown(unsigned nVirtualKey)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnKeyUp(unsigned nVirtualKey)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnCharKey(unsigned nChar, bool bIsExtendedKey)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnLButtonDown(short x, short y, int nModifierKeys)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnLButtonUp(short x, short y, int nModifierKeys)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnRButtonDown(short x, short y, int nModifierKeys)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnRButtonUp(short x, short y, int nModifierKeys)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnMButtonDown(short x, short y, int nModifierKeys)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnMButtonUp(short x, short y, int nModifierKeys)
{
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnMouseMove(short x, short y, int nModifierKeys)
{
  // Set the mouse cursor assigned to this window
  ::SetCursor( m_hMouseCursor );

  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
LRESULT kmbWindow::_OnMouseWheel(int nDelta, short x, short y, int nModifierKeys)
{
  return 0;
}
