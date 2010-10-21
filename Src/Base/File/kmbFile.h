//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _FILE_H_
#define _FILE_H_


/**
** Represents a physical file. It gives functionality for typical operations
** on files such as opening an existing file or creating a new one, reading
** and writing in text or binary mode and so on.
*/
class kmbFile
{
public:

  enum EAccessFlags
  {
    ACCESS_READ       = 0x01,                        //< Enables read accesses.
    ACCESS_WRITE      = 0x02,                        //< Enables write accesses.
    ACCESS_READ_WRITE = ACCESS_READ | ACCESS_WRITE,  //< Enables both read and write accesses.
  };

  enum EFileMode
  {
    FILEMODE_BINARY = 0,      //< Assumes the file contains binary data. Bytes are read from/written to the file as-is, i.e. they are not modified in any way.
    FILEMODE_TEXT,            //< Assumes the file contains text. Modifications are made to linefeed and end-of-file characters.
  };


                            kmbFile                   ()  { m_bOk = false; }
  virtual                  ~kmbFile                   ()  { End(); }

  /** Initializes the object. It doesn't access the file system at all, it just
  *** binds this object to a file name for its use on subsequent operations. */
  bool                      Init                      (const string& sFileName);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  /** Returns the file name this object is using. */
  const string&             GetFileName               () const  { return m_sFileName; }

  /** Opens the file. Returns true if successful using the specified flags, or false otherwise. */
  bool                      Open                      (EAccessFlags eAccessFlags, EFileMode eFileMode);
  /** Closes the file. */
  void                      Close                     ();

  /** Returns true if the file is open, or false otherwise. */
  bool                      IsOpen                    () const  { return (m_pFile != NULL); }
  /** Returns true if the file is open and has read privileges, or false otherwise. */
  bool                      CanRead                   () const;
  /** Returns true if the file is open and has write privileges, or false otherwise. */
  bool                      CanWrite                  () const;
  /** Returns the file mode (binary or text) in which the file was open. */
  EFileMode                 GetFileMode               () const  { return m_eFileMode; }
  /** Returns the file length, in bytes. */
  unsigned                  GetLength                 ();

  /** Returns true if the last read operation has attempted to read past the end of the file. */
  bool                      IsAtEof                   () const;

  /** Reads a variable. The file must be open in binary mode. The variable size must
  *** be known at compile time; i.e., sizeof(pVar) must return the correct size
  *** (built-in types, structs made of built-in types, etc.). */
  template <typename T>
  bool                      ReadBytes                 (T* pVar);

  /** Reads the given number of bytes. The file must be open in binary mode. */
  bool                      ReadBytes                 (void* pBuffer, unsigned uNumBytes);

  /** Reads a single character. The file must be open in text mode. */
  bool                      ReadChar                  (char* pcChar);

  /** Writes a variable. The file must be open in binary mode with write access. The variable size must
  *** be known at compile time; i.e., sizeof(pVar) must return the correct size
  *** (built-in types, structs made of built-in types, etc.). */
  template <typename T>
  bool                      WriteBytes                (const T& Var);

  /** Writes the given number of bytes. The file must be open in binary mode with write access. */
  bool                      WriteBytes                (const void* pBuffer, unsigned uNumBytes);

  /** Writes a single character. The file must be open in text mode with write access. */
  bool                      WriteChar                 (char cChar);
  /** Writes a string. The file must be open in text mode with write access. */
  bool                      WriteString               (const char* pStr);
  /** Writes a string. The file must be open in text mode with write access. */
  bool                      WriteString               (const string& sStr)  { return WriteString(sStr.c_str()); }


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();


  bool                      m_bOk;

  string                    m_sFileName;
  FILE*                     m_pFile;
  EAccessFlags              m_eAccessFlags;
  EFileMode                 m_eFileMode;

};

#include "kmbFile.inl"


#endif // _FILE_H_
