//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
** 
*/
template <typename T>
bool kmbFile::ReadBytes(T* pVar)
{
  KMASSERT( pVar != NULL );
  KMASSERTM( IsOpen(), ("File must be open before reading from it.") );
  KMASSERTM( CanRead(), ("File was not open with read access.") );
  KMASSERTM( GetFileMode()==kmbFile::FILEMODE_BINARY, ("Don't call this method when the file is open in text mode.") );

  return ( fread(pVar, sizeof(T), 1, m_pFile) == 1 );
}

//----------------------------------------------------------------------------
/**
** 
*/
template <typename T>
bool kmbFile::WriteBytes(const T& Var)
{
  KMASSERTM( IsOpen(), ("File must be open before reading from it.") );
  KMASSERTM( CanWrite(), ("File was not open with write access.") );
  KMASSERTM( GetFileMode()==kmbFile::FILEMODE_BINARY, ("Don't call this method when the file is open in text mode.") );

  return ( fwrite(&Var, sizeof(T), 1, m_pFile) == 1 );
}
