//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBTEXTMSBWRITER_H_
#define _KMBTEXTMSBWRITER_H_


#include "kmbMsb.h"

class kmbOutputStream;


/**
** Writes an existing kmbMsb to a kmbOutputStream, in text format. The stream can be associated to a file, a memory buffer, etc.
*/
class kmbTextMsbWriter
{
public:

                            kmbTextMsbWriter          ()  { m_bOk = false; }
  virtual                  ~kmbTextMsbWriter          ()  { End(); }

  bool                      Init                      (const kmbMsbPtr& ptrMsb, kmbOutputStream* pStream);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  bool                      Write                     ();


private:

  static const unsigned     INDENTATION_INCREMENT;


  void                      ResetVars                 ();
  void                      FreeVars                  ();

  bool                      WriteMsb                  (const kmbMsb* pMsb, unsigned nIndentation);
  bool                      WriteIntegerMsb           (const CIntegerMsb* pIntegerMsb, unsigned nIndentation);
  bool                      WriteNullMsb              (const CNullMsb* pNullMsb, unsigned nIndentation);
  bool                      WriteRealMsb              (const CRealMsb* pRealMsb, unsigned nIndentation);
  bool                      WriteStringMsb            (const CStringMsb* pStringMsb, unsigned nIndentation);
  bool                      WriteTaggedMsb            (const CTaggedMsb* pTaggedMsb, unsigned nIndentation);
  bool                      WriteEnumeratedMsb        (const CEnumeratedMsb* pEnumeratedMsb, unsigned nIndentation);
  bool                      WriteEnumeratedMsbAsVecQuat(const CEnumeratedMsb* pEnumeratedMsb, unsigned nIndentation);
  bool                      WriteSpaces               (unsigned nNumSpaces);

  bool                      IsVecQuat                 (const CEnumeratedMsb* pEnumeratedMsb) const;
  void                      ConvertDoubleToString     (double dValue, string* pResult);


  bool                      m_bOk;
  kmbMsbPtr                 m_ptrMsb;
  kmbOutputStream*          m_pStream;

};

#endif // _KMBTEXTMSBWRITER_H_
