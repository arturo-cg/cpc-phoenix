//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbMemoryOutputStream.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbMemoryOutputStream::Init(void* pDestBuffer, unsigned uDestBufferSize)
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
    bRet = (pDestBuffer != NULL) && (uDestBufferSize > 0);
  }

  // Initialize class members
  if (bRet)
  {
    m_pDestBuffer     = (char*) pDestBuffer;
    m_uDestBufferSize = uDestBufferSize;
    m_pCurrentPos     = (char*) pDestBuffer;
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
/*virtual*/ void kmbMemoryOutputStream::End(bool bIncludedSuper/* = true*/)
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
void kmbMemoryOutputStream::ResetVars()
{
  m_pDestBuffer     = NULL;
  m_uDestBufferSize = 0;
  m_pCurrentPos     = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbMemoryOutputStream::FreeVars()
{
  //...
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ bool kmbMemoryOutputStream::Write(const void* pBuffer, unsigned uNumBytes)
{
  bool bRet = true;

  KMASSERTM( pBuffer != NULL, ("The source buffer (pBuffer) is NULL.") );
  KMASSERTM( uNumBytes > 0, ("Trying to write 0 bytes.") );

  // Compute how many bytes to write. If there still are enough space in the buffer, all the uNumBytes bytes will be written.
  unsigned uNumBytesToWrite;
  uNumBytesToWrite = ( uNumBytes < (m_uDestBufferSize-m_uWrittenBytesCount) ? uNumBytes : m_uDestBufferSize - m_uWrittenBytesCount );

  // Write the bytes
  if (uNumBytesToWrite > 0)
  {
    memcpy( m_pCurrentPos, pBuffer, uNumBytesToWrite );
    m_pCurrentPos        += uNumBytesToWrite;
    m_uWrittenBytesCount += uNumBytesToWrite;
  }

  return bRet;
}
