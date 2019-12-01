//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbBinaryMsbWriter.h"
#include "Stream/kmbOutputStream.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbBinaryMsbWriter::Init(const kmbMsbPtr& ptrMsb, kmbOutputStream* pStream)
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    bRet = (pStream != NULL) && pStream->IsOk();
    KMASSERTM( bRet, ("The given output stream is not valid.") );
  }

  // Initialize class members
  if (bRet)
  {
    m_ptrMsb  = ptrMsb;
    m_pStream = pStream;
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
/*virtual*/ void kmbBinaryMsbWriter::End()
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
void kmbBinaryMsbWriter::ResetVars()
{
  m_ptrMsb  = NULL;
  m_pStream = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbBinaryMsbWriter::FreeVars()
{
  m_ptrMsb = NULL;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::Write()
{
  return WriteMsb( m_ptrMsb.Get() );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteMsb(const kmbMsb* pMsb)
{
  bool bRet = true;

  // Write the MSB type
  if (bRet)
  {
    bRet = m_pStream->Write( pMsb->GetType() );
  }

  // Write the MSB itself
  if (bRet)
  {
    switch (pMsb->GetType())
    {
    case kmbMsb::TYPE_INTEGER:
      bRet = WriteIntegerMsb( static_cast<const CIntegerMsb*>(pMsb) );
      break;

    case kmbMsb::TYPE_NULL:
      bRet = WriteNullMsb( static_cast<const CNullMsb*>(pMsb) );
      break;

    case kmbMsb::TYPE_REAL:
      bRet = WriteRealMsb( static_cast<const CRealMsb*>(pMsb) );
      break;

    case kmbMsb::TYPE_STRING:
      bRet = WriteStringMsb( static_cast<const CStringMsb*>(pMsb) );
      break;

    case kmbMsb::TYPE_TAGGED:
      bRet = WriteTaggedMsb( static_cast<const CTaggedMsb*>(pMsb) );
      break;

    case kmbMsb::TYPE_ENUMERATED:
      bRet = WriteEnumeratedMsb( static_cast<const CEnumeratedMsb*>(pMsb) );
      break;

    default:
      KMASSERTM( false, ("MSB type not implemented!") );
    }
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteIntegerMsb(const CIntegerMsb* pIntegerMsb)
{
  return m_pStream->Write( pIntegerMsb->GetInt() );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteNullMsb(const CNullMsb* pNullMsb)
{
  return true;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteRealMsb(const CRealMsb* pRealMsb)
{
  return m_pStream->Write( pRealMsb->GetFloat() );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteStringMsb(const CStringMsb* pStringMsb)
{
  return WriteVariableLengthString( pStringMsb->GetString("") );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteTaggedMsb(const CTaggedMsb* pTaggedMsb)
{
  bool bRet = true;

  // Write child count
  if (bRet)
  {
    bRet = m_pStream->Write( pTaggedMsb->GetNumChildren() );
  }

  // Write each child MSB
  if (bRet)
  {
    CTaggedMsb::TMsbMap::iterator iter;
    CTaggedMsb::TMsbMap::iterator iterEnd;
    pTaggedMsb->GetChildrenIterators( &iter, &iterEnd );
    for (/*EMPTY*/; bRet && (iter != iterEnd); ++iter)
    {
      const string&    sTag        = iter->first;
      const kmbMsbPtr& ptrChildMsb = iter->second;
      // Write child tag
      bRet = bRet && WriteVariableLengthString( sTag );
      // Write child MSB
      bRet = bRet && WriteMsb( ptrChildMsb.Get() );
    }
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteEnumeratedMsb(const CEnumeratedMsb* pEnumeratedMsb)
{
  bool bRet = true;

  // Write child count
  if (bRet)
  {
    bRet = m_pStream->Write( pEnumeratedMsb->GetNumChildren() );
  }

  // Write each child MSB
  if (bRet)
  {
    unsigned i = 0;
    while (bRet && (i < pEnumeratedMsb->GetNumChildren()))
    {
      bRet = WriteMsb( pEnumeratedMsb->GetChild(i).Get() );
      i++;
    }
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbBinaryMsbWriter::WriteVariableLengthString(const string& sString)
{
  bool bRet = true;

  // Write the string length
  bRet = m_pStream->Write( (unsigned) sString.length() );

  // Write the string itself (if it is not empty)
  if (bRet && !sString.empty())
  {
    bRet = m_pStream->Write( sString.c_str(), sString.length() );
  }

  return bRet;
}
