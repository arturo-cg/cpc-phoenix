//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbFileOutputStream.h"
#include "File/kmbFile.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbFileOutputStream::Init(const string& sFileName)
{
  bool bRet = true;

  End( false );
  ResetVars();

  // Initialize base class
  if (bRet)
  {
    bRet = Super::Init();
  }

  // Open the target file
  if (bRet)
  {
    m_bIsFileOwner = true;

    m_pFile = new kmbFile;
    bRet = m_pFile->Init( sFileName );

    if (bRet)
    {
      bRet = m_pFile->Open( kmbFile::ACCESS_WRITE, kmbFile::FILEMODE_BINARY );
    }
  }

  // Initialize other class members
  if (bRet)
  {
    //...
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
** Init
*/
bool kmbFileOutputStream::Init(kmbFile* pFile)
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
    bRet = (pFile != NULL) && pFile->IsOpen() && pFile->CanWrite();
  }

  // Initialize class members
  if (bRet)
  {
    m_pFile        = pFile;
    m_bIsFileOwner = false;
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
/*virtual*/ void kmbFileOutputStream::End(bool bIncludedSuper/* = true*/)
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
void kmbFileOutputStream::ResetVars()
{
  m_pFile        = NULL;
  m_bIsFileOwner = false;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbFileOutputStream::FreeVars()
{
  // Close and destroy the file only if we own it
  if ( (m_pFile != NULL) && m_bIsFileOwner )
  {
    m_pFile->Close();
    KMBDISPOSE( m_pFile );
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ bool kmbFileOutputStream::Write(const void* pBuffer, unsigned uNumBytes)
{
  ASSERTM( pBuffer != NULL, ("The source buffer (pBuffer) is NULL.") );
  ASSERTM( uNumBytes > 0, ("Trying to write 0 bytes.") );

  if ( m_pFile->WriteBytes( pBuffer, uNumBytes ) )
  {
    m_uWrittenBytesCount += uNumBytes;
    return true;
  }
  else
  {
    return false;
  }
}
