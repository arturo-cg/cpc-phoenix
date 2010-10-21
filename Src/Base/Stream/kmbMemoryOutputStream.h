//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBMEMORYOUTPUTSTREAM_H_
#define _KMBMEMORYOUTPUTSTREAM_H_


#include "kmbOutputStream.h"


/**
** 
*/
class kmbMemoryOutputStream : public kmbOutputStream
{
public:

                            kmbMemoryOutputStream     ()  { }
  virtual                  ~kmbMemoryOutputStream     ()  { End( false ); }

  /** Initializes the stream to write to the given memory buffer.
  *** - The caller is responsible for allocating a large enough memory buffer (of at least uBufferSize bytes)
  ***   and freeing it after this kmbMemoryOutputStream has been destroyed.
  *** - The kmbMemoryOutputStream ensures that no bytes will be written past the end of the supplied memory buffer. */
  bool                      Init                      (void* pDestBuffer, unsigned uDestBufferSize);
  virtual void              End                       (bool bIncludedSuper = true);

  /** Returns the specific type of this kmbOutputStream-derived stream. */
  virtual kmbEStreamType    GetType                   () const  { return STREAMTYPE_MEMORY; }

  /** Returns the memory buffer to which this stream writes the bytes. */
  const void*               GetDestinationMemoryBuffer       () const  { return m_pDestBuffer; }
  /** Returns the length (in bytes) of the memory buffer to which this stream writes the bytes. */
  unsigned                  GetDestinationMemoryBufferLength () const  { return m_uDestBufferSize; }

  /** Writes the given number of bytes. */
  virtual bool              Write                     (const void* pBuffer, unsigned uNumBytes);


private:

  typedef                   kmbOutputStream           Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  char*                     m_pDestBuffer;
  unsigned                  m_uDestBufferSize;
  char*                     m_pCurrentPos;

};

#endif // _KMBMEMORYOUTPUTSTREAM_H_
