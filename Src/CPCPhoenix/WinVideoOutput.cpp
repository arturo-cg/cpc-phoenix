//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "WinVideoOutput.h"



//----------------------------------------------------------------------------
/**
**
*/
CWinVideoOutput::CWinVideoOutput(CPC::CMachine* pMachine) : inherited(pMachine)
{
    //...
}

//----------------------------------------------------------------------------
/**
**
*/
bool CWinVideoOutput::Init()
{
    bool bRet = true;

    ResetVars();

    // Buffers
    if (bRet)
    {
        for (unsigned i = 0; i < BUFFER_COUNT; i++)
        {
            // Create buffer
            unsigned nBackBufferLength;
            nBackBufferLength = CPC::CVideoOutput::BUFFER_WIDTH * CPC::CVideoOutput::BUFFER_HEIGHT * 4/*bytes-per-pixel*/;

            m_pBufferDibBits[i] = new unsigned char[nBackBufferLength];

            // Fill buffer DIB infos
            memset(&m_bufferDibInfo[i], 0, sizeof(m_bufferDibInfo[i]));
            m_bufferDibInfo[i].bmiHeader.biSize = sizeof(m_bufferDibInfo[i]);
            m_bufferDibInfo[i].bmiHeader.biWidth = (LONG)CPC::CVideoOutput::BUFFER_WIDTH;
            m_bufferDibInfo[i].bmiHeader.biHeight = -(LONG)CPC::CVideoOutput::BUFFER_HEIGHT;    // Note: Negative height for a top-down DIB, with its origin at the upper-left corner.
            m_bufferDibInfo[i].bmiHeader.biPlanes = 1;
            m_bufferDibInfo[i].bmiHeader.biBitCount = 32;
            m_bufferDibInfo[i].bmiHeader.biCompression = BI_RGB;
            m_bufferDibInfo[i].bmiHeader.biSizeImage = 0;
            m_bufferDibInfo[i].bmiHeader.biClrUsed = 0;
            m_bufferDibInfo[i].bmiHeader.biClrImportant = 0;

            // Buffer properties
            m_bufferProperties.nWidth = CPC::CVideoOutput::BUFFER_WIDTH;
            m_bufferProperties.nStride = 0;
            m_bufferProperties.eFormat = CPC::CVideoOutput::PIXELFORMAT_B8G8R8X8;

            // Fill it with the debug color.
            ClearBuffer(i, DEBUG_COLOR_ARGB);
        }

        m_nBackBuffer = 0;
        m_nFrontBuffer = m_nBackBuffer;
        m_nBackBuffer = (m_nBackBuffer + 1) % BUFFER_COUNT;
    }

    return bRet;
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinVideoOutput::End()
{
    //...
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void CWinVideoOutput::ResetVars()
{
    for (unsigned i = 0; i < BUFFER_COUNT; i++)
    {
        memset(&m_bufferDibInfo[i], 0, sizeof(m_bufferDibInfo[i]));
        m_pBufferDibBits[i] = NULL;
    }
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void CWinVideoOutput::FreeVars()
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
/*virtual*/ const CPC::CVideoOutput::SBufferProperties& CWinVideoOutput::GetBufferProperties() const
{
    return m_bufferProperties;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ unsigned char* CWinVideoOutput::GetBuffer()
{
    return m_pBufferDibBits[m_nBackBuffer];
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ void CWinVideoOutput::OnBufferComplete()
{
    // Swap buffers.
    m_nFrontBuffer = m_nBackBuffer;
    m_nBackBuffer = (m_nBackBuffer + 1) % BUFFER_COUNT;

    //ClearBuffer(m_nBackBuffer, DEBUG_COLOR_ARGB);
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinVideoOutput::GetOutput(SOutput* pOutput) const
{
    if (pOutput != NULL)
    {
        pOutput->nWidth = m_bufferProperties.nWidth;
        pOutput->nHeight = CPC::CVideoOutput::BUFFER_HEIGHT;
        pOutput->pDibInfo = &m_bufferDibInfo[m_nFrontBuffer];
        pOutput->pDibBits = m_pBufferDibBits[m_nFrontBuffer];
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinVideoOutput::ClearBuffer(unsigned bufferIndex, unsigned nRgb)
{
    unsigned nBackBufferPixelCount;
    nBackBufferPixelCount = CPC::CVideoOutput::BUFFER_WIDTH * CPC::CVideoOutput::BUFFER_HEIGHT;
    unsigned* pPixels = (unsigned*)m_pBufferDibBits[bufferIndex];
    for (unsigned i = 0; i < nBackBufferPixelCount; i++)
    {
        *pPixels++ = nRgb;
    }
}
