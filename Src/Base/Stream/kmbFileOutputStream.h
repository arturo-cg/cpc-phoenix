//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBFILEOUTPUTSTREAM_H_
#define _KMBFILEOUTPUTSTREAM_H_


#include "kmbOutputStream.h"

class kmbFile;


/**
** 
*/
class kmbFileOutputStream : public kmbOutputStream
{
public:

                            kmbFileOutputStream       ()  { }
  virtual                  ~kmbFileOutputStream       ()  { End( false ); }

  /** Initializes the stream to write to the file with the specified name.
  *** The file is automatically opened and, in the end, closed. */
  bool                      Init                      (const string& sFileName);
  /** Initializes the stream to write to the given file.
  *** The caller is responsible for opening the file with write access before calling this method and,
  *** after the kmbFileOutputStream is destroyed, closing it. */
  bool                      Init                      (kmbFile* pFile);
  virtual void              End                       (bool bIncludedSuper = true);

  /** Returns the specific type of this kmbOutputStream-derived stream. */
  virtual kmbEStreamType    GetType                   () const  { return STREAMTYPE_FILE; }

  /** Returns the file to which this stream writes the bytes. */
  const kmbFile*            GetDestinationFile        () const  { return m_pFile; }

  /** Writes the given number of bytes. */
  virtual bool              Write                     (const void* pBuffer, unsigned uNumBytes);


private:

  typedef                   kmbOutputStream           Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  kmbFile*                  m_pFile;
  bool                      m_bIsFileOwner;

};

#endif // _KMBFILEOUTPUTSTREAM_H_
