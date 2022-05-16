//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbMemoryInputStream.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbMemoryInputStream::Init(const void* pSrcBuffer, unsigned uSrcBufferSize)
{
  bool bRet = true;

  End( false );
  ResetVars();

  // Initialize base class
  if (bRet)
  {
    bRet = Super::Init();
  }

  // Check parameters
  if (bRet)
  {
    bRet = (pSrcBuffer != NULL) && (uSrcBufferSize > 0);
  }

  // Initialize class members
  if (bRet)
  {
    m_pSrcBuffer     = (const char*) pSrcBuffer;
    m_uSrcBufferSize = uSrcBufferSize;
    m_pCurrentPos    = (const char*) pSrcBuffer;
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
/*virtual*/ void kmbMemoryInputStream::End(bool bIncludedSuper/* = true*/)
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
void kmbMemoryInputStream::ResetVars()
{
  m_pSrcBuffer     = NULL;
  m_uSrcBufferSize = 0;
  m_pCurrentPos    = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbMemoryInputStream::FreeVars()
{
  //...
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ bool kmbMemoryInputStream::IsAtEnd() const
{
  return (m_uReadBytesCount >= m_uSrcBufferSize);
}

//----------------------------------------------------------------------------
/**
**
*/
bool kmbMemoryInputStream::Seek(unsigned newPosition)
{
    m_pCurrentPos = m_pSrcBuffer + newPosition;
    return (newPosition < m_uSrcBufferSize);
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ bool kmbMemoryInputStream::Read(void* pBuffer, unsigned uNumBytes)
{
  bool bRet = true;

  KMASSERTM( pBuffer != NULL, ("The destination buffer (pBuffer) is NULL.") );
  KMASSERTM( uNumBytes > 0, ("Trying to read 0 bytes.") );

  // Compute how many bytes to read. If there still are enough space in the buffer, all the uNumBytes bytes will be read.
  unsigned uNumBytesToRead;
  uNumBytesToRead = ( uNumBytes < (m_uSrcBufferSize-m_uReadBytesCount) ? uNumBytes : m_uSrcBufferSize - m_uReadBytesCount );

  // Read the bytes
  if (uNumBytesToRead > 0)
  {
    memcpy( pBuffer, m_pCurrentPos, uNumBytesToRead );
    m_pCurrentPos     += uNumBytesToRead;
    m_uReadBytesCount += uNumBytesToRead;
  }

  return bRet;
}
