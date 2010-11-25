//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "StatusBar.h"
#include "Window/kmbWindow.h"

#include "resource.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool StatusBar::Init(kmbWindow* pParentWnd)
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    bRet = (pParentWnd != NULL);
  }

  // Create the status bar window
  if (bRet)
  {
    m_hWnd = ::CreateWindowEx( 0/*dwExStyle*/, STATUSCLASSNAME/*lpClassName*/, (LPCTSTR) NULL/*lpWindowName*/,
                               WS_CHILD | WS_VISIBLE/*dwStyle*/, 0/*X*/, 0/*Y*/, 0/*nWidth*/, 0/*nHeight*/, pParentWnd->GetHWnd()/*hWndParent*/,
                               (HMENU) ID_STATUSBAR/*hMenu*/, ::GetModuleHandle(NULL), NULL/*lpParam*/ );
    bRet = (m_hWnd != NULL);
  }

  // Add parts
  if (bRet)
  {
    int aWidths[PART_LAST] = { 500, -1 };
    ::SendMessage( m_hWnd, SB_SETPARTS, PART_LAST/*part count*/, (LPARAM) aWidths );

    SetInsertedDiskName( "" );
    SetEmulationSpeed( 0.f );
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
/*virtual*/ void StatusBar::End()
{
  if ( IsOk() )
  {
    FreeVars();
    m_bOk = false;
  }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void StatusBar::ResetVars()
{
  m_hWnd = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void StatusBar::FreeVars()
{
  //...
}

//----------------------------------------------------------------------------
/**
** 
*/
long StatusBar::GetHeight() const
{
  RECT rRect;
  ::GetWindowRect( m_hWnd, &rRect );
  return (rRect.bottom - rRect.top);
}

//----------------------------------------------------------------------------
/**
** 
*/
void StatusBar::SetInsertedDiskName(const string& sDiskName)
{
  char szText[300];
  if ( !sDiskName.empty() )
  {
    _snprintf_s( szText, sizeof(szText), "\tInserted disk: [%s]", sDiskName.c_str() );
  }
  else
  {
    _snprintf_s( szText, sizeof(szText), "\t<No disk inserted>" );
  }

  ::SendMessage( m_hWnd, SB_SETTEXT, PART_INSERTEDDISK | 0/*style*/, (LPARAM) szText );
}

//----------------------------------------------------------------------------
/**
** 
*/
void StatusBar::SetEmulationSpeed(float fSpeed)
{
  char szText[100];
  _snprintf_s( szText, sizeof(szText), "\tSpeed: %.1f%%", fSpeed );

  ::SendMessage( m_hWnd, SB_SETTEXT, PART_EMULATIONSPEED | 0/*style*/, (LPARAM) szText );
}
