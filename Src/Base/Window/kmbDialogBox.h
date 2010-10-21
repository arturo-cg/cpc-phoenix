//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBDIALOGBOX_H_
#define _KMBDIALOGBOX_H_


#include "kmbWindow.h"


/**
** A Windows dialog box wrapper. The dialog box can be modal or modeless:
**    - Modeless: Just call InitModeless. This creates the underlying Windows dialog box.
**    - Modal: Call InitModal and then call ShowModal, which actually shows the modal dialog box.
*/
class kmbDialogBox : public kmbWindow
{
public:

  enum EModalReturnValue
  {
    MODAL_OK     = 1,  // Results from a modal dialog box should be processed.
    MODAL_CANCEL = 2   // Results from a modal dialog box should be ignored (because the user pressed the Cancel button, for example).
  };


                            kmbDialogBox              ()  { }
  virtual                  ~kmbDialogBox              ()  { End( false ); }

  //
  // --- Modeless dialog box exclusive methods ---
  //

  /** Creates the dialog box as modeless. */
  bool                      InitModeless              (LPCTSTR lpTemplate, HWND hOwnerWnd);


  //
  // --- Modal dialog box exclusive methods ---
  //

  /** Initializes the class but doesn't create the dialog box yet. To do so, you then must call ShowModal.
  *** The lpTemplate parameter can be either the name of the template or, most typically, an integer
  *** generated with the MAKEINTRESOURCE macro. */
  bool                      InitModal                 (LPCTSTR lpTemplate, HWND hOwnerWnd);

  /** Creates and shows the dialog box as modal. The call will not return until the user closes the dialog box.
  *** The return value is MODAL_OK to indicate that the results should be processed and MODAL_CANCEL when
  *** they should be ignored (i.e. the user pressed the Cancel button).
  *** When the return value is MODAL_OK, the derived class must provide specific methods for getting the results. */
  EModalReturnValue         ShowModal                 ();
  /** Terminates a modal dialog box that is executing ShowModal.
  *** Specify MODAL_OK as the return value to indicate that the results should be processed and MODAL_CANCEL when
  *** they should be ignored (i.e. the user pressed the Cancel button). */
  void                      EndModal                  (EModalReturnValue eReturnValue);


  //
  // --- Common methods ---
  //

  virtual void              End                       (bool bIncludedSuper = true);


  //
  // Message handlers (INTERNAL USE ONLY)
  //

  virtual void              _OnDlgInitDialog          ()  { }
  //--- Button ---//
  virtual void              _OnButtonClicked          (WORD nCtrlId, HWND hCtrlWnd)  { }
  //--- Edit Box ---//
  virtual void              _OnEditBoxSetFocus        (WORD nCtrlId, HWND hCtrlWnd)  { }
  virtual void              _OnEditBoxKillFocus       (WORD nCtrlId, HWND hCtrlWnd)  { }
  //--- List View ---//
  virtual void              _OnListViewGetDispInfo    (NMLVDISPINFO* pDispInfo, WORD nCtrlId, HWND hCtrlWnd)  { }
  virtual int               _OnListViewItemChanging   (NMLISTVIEW* pInfo, WORD nCtrlId, HWND hCtrlWnd)  { return TRUE; }
  virtual void              _OnListViewItemChanged    (NMLISTVIEW* pInfo, WORD nCtrlId, HWND hCtrlWnd)  { }
  virtual void              _OnListViewItemActivate   (NMITEMACTIVATE* pItemActivate, WORD nCtrlId, HWND hCtrlWnd)  { }
  //--- Generic Control ---//
  virtual void              _OnGenericControlClick    (void* pInfo, WORD nCtrlId, HWND hCtrlWnd)  { }
  virtual void              _OnGenericControlDoubleClick(void* pInfo, WORD nCtrlId, HWND hCtrlWnd)  { }


private:

  typedef                   kmbWindow                 Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  HWND                      m_hOwnerWnd;
  LPCTSTR                   m_lpModalTemplate;

};

#endif // _KMBDIALOGBOX_H_
