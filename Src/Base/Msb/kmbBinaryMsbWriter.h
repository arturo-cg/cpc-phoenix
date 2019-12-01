//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBBINARYMSBWRITER_H_
#define _KMBBINARYMSBWRITER_H_


#include "kmbMsb.h"

class kmbOutputStream;


/**
** Writes an existing kmbMsb to a kmbOutputStream, in binary format. The stream can be associated to a file, a memory buffer, etc.
*/
class kmbBinaryMsbWriter
{
public:

                            kmbBinaryMsbWriter        ()  { m_bOk = false; }
  virtual                  ~kmbBinaryMsbWriter        ()  { End(); }

  bool                      Init                      (const kmbMsbPtr& ptrMsb, kmbOutputStream* pStream);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  bool                      Write                     ();


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();

  bool                      WriteMsb                  (const kmbMsb* pMsb);
  bool                      WriteIntegerMsb           (const CIntegerMsb* pIntegerMsb);
  bool                      WriteNullMsb              (const CNullMsb* pNullMsb);
  bool                      WriteRealMsb              (const CRealMsb* pRealMsb);
  bool                      WriteStringMsb            (const CStringMsb* pStringMsb);
  bool                      WriteTaggedMsb            (const CTaggedMsb* pTaggedMsb);
  bool                      WriteEnumeratedMsb        (const CEnumeratedMsb* pEnumeratedMsb);

  bool                      WriteVariableLengthString (const string& sString);


  bool                      m_bOk;
  kmbMsbPtr                 m_ptrMsb;
  kmbOutputStream*          m_pStream;

};

#endif // _KMBBINARYMSBWRITER_H_
