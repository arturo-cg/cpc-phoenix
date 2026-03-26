//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbDialogBox.h"



//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#define EXTRACT_KMBDIALOGBOX_PTR(hWnd)   (reinterpret_cast<kmbDialogBox*>( ::GetWindowLongPtr(hWnd, GWLP_USERDATA) ))

INT_PTR CALLBACK kmbDialogProc(HWND hDlgWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{ 
  switch (nMessage)
  { 
    case WM_INITDIALOG:
      {
        // Store the 'this' pointer in the dialog's user data field
        kmbDialogBox* pDlg;
        pDlg = reinterpret_cast<kmbDialogBox*>( lParam );
        pDlg->_SetHWnd( hDlgWnd );

        ::SetLastError( ERROR_SUCCESS );
        if ( ::SetWindowLongPtr( hDlgWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlg) ) == 0 )
        {
          KMASSERT( ::GetLastError() == ERROR_SUCCESS );
        }

        // Pass the message notification to the kmbDialogProc object
        pDlg->_OnDlgInitDialog();
        return 0;    // Returns 0 to prevent the system from setting the default keyboard focus.
      }
      break;

    case WM_COMMAND:
      {
        kmbDialogBox* pDlg;
        pDlg = EXTRACT_KMBDIALOGBOX_PTR( hDlgWnd );

        if (lParam == 0)   // 0: From a menu or from an accelerator; >= 1 From a control.
        {
          pDlg->_OnMenuCommand( LOWORD(wParam), HIWORD(wParam)==1 );
        }
        else
        {
          switch ( HIWORD(wParam) )
          {
            case BN_CLICKED:           pDlg->_OnButtonClicked( LOWORD(wParam), (HWND) lParam ); break;
            case EN_SETFOCUS:          pDlg->_OnEditBoxSetFocus( LOWORD(wParam), (HWND) lParam ); break;
            case EN_KILLFOCUS:         pDlg->_OnEditBoxKillFocus( LOWORD(wParam), (HWND) lParam ); break;
            //default:                   pDlg->_OnControlCommand( HIWORD(wParam), LOWORD(wParam), (HWND)lParam );
          }
        }

        return TRUE;
      }
      break;

    case WM_NOTIFY:
      {
        kmbDialogBox* pDlg;
        pDlg = EXTRACT_KMBDIALOGBOX_PTR( hDlgWnd );

        LPNMHDR pHdr;
        pHdr = (LPNMHDR) lParam;
        switch (pHdr->code)
        {
          case LVN_GETDISPINFO:   pDlg->_OnListViewGetDispInfo( (NMLVDISPINFO*) lParam, static_cast<WORD>(pHdr->idFrom), pHdr->hwndFrom ); break;
          case LVN_ITEMCHANGING:  return pDlg->_OnListViewItemChanging( (NMLISTVIEW*) lParam, static_cast<WORD>(pHdr->idFrom), pHdr->hwndFrom ); break;
          case LVN_ITEMCHANGED:   pDlg->_OnListViewItemChanged( (NMLISTVIEW*) lParam, static_cast<WORD>(pHdr->idFrom), pHdr->hwndFrom ); break;
          case LVN_ITEMACTIVATE:  pDlg->_OnListViewItemActivate( (NMITEMACTIVATE*) lParam, static_cast<WORD>(pHdr->idFrom), pHdr->hwndFrom ); break;
          case NM_CLICK:          pDlg->_OnGenericControlClick( (void*) lParam, static_cast<WORD>(pHdr->idFrom), pHdr->hwndFrom ); break;
          case NM_DBLCLK:         pDlg->_OnGenericControlDoubleClick( (void*) lParam, static_cast<WORD>(pHdr->idFrom), pHdr->hwndFrom ); break;
        }
      }
      break;
  } 

  return FALSE;    // Tells Windows to process the message.
} 

#undef EXTRACT_KMBDIALOGBOX_PTR

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
** InitModeless
*/
bool kmbDialogBox::InitModeless(LPCTSTR lpTemplate, HWND hOwnerWnd)
{
  bool bRet = true;

  End( false );
  ResetVars();

  // Initialize base class
  if (bRet)
  {
    bRet = Super::InitDialog();
  }

  // Check parameters
  if (bRet)
  {
    bRet = (::IsWindow(hOwnerWnd) != 0);
  }

  // Initialize class members
  if (bRet)
  {
    m_hOwnerWnd       = hOwnerWnd;
    m_lpModalTemplate = lpTemplate;
  }

  // Create the modeless dialog box
  if (bRet)
  {
    if (::CreateDialogParam( ::GetModuleHandle(NULL), lpTemplate, hOwnerWnd, kmbDialogProc, (LPARAM) this ) == NULL)
    {
      KMASSERTM( false, ("Error creating a modeless dialog box: GetLastError() == %d", ::GetLastError()) );
      bRet = false;
    }
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
** InitModal
*/
bool kmbDialogBox::InitModal(LPCTSTR lpTemplate, HWND hOwnerWnd)
{
  bool bRet = true;

  End( false );
  ResetVars();

  // Initialize base class
  if (bRet)
  {
    bRet = Super::InitDialog();
  }

  // Check parameters
  if (bRet)
  {
    bRet = (::IsWindow(hOwnerWnd) != 0);
  }

  // Initialize class members
  if (bRet)
  {
    m_hOwnerWnd       = hOwnerWnd;
    m_lpModalTemplate = lpTemplate;
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
/*virtual*/ void kmbDialogBox::End(bool bIncludedSuper/* = true*/)
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
void kmbDialogBox::ResetVars()
{
  m_hOwnerWnd       = NULL;
  m_lpModalTemplate = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbDialogBox::FreeVars()
{
  //...
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbDialogBox::EModalReturnValue kmbDialogBox::ShowModal()
{
  EModalReturnValue eRet;
  INT_PTR          nResult;

  // Create and show the modal dialog box. The call doesn't return until the user closes the dialog box.
  nResult = ::DialogBoxParam( GetModuleHandle(NULL), m_lpModalTemplate, m_hOwnerWnd, kmbDialogProc, (LPARAM) this );
  KMASSERTM( nResult != -1, ("Error creating a modal dialog box: GetLastError() == %d", ::GetLastError()) );

  eRet = (EModalReturnValue) nResult;
  return eRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbDialogBox::EndModal(EModalReturnValue eReturnValue)
{
  ::EndDialog( GetHWnd(), eReturnValue );
}
