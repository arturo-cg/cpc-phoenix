#include "stdafx.h"
#include "CPCPhoenix.h"
#include "EmulatorWnd.h"
#include "cpcMachine.h"
#include "cpcDisplay.h"


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
  m_pEmulatedMachine = NULL;
  m_pBackBuffer      = NULL;
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
  // Create the back-buffer
  unsigned nBackBufferLength;
  nBackBufferLength = CPC::CDisplay::IMAGEBUFFER_WIDTH * CPC::CDisplay::IMAGEBUFFER_HEIGHT * 4/*bytes-per-pixel*/;

  m_pBackBuffer = new unsigned char [nBackBufferLength];

  // Create a CBitmap to hold the back-buffer and tell it to grab pixels from m_pBackBuffer
  CSize bitmapSize;
  bitmapSize.SetSize( CPC::CDisplay::IMAGEBUFFER_WIDTH, CPC::CDisplay::IMAGEBUFFER_HEIGHT );

  ////////m_BackBufferBitmap.CreateBitmap( bitmapSize.cx, bitmapSize.cy, 1/*nPlanes*/, 32/*nBitCount = bpp*/, m_pBackBuffer/*lpBits*/ );
  m_BackBufferBitmap.CreateCompatibleBitmap( GetDC(), bitmapSize.cx, bitmapSize.cy );

//*************************************** TODO - TODO - TODO *********************************************
//*************************************** TODO - TODO - TODO *********************************************
  // Check device context format and tell CPC::CDisplay to decode the image in the same format.
  BITMAP bitmapInfo;
  m_BackBufferBitmap.GetBitmap( &bitmapInfo );
//*************************************** TODO - TODO - TODO *********************************************
//*************************************** TODO - TODO - TODO *********************************************

  // Create the memory DC that will be used as the back-buffer
  m_BackBufferDC.CreateCompatibleDC( GetDC() );
  m_BackBufferDC.SelectObject( &m_BackBufferBitmap );

  // Everything OK, go on with window creation
  return 0;
}

//----------------------------------------------------------------------------
/**
** 
*/
void CEmulatorWnd::UpdateDisplayImage()
{
  // Tell the emulated machine to decode the current frame
  m_pEmulatedMachine->GetDisplay()->DecodeImage_B8G8R8X8( m_pBackBuffer );
  m_BackBufferBitmap.SetBitmapBits( CPC::CDisplay::IMAGEBUFFER_WIDTH * CPC::CDisplay::IMAGEBUFFER_HEIGHT * 4/*bytes-per-pixel*/, m_pBackBuffer );
}

//----------------------------------------------------------------------------
/**
** 
*/
void CEmulatorWnd::OnPaint() 
{
  CPaintDC dc(this);

  // Copy the back-buffer to the window DC
  CRect rClientArea;
  GetClientRect( &rClientArea );

  dc.StretchBlt( 0, 0, rClientArea.Width(), rClientArea.Height(),
                 &m_BackBufferDC, 0, 0, CPC::CDisplay::IMAGEBUFFER_WIDTH, CPC::CDisplay::IMAGEBUFFER_HEIGHT, SRCCOPY );
  //     dc.BitBlt( 0, 0, rClientArea.Width(), rClientArea.Height(),
  //                &m_BackBufferDC, 0, 0, SRCCOPY );
}
