// AppWnd.cpp : implementation of the CAppWnd class
//

#include "stdafx.h"
#include "CPCPhoenix.h"

#include "AppWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppWnd

IMPLEMENT_DYNAMIC(CAppWnd, CFrameWnd)

BEGIN_MESSAGE_MAP(CAppWnd, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};


// CAppWnd construction/destruction

CAppWnd::CAppWnd()
{
    // TODO: add member initialization code here
}

CAppWnd::~CAppWnd()
{
}


int CAppWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create the window that will show the emulator output
    if( !m_wndEmulator.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 50, 50), this, AFX_IDW_PANE_FIRST, NULL) )
    {
        TRACE0("Failed to create view window\n");
        return -1;
    }
    
    if( !m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME) )
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    // TODO: Delete these three lines if you don't want the toolbar to be dockable
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    return 0;
}

BOOL CAppWnd::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);
    return TRUE;
}


// CAppWnd diagnostics

#ifdef _DEBUG
void CAppWnd::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CAppWnd::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CAppWnd message handlers

void CAppWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
    // forward focus to the view window
    m_wndEmulator.SetFocus();
}

BOOL CAppWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    // let the view have first crack at the command
    if (m_wndEmulator.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return TRUE;

    // otherwise, do default handling
    return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
