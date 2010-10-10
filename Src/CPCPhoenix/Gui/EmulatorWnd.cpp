#include "stdafx.h"
#include "CPCPhoenix.h"
#include "EmulatorWnd.h"
#include "WinVideoOutput.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CEmulatorWnd, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
END_MESSAGE_MAP()



//----------------------------------------------------------------------------
/**
** 
*/
CEmulatorWnd::CEmulatorWnd()
{
}

//----------------------------------------------------------------------------
/**
** 
*/
CEmulatorWnd::~CEmulatorWnd()
{
}

//----------------------------------------------------------------------------
/**
** 
*/
BOOL CEmulatorWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
        ::LoadCursor(NULL, IDC_ARROW), NULL/*reinterpret_cast<HBRUSH>(COLOR_WINDOW+1)*/, NULL);

    return TRUE;
}

//----------------------------------------------------------------------------
/**
** 
*/
int CEmulatorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    // Create the memory DC that will be used as the back-buffer
    m_BackBufferDC.CreateCompatibleDC( GetDC() );
    m_BackBufferSize.SetSize( CPC::CVideoOutput::MAX_SCREEN_WIDTH, CPC::CVideoOutput::MAX_SCREEN_HEIGHT );

    // Create a bitmap with size equal to maximum CPC screen size and 32 color bits
    m_BackBufferBmp.CreateBitmap( m_BackBufferSize.cx, m_BackBufferSize.cy, 1, 32, NULL );

    // Everything OK, go on with window creation
    return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
void CEmulatorWnd::OnPaint() 
{
    CPaintDC dc(this);

    // Select the back-buffer bitmap
    CBitmap *pOldBmp;
    pOldBmp = m_BackBufferDC.SelectObject( &m_BackBufferBmp );

//******************************************* TODO - TODO - TODO ************************************************
//******************************************* TODO - TODO - TODO ************************************************
// Here we should paint the emulated video output.

    // Paint a colored rectangle that occupies the whole back-buffer
    CBrush brush;
    CBrush *pOldBrush;
    //brush.CreateStockObject( DKGRAY_BRUSH );
    brush.CreateSolidBrush( RGB(m_nColorBG,m_nColorBG,m_nColorBG) );

    pOldBrush = m_BackBufferDC.SelectObject( &brush );
    m_BackBufferDC.Rectangle( CRect(CPoint(0,0), m_BackBufferSize) );

    m_BackBufferDC.SelectObject( pOldBrush );

    // Copy the back-buffer to the window DC
    CRect rClientArea;
    GetClientRect( &rClientArea );

    dc.StretchBlt( 0, 0, rClientArea.Width(), rClientArea.Height(),
                   &m_BackBufferDC, 0, 0, m_BackBufferSize.cx, m_BackBufferSize.cy, SRCCOPY );
//     dc.BitBlt( 0, 0, rClientArea.Width(), rClientArea.Height(),
//                &m_BackBufferDC, 0, 0, SRCCOPY );
//******************************************* TODO - TODO - TODO ************************************************
//******************************************* TODO - TODO - TODO ************************************************

    // Unselect the back-buffer bitmap
    m_BackBufferDC.SelectObject( /*&pOldBmp*/(CBitmap*)NULL );
}
