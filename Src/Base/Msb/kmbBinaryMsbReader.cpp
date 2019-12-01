//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbBinaryMsbReader.h"
#include "Stream/kmbInputStream.h"
#include "Msb/kmbMsbManager.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbBinaryMsbReader::Init(kmbInputStream* pStream)
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    bRet = (pStream != NULL) && pStream->IsOk();
    KMASSERTM( bRet, ("The given input stream is not valid.") );
  }

  // Initialize class members
  if (bRet)
  {
    m_pStream = pStream;

    m_uBufferLength = 100;      // Initial buffer length
    m_pszBuffer = new char [m_uBufferLength];
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
/*virtual*/ void kmbBinaryMsbReader::End()
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
void kmbBinaryMsbReader::ResetVars()
{
  m_pStream       = NULL;
  m_pszBuffer     = NULL;
  m_uBufferLength = 0;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbBinaryMsbReader::FreeVars()
{
  delete [] m_pszBuffer;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::Read()
{
  kmbMsbPtr ptrRet;
  ptrRet = ReadMsb();
  return ptrRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::ReadMsb()
{
  kmbMsbPtr ptrRet;

  // Read the MSB type
  kmbMsb::EType eType = kmbMsb::TYPE_INVALID;
  if ( m_pStream->Read(&eType) )
  {
    // Read the MSB itself
    switch (eType)
    {
    case kmbMsb::TYPE_INTEGER:
      ptrRet = ReadIntegerMsb();
      break;

    case kmbMsb::TYPE_NULL:
      ptrRet = ReadNullMsb();
      break;

    case kmbMsb::TYPE_REAL:
      ptrRet = ReadRealMsb();
      break;

    case kmbMsb::TYPE_STRING:
      ptrRet = ReadStringMsb();
      break;

    case kmbMsb::TYPE_TAGGED:
      ptrRet = ReadTaggedMsb();
      break;

    case kmbMsb::TYPE_ENUMERATED:
      ptrRet = ReadEnumeratedMsb();
      break;

    default:
      KMASSERTM( false, ("MSB type not implemented!") );
    }
  }

  return ptrRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::ReadIntegerMsb()
{
  kmbMsbPtr ptrRet;

  int nValue;
  if ( m_pStream->Read(&nValue) )
  {
    ptrRet = kmbMsbManager::Singleton()->CreateIntegerMsb( nValue );
  }

  return ptrRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::ReadNullMsb()
{
  return kmbMsbManager::Singleton()->GetUniqueNullMsb();
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::ReadRealMsb()
{
  kmbMsbPtr ptrRet;

  float fValue;
  if ( m_pStream->Read(&fValue) )
  {
    ptrRet = kmbMsbManager::Singleton()->CreateRealMsb( fValue );
  }

  return ptrRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::ReadStringMsb()
{
  kmbMsbPtr ptrRet;

  string sValue;
  if ( ReadVariableLengthString(&sValue) )
  {
    ptrRet = kmbMsbManager::Singleton()->CreateStringMsb( sValue );
  }

  return ptrRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::ReadTaggedMsb()
{
  kmbMsbPtr ptrRet;

  // Read child count
  unsigned nChildCount;
  if ( m_pStream->Read(&nChildCount) )
  {
    ptrRet = kmbMsbManager::Singleton()->CreateTaggedMsb();

    // Read each child MSB
    bool     bOk = true;
    unsigned i;
    for (i = 0; bOk && (i < nChildCount); i++)
    {
      bOk = false;

      // Read child tag
      string sTag;
      if ( ReadVariableLengthString(&sTag) )
      {
        // Read child MSB
        kmbMsbPtr ptrMsb;
        ptrMsb = ReadMsb();
        if (ptrMsb != NULL)
        {
          ptrRet->AddChild( sTag, ptrMsb );
          bOk = true;
        }
      }
    }

    if (!bOk)
    {
      ptrRet = NULL;
    }
  }

  return ptrRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
kmbMsbPtr kmbBinaryMsbReader::ReadEnumeratedMsb()
{
  kmbMsbPtr ptrRet;

  // Read child count
  unsigned nChildCount;
  if ( m_pStream->Read(&nChildCount) )
  {
    ptrRet = kmbMsbManager::Singleton()->CreateEnumeratedMsb();

    // Read each child MSB
    bool     bOk = true;
    unsigned i;
    for (i = 0; bOk && (i < nChildCount); i++)
    {
      bOk = false;

      // Read child MSB
      kmbMsbPtr ptrMsb;
      ptrMsb = ReadMsb();
      if (ptrMsb != NULL)
      {
        ptrRet->AddChild( ptrMsb );
        bOk = true;
      }
    }

    if (!bOk)
    {
      ptrRet = NULL;
    }
  }

  return ptrRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbReader::ReadVariableLengthString(string* psString)
{
  bool bRet = false;

  if (psString != NULL)
  {
    // Read the string length
    unsigned nLength;
    if ( m_pStream->Read(&nLength) )
    {
      // Reallocate the buffer if it is not big enough, taking into account the \0 character
      if (m_uBufferLength < (nLength+1))
      {
        delete [] m_pszBuffer;
        m_uBufferLength = nLength + 1;
        m_pszBuffer = new char [m_uBufferLength];
      }

      // Read the string itself
      if ( m_pStream->Read(m_pszBuffer, nLength) )
      {
        m_pszBuffer[nLength] = '\0';
        psString->assign( m_pszBuffer );
        bRet = true;
      }
    }
  }

  return bRet;
}
