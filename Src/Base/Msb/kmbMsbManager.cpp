//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbMsbManager.h"
#include "kmbTextMsbReader.h"
#include "Stream/kmbFileInputStream.h"



template<> kmbMsbManager* kmbSingleton<kmbMsbManager>::m_pSingleton = NULL;



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbMsbManager::Init()
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
  }

  // Create the unique instance of CNullMsb
  if (bRet)
  {
    m_uniqueNullMsb = CreateNullMsb();
  }


  if (bRet)
  {
    m_bOk = true;
  }
  else
  {
    FreeVars();
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void kmbMsbManager::End()
{
  if ( IsOk() )
  {
    FreeVars();
    m_bOk = false;
  }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void kmbMsbManager::ResetVars()
{
  m_uniqueNullMsb = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbMsbManager::FreeVars()
{
  m_uniqueNullMsb = NULL;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateMsbFromFile(const std::string& sMsbFilename)
{
  kmbMsbPtr ptrRet = NULL;

  kmbFileInputStream stream;
  if ( stream.Init(sMsbFilename) )
  {
    ptrRet = CreateMsbFromStream( &stream );
  }
  else
  {
    KMASSERTM( false, ("Error opening file '%s' to read a MSB.", sMsbFilename.c_str()) );
  }

  return ptrRet;
}


//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateMsbFromStream(kmbInputStream* pStream)
{
  kmbMsbPtr        ptrRet = NULL;
  kmbTextMsbReader TextMsbReader;

  if(TextMsbReader.Init(pStream))
  {
    ptrRet = TextMsbReader.GetRootMsb();
  }
  else
  {
    ptrRet = CreateNullMsb();
  }

  return ptrRet;
}


//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateNullMsb()
{
  CNullMsb *pRet;

  pRet = new CNullMsb;
  pRet->Init();

  return pRet;
}


//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateIntegerMsb(int iValue)
{
  CIntegerMsb *pRet;

  pRet = new CIntegerMsb;
  pRet->Init(iValue);

  return pRet;
}


//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateRealMsb(double dValue)
{
  CRealMsb *pRet;

  pRet = new CRealMsb;
  pRet->Init(dValue);

  return pRet;
}


//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateStringMsb(const std::string& sValue)
{
  CStringMsb *pRet;

  pRet = new CStringMsb;
  pRet->Init(sValue);

  return pRet;
}


//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateTaggedMsb()
{
  CTaggedMsb *pRet;

  pRet = new CTaggedMsb;
  pRet->Init();

  return pRet;
}


//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbMsbManager::CreateEnumeratedMsb()
{
  CEnumeratedMsb *pRet;

  pRet = new CEnumeratedMsb;
  pRet->Init();

  return pRet;
}
