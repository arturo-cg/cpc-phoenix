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
    m_hWnd = ::CreateWindowEx( 0/*dwExStyle*/, STATUSCLASSNAME/*lpClassName*/, "CPCPhoenix Status Bar"/*lpWindowName*/,
                               WS_CHILD | WS_VISIBLE/*dwStyle*/, 0/*X*/, 0/*Y*/, 0/*nWidth*/, 0/*nHeight*/, pParentWnd->GetHWnd()/*hWndParent*/,
                               (HMENU) ID_STATUSBAR/*hMenu*/, ::GetModuleHandle(NULL), NULL/*lpParam*/ );
    bRet = (m_hWnd != NULL);
  }

  // Add parts
  if (bRet)
  {
    int aWidths[PART_LAST] = { 500, -1 };
    ::SendMessage( m_hWnd, SB_SETPARTS, PART_LAST/*part count*/, (LPARAM) aWidths );

    SetInsertedDiskText( "\t<No disk inserted>" );
    SetEmulationSpeedText( "\t\tSpeed: ---" );
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
void StatusBar::SetInsertedDiskText(const string& sText)
{
  ::SendMessage( m_hWnd, SB_SETTEXT, PART_INSERTEDDISK | 0/*style*/, (LPARAM) sText.c_str() );
}

//----------------------------------------------------------------------------
/**
** 
*/
void StatusBar::SetEmulationSpeedText(const string& sText)
{
  ::SendMessage( m_hWnd, SB_SETTEXT, PART_EMULATIONSPEED | 0/*style*/, (LPARAM) sText.c_str() );
}
