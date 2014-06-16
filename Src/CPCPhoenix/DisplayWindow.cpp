//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "DisplayWindow.h"
#include "Application.h"
#include "cpcMachine.h"
#include "cpcDisplay.h"

#include <Windows.h>
#include "resource.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool DisplayWindow::Init(const RECT& placement, kmbWindow* pParent)
{
  bool bRet = true;

  End( false );
  ResetVars();

  // Initialize base class
  if (bRet)
  {
    DWORD dwStyles;
    dwStyles = (WS_CHILD | WS_VISIBLE);

    bRet = Super::Init( "", dwStyles, placement.left, placement.top, placement.right, placement.bottom, pParent->GetHWnd() );
  }

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Buffers
  if (bRet)
  {
    for (unsigned i = 0; i < BUFFER_COUNT; i++)
    {
      // Create buffer
      unsigned nBackBufferLength;
      nBackBufferLength = CPC::CDisplay::IMAGEBUFFER_WIDTH * CPC::CDisplay::IMAGEBUFFER_HEIGHT * 4/*bytes-per-pixel*/;

      m_pBufferDibBits[i] = new unsigned char [nBackBufferLength];

      // Fill buffer DIB infos
      memset( &m_bufferDibInfo[i], 0, sizeof(m_bufferDibInfo[i]) );
      m_bufferDibInfo[i].bmiHeader.biSize = sizeof(m_bufferDibInfo[i]);
      m_bufferDibInfo[i].bmiHeader.biWidth = (LONG) CPC::CDisplay::IMAGEBUFFER_WIDTH;
      m_bufferDibInfo[i].bmiHeader.biHeight = -(LONG) CPC::CDisplay::IMAGEBUFFER_HEIGHT;    // Note: Negative height for a top-down DIB, with its origin at the upper-left corner.
      m_bufferDibInfo[i].bmiHeader.biPlanes = 1;
      m_bufferDibInfo[i].bmiHeader.biBitCount = 32;
      m_bufferDibInfo[i].bmiHeader.biCompression = BI_RGB;
      m_bufferDibInfo[i].bmiHeader.biSizeImage = 0;
      m_bufferDibInfo[i].bmiHeader.biClrUsed = 0;
      m_bufferDibInfo[i].bmiHeader.biClrImportant = 0;
    }

    m_nBackBuffer = 0;
    m_nFrontBuffer = m_nBackBuffer;
    m_nBackBuffer = (m_nBackBuffer + 1) % BUFFER_COUNT;
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
/*virtual*/ void DisplayWindow::End(bool bIncludedSuper/* = true*/)
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
void DisplayWindow::ResetVars()
{
  for (unsigned i = 0; i < BUFFER_COUNT; i++)
  {
    memset( &m_bufferDibInfo[i], 0, sizeof(m_bufferDibInfo[i]) );
    m_pBufferDibBits[i] = NULL;
  }
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void DisplayWindow::FreeVars()
{
  for (unsigned i = 0; i < BUFFER_COUNT; i++)
  {
    delete m_pBufferDibBits[i];
    m_pBufferDibBits[i] = NULL;
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
void DisplayWindow::UpdateDisplayImage()
{
  // Decode current frame on the back-buffer.
  Application::Singleton()->GetEmulatedMachine()->GetDisplay()->DecodeImage_B8G8R8X8( m_pBufferDibBits[m_nBackBuffer] );

  // Move to next buffer.
  m_nFrontBuffer = m_nBackBuffer;
  m_nBackBuffer = (m_nBackBuffer + 1) % BUFFER_COUNT;
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ LRESULT DisplayWindow::_OnPaint(HDC hDc)
{
  // Copy the back-buffer DIB to the window DC
  RECT rClientArea;
  GetClientRect( &rClientArea );

  ::StretchDIBits( hDc, 0, 0, rClientArea.right - rClientArea.left, rClientArea.bottom - rClientArea.top,
                   0, 0, CPC::CDisplay::IMAGEBUFFER_WIDTH, CPC::CDisplay::IMAGEBUFFER_HEIGHT,
                   m_pBufferDibBits[m_nFrontBuffer], &m_bufferDibInfo[m_nFrontBuffer], DIB_RGB_COLORS, SRCCOPY );

  return 0;
}
