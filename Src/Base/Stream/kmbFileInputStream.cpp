//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbFileInputStream.h"
#include "File/kmbFile.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbFileInputStream::Init(const std::string& sFileName)
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
      bRet = m_pFile->Open( kmbFile::ACCESS_READ, kmbFile::FILEMODE_BINARY );
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
bool kmbFileInputStream::Init(kmbFile* pFile)
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
    bRet = (pFile != NULL) && pFile->IsOpen() && pFile->CanRead();
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
/*virtual*/ void kmbFileInputStream::End(bool bIncludedSuper/* = true*/)
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
void kmbFileInputStream::ResetVars()
{
  m_pFile        = NULL;
  m_bIsFileOwner = false;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbFileInputStream::FreeVars()
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
/*virtual*/ unsigned kmbFileInputStream::GetLength()
{
  return m_pFile->GetLength();
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ bool kmbFileInputStream::IsAtEnd() const
{
  return m_pFile->IsAtEof();
}

//----------------------------------------------------------------------------
/**
**
*/
bool kmbFileInputStream::Seek(unsigned newPosition)
{
    return m_pFile->Seek(int(newPosition));
}

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ bool kmbFileInputStream::Read(void* pBuffer, unsigned uNumBytes)
{
  KMASSERTM( pBuffer != NULL, ("The destination buffer (pBuffer) is NULL.") );
  KMASSERTM( uNumBytes > 0, ("Trying to read 0 bytes.") );

  if ( m_pFile->ReadBytes( pBuffer, uNumBytes ) )
  {
    m_uReadBytesCount += uNumBytes;
    return true;
  }
  else
  {
    return false;
  }
}
