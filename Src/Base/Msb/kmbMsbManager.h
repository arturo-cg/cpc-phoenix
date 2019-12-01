//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBMSBMANAGER_H_
#define _KMBMSBMANAGER_H_


#include "Msb/kmbMsb.h"

class kmbInputStream;


/**
** It is responsible for creating MSBs (destruction is automatic as kmbMsb objects are reference counted).
** Always use this class to create kmbMsb objects, never use new & delete operators.
*/
class kmbMsbManager : public kmbSingleton<kmbMsbManager>
{
public:

                            kmbMsbManager             ()  { m_bOk = false; }
  virtual                  ~kmbMsbManager             ()  { End(); }

  bool                      Init                      ();
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  kmbMsbPtr                 CreateMsbFromFile         (const std::string& sMsbFilename);
  kmbMsbPtr                 CreateMsbFromStream       (kmbInputStream* pStream);

  kmbMsbPtr                 GetUniqueNullMsb          ()  { return m_uniqueNullMsb; }
  kmbMsbPtr                 CreateIntegerMsb          (int iValue);
  kmbMsbPtr                 CreateRealMsb             (double dValue);
  kmbMsbPtr                 CreateStringMsb           (const std::string& sValue);
  kmbMsbPtr                 CreateTaggedMsb           ();
  kmbMsbPtr                 CreateEnumeratedMsb       ();


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();

  kmbMsbPtr                 CreateNullMsb             ();


  bool                      m_bOk;
  kmbMsbPtr                 m_uniqueNullMsb;

};

#endif // _KMBMSBMANAGER_H_
