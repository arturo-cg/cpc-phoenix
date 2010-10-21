//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBINPUTSTREAM_H_
#define _KMBINPUTSTREAM_H_


#include "kmbStreamTypes.h"


/**
** Base class of all classes representing a binary stream from which bytes of data can be read.
** There are derived classes for reading from a file, a memory buffer, etc.
*/
class kmbInputStream
{
public:

  virtual                  ~kmbInputStream            ()  { End(); }

  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  /** Returns the specific type of this kmbInputStream-derived stream. */
  virtual kmbEStreamType    GetType                   () const = 0;

  /** Returns the total length of the stream. If its length cannot be determined, like in the case of a stream
  *** that reads from a socket, the method returns 0xFFFFFFFF. */
  virtual unsigned          GetLength                 () = 0;
  /** Returns the count of bytes read so far. */
  unsigned                  GetReadBytesCount         () const  { return m_uReadBytesCount; }

  /** Returns true if the last read operation has attempted to read past the end of the stream. */
  virtual bool              IsAtEnd                   () const = 0;

  /** Reads a variable. The variable size must be known at compile time; i.e., sizeof(pVar) must return
  *** the correct size (built-in types, structs made of built-in types, etc.). */
  template <typename T>
  bool                      Read                      (T* pVar);

  /** Reads the given number of bytes. */
  virtual bool              Read                      (void* pBuffer, unsigned uNumBytes) = 0;


protected:

  // Constructor and Init() are protected to avoid instantiating this class
                            kmbInputStream            ()  { m_bOk = false; }
  bool                      Init                      ();


  bool                      m_bOk;
  unsigned                  m_uReadBytesCount;


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();

};

#include "kmbInputStream.inl"


#endif // _KMBINPUTSTREAM_H_
