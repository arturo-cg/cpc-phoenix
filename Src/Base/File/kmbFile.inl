//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
** 
*/
template <typename T>
bool kmbFile::ReadBytes(T* pVar)
{
  ASSERT( pVar != NULL );
  ASSERTM( IsOpen(), ("File must be open before reading from it.") );
  ASSERTM( CanRead(), ("File was not open with read access.") );
  ASSERTM( GetFileMode()==kmbFile::FILEMODE_BINARY, ("Don't call this method when the file is open in text mode.") );

  return ( fread(pVar, sizeof(T), 1, m_pFile) == 1 );
}

//----------------------------------------------------------------------------
/**
** 
*/
template <typename T>
bool kmbFile::WriteBytes(const T& Var)
{
  ASSERT( pVar != NULL );
  ASSERTM( IsOpen(), ("File must be open before reading from it.") );
  ASSERTM( CanWrite(), ("File was not open with write access.") );
  ASSERTM( GetFileMode()==kmbFile::FILEMODE_BINARY, ("Don't call this method when the file is open in text mode.") );

  return ( fwrite(&Var, sizeof(T), 1, m_pFile) == 1 );
}
