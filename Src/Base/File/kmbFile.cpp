//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include <io.h>
#include "kmbFile.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbFile::Init(const string& sFileName)
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if(bRet)
  {
  }

  // Initialize class members
  if(bRet)
  {
    m_sFileName = sFileName;
  }


  if(bRet)
    m_bOk = true;
  else
    FreeVars();

  return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void kmbFile::End()
{
  if( IsOk() )
  {
    FreeVars();
    m_bOk = false;
  }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void kmbFile::ResetVars()
{
  m_sFileName.clear();
  m_pFile = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbFile::FreeVars()
{
  Close();
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::Open(EAccessFlags eAccessFlags, EFileMode eFileMode)
{
  bool bRet = true;

  // Close the file if already open
  Close();

  // Convert open flags
  string sOpenFlags;

  if (bRet)
  {
    if (eAccessFlags == ACCESS_READ_WRITE)
    {
      // Read & write access. If the file exists, its contents are destroyed.
      sOpenFlags = "w+";
    }
    else
      if (eAccessFlags == ACCESS_READ)
    {
      // Read-only access. The file must already exist.
      sOpenFlags = 'r';
    }
    else
    if (eAccessFlags == ACCESS_WRITE)
    {
      // Write-only access. If the file exists, its contents are destroyed.
      sOpenFlags = 'w';
    }
    else
    {
      bRet = false;
      KMASSERT( false );
    }
  }

  if (bRet)
  {
    sOpenFlags += ( eFileMode==FILEMODE_BINARY ? 'b' : 't' );
  }

  // Open the file
  if (bRet)
  {
    m_pFile        = fopen( m_sFileName.c_str(), sOpenFlags.c_str() );
    m_eAccessFlags = eAccessFlags;
    m_eFileMode    = eFileMode;
    bRet           = (m_pFile != NULL);
    KMASSERTM( bRet, ("Could not open the file '%s' with %s access in %s mode.",
                    m_sFileName.c_str(),
                    ( eAccessFlags==ACCESS_READ_WRITE ? "READ&WRITE" : ( eAccessFlags==ACCESS_READ ? "READ" : "WRITE" ) ),
                    ( eFileMode==FILEMODE_BINARY ? "BINARY" : "TEXT" ) ) );
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbFile::Close()
{
  if ( IsOpen() )
  {
    fclose( m_pFile );
    m_pFile = NULL;
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::CanRead() const
{
  return ( IsOpen() && (m_eAccessFlags&ACCESS_READ) );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::CanWrite() const
{
  return ( IsOpen() && (m_eAccessFlags&ACCESS_WRITE) );
}

//----------------------------------------------------------------------------
/**
** 
*/
unsigned kmbFile::GetLength()
{
  return (unsigned) _filelength( _fileno(m_pFile) );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::IsAtEof() const
{
  return ( !IsOpen() || (feof(m_pFile) != 0) );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::Seek(int nPos)
{
  return ( fseek( m_pFile, nPos, SEEK_SET ) == 0 );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::ReadBytes(void* pBuffer, unsigned uNumBytes)
{
  KMASSERT( pBuffer != NULL );
  KMASSERT( uNumBytes > 0 );
  KMASSERTM( IsOpen(), ("File must be open before reading from it.") );
  KMASSERTM( CanRead(), ("File was not open with read access.") );
  KMASSERTM( GetFileMode() == FILEMODE_BINARY, ("Don't call this method when the file is open in text mode.") );

  return ( fread(pBuffer, 1, uNumBytes, m_pFile) == uNumBytes );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::ReadChar(char* pcChar)
{
  KMASSERT( pcChar != NULL );
  KMASSERTM( IsOpen(), ("File must be open before reading from it.") );
  KMASSERTM( CanRead(), ("File was not open with read access.") );
  KMASSERTM( GetFileMode() == FILEMODE_TEXT, ("Don't call this method when the file is open in binary mode.") );

  int nValue;
  nValue  = fgetc( m_pFile );
  *pcChar = (char) nValue;

  return (nValue != EOF);
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::WriteBytes(const void* pBuffer, unsigned uNumBytes)
{
  KMASSERT( pBuffer != NULL );
  KMASSERT( uNumBytes > 0 );
  KMASSERTM( IsOpen(), ("File must be open before reading from it.") );
  KMASSERTM( CanWrite(), ("File was not open with write access.") );
  KMASSERTM( GetFileMode() == FILEMODE_BINARY, ("Don't call this method when the file is open in text mode.") );

  return ( fwrite(pBuffer, 1, uNumBytes, m_pFile) == uNumBytes );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::WriteChar(char cChar)
{
  KMASSERTM( IsOpen(), ("File must be open before reading from it.") );
  KMASSERTM( CanWrite(), ("File was not open with write access.") );
  KMASSERTM( GetFileMode() == FILEMODE_TEXT, ("Don't call this method when the file is open in binary mode.") );

  return ( fputc((int)cChar, m_pFile) != EOF );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbFile::WriteString(const char* pStr)
{
  KMASSERTM( IsOpen(), ("File must be open before reading from it.") );
  KMASSERTM( CanWrite(), ("File was not open with write access.") );
  KMASSERTM( GetFileMode() == FILEMODE_TEXT, ("Don't call this method when the file is open in binary mode.") );

  return ( fputs(pStr, m_pFile) != EOF );
}
