//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBBINARYMSBREADER_H_
#define _KMBBINARYMSBREADER_H_


#include "kmbMsb.h"

class kmbInputStream;


/**
** Reads a kmbMsb from a kmbInputStream, in binary format. The stream can be associated to a file, a memory buffer, etc.
*/
class kmbBinaryMsbReader
{
public:

                            kmbBinaryMsbReader        ()  { m_bOk = false; }
  virtual                  ~kmbBinaryMsbReader        ()  { End(); }

  bool                      Init                      (kmbInputStream* pStream);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  kmbMsbPtr                 Read                      ();


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();

  kmbMsbPtr                 ReadMsb                   ();
  kmbMsbPtr                 ReadIntegerMsb            ();
  kmbMsbPtr                 ReadNullMsb               ();
  kmbMsbPtr                 ReadRealMsb               ();
  kmbMsbPtr                 ReadStringMsb             ();
  kmbMsbPtr                 ReadTaggedMsb             ();
  kmbMsbPtr                 ReadEnumeratedMsb         ();

  bool                      ReadVariableLengthString  (std::string* psString);


  bool                      m_bOk;
  kmbInputStream*           m_pStream;
  char*                     m_pszBuffer;
  unsigned                  m_uBufferLength;

};

#endif // _KMBBINARYMSBREADER_H_
