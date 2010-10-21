//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBMEMORYINPUTSTREAM_H_
#define _KMBMEMORYINPUTSTREAM_H_


#include "kmbInputStream.h"


/**
** 
*/
class kmbMemoryInputStream : public kmbInputStream
{
public:

                            kmbMemoryInputStream      ()  { }
  virtual                  ~kmbMemoryInputStream      ()  { End( false ); }

  /** Initializes the stream to read from the given memory buffer.
  *** - The source buffer must exist until this kmbMemoryInputStream is destroyed or, at least, until the last read operation is performed.
  *** - The kmbMemoryInputStream ensures that no bytes will be read past the end of the supplied memory buffer. */
  bool                      Init                      (const void* pSrcBuffer, unsigned uSrcBufferSize);
  virtual void              End                       (bool bIncludedSuper = true);

  /** Returns the specific type of this kmbInputStream-derived stream. */
  virtual kmbEStreamType    GetType                   () const  { return STREAMTYPE_MEMORY; }

  /** Returns the memory buffer from which this stream reads the bytes. */
  const void*               GetSourceMemoryBuffer       () const  { return m_pSrcBuffer; }
  /** Returns the length (in bytes) of the memory buffer from which this stream reads the bytes. */
  unsigned                  GetSourceMemoryBufferLength () const  { return m_uSrcBufferSize; }

  /** Returns the total length of the stream. If its length cannot be determined, like in the case of a stream
  *** that reads from a socket, the method returns 0xFFFFFFFF. */
  virtual unsigned          GetLength                 ()  { return m_uSrcBufferSize; }

  /** Returns true if the last read operation has attempted to read past the end of the source memory buffer. */
  virtual bool              IsAtEnd                   () const;

  /** Reads the given number of bytes. */
  virtual bool              Read                      (void* pBuffer, unsigned uNumBytes);


private:

  typedef                   kmbInputStream            Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  const char*               m_pSrcBuffer;
  unsigned                  m_uSrcBufferSize;
  const char*               m_pCurrentPos;

};

#endif // _KMBMEMORYINPUTSTREAM_H_
