//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBFILEINPUTSTREAM_H_
#define _KMBFILEINPUTSTREAM_H_


#include "kmbInputStream.h"

class kmbFile;


/**
** 
*/
class kmbFileInputStream : public kmbInputStream
{
public:

                            kmbFileInputStream        ()  { }
  virtual                  ~kmbFileInputStream        ()  { End( false ); }

  /** Initializes the stream to read from the file with the specified name.
  *** The file is automatically opened and, in the end, closed. */
  bool                      Init                      (const std::string& sFileName);
  /** Initializes the stream to read from the given file.
  *** The caller is responsible for opening the file with read access before calling this method and,
  *** after the kmbFileInputStream is destroyed, closing it. */
  bool                      Init                      (kmbFile* pFile);
  virtual void              End                       (bool bIncludedSuper = true);

  /** Returns the specific type of this kmbInputStream-derived stream. */
  virtual kmbEStreamType    GetType                   () const  { return STREAMTYPE_FILE; }

  /** Returns the file from which this stream reads the bytes. */
  const kmbFile*            GetSourceFile             () const  { return m_pFile; }

  /** Returns the total length of the stream. If its length cannot be determined, like in the case of a stream
  *** that reads from a socket, the method returns 0xFFFFFFFF. */
  virtual unsigned          GetLength                 ();

  /** Returns true if the last read operation has attempted to read past the end of the file. */
  virtual bool              IsAtEnd                   () const;

  /** It returns true if the stream allows Seek to be used, or false otherwise.
  *   A kmbFileInputStream always returns true.
  */
  virtual bool              IsSeekAllowed             () const { return true; }
  /** Sets the current position in the stream so that the next read operation starts from there.
  *   The position is relative to the beginning of the stream, e.g. position 0 is the start of the stream.
  *   It does nothing if IsSeekAllowed returns false. */
  virtual bool              Seek                      (unsigned newPosition);

  /** Reads the given number of bytes. */
  virtual bool              Read                      (void* pBuffer, unsigned uNumBytes);


private:

  typedef                   kmbInputStream            Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  kmbFile*                  m_pFile;
  bool                      m_bIsFileOwner;

};

#endif // _KMBFILEINPUTSTREAM_H_
