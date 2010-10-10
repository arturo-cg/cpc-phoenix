#ifndef _APPWND_H_
#define _APPWND_H_


#include "EmulatorWnd.h"


class CAppWnd : public CFrameWnd
{
public:

                              CAppWnd                   ();
    virtual                  ~CAppWnd                   ();

    CEmulatorWnd             *GetEmulatorWnd            ()  { return &m_wndEmulator; }


    virtual BOOL              PreCreateWindow           (CREATESTRUCT& cs);
    virtual BOOL              OnCmdMsg                  (UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

#ifdef _DEBUG
    virtual void              AssertValid               () const;
    virtual void              Dump                      (CDumpContext& dc) const;
#endif


private:

    afx_msg int               OnCreate                  (LPCREATESTRUCT lpCreateStruct);
    afx_msg void              OnSetFocus                (CWnd *pOldWnd);


    CStatusBar                m_wndStatusBar;
    CToolBar                  m_wndToolBar;
    CEmulatorWnd              m_wndEmulator;

    DECLARE_DYNAMIC(CAppWnd)
    DECLARE_MESSAGE_MAP()
};

#endif //_APPWND_H_
