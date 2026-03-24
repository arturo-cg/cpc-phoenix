//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbTextMsbWriter.h"
#include <sstream>
#include "Stream/kmbOutputStream.h"


/*static*/ const unsigned kmbTextMsbWriter::INDENTATION_INCREMENT = 2;


//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbTextMsbWriter::Init(const kmbMsbPtr& ptrMsb, kmbOutputStream* pStream)
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
/*virtual*/ void kmbTextMsbWriter::End()
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
void kmbTextMsbWriter::ResetVars()
{
  m_ptrMsb  = NULL;
  m_pStream = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbTextMsbWriter::FreeVars()
{
  m_ptrMsb = NULL;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::Write()
{
  return WriteMsb( m_ptrMsb.Get(), 0 );
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteMsb(const kmbMsb* pMsb, unsigned nIndentation)
{
  bool bRet = true;

  // Write the MSB itself
  switch (pMsb->GetType())
  {
  case kmbMsb::TYPE_INTEGER:
    bRet = WriteIntegerMsb( static_cast<const CIntegerMsb*>(pMsb), nIndentation );
    break;

  case kmbMsb::TYPE_NULL:
    bRet = WriteNullMsb( static_cast<const CNullMsb*>(pMsb), nIndentation );
    break;

  case kmbMsb::TYPE_REAL:
    bRet = WriteRealMsb( static_cast<const CRealMsb*>(pMsb), nIndentation );
    break;

  case kmbMsb::TYPE_STRING:
    bRet = WriteStringMsb( static_cast<const CStringMsb*>(pMsb), nIndentation );
    break;

  case kmbMsb::TYPE_TAGGED:
    bRet = WriteTaggedMsb( static_cast<const CTaggedMsb*>(pMsb), nIndentation );
    break;

  case kmbMsb::TYPE_ENUMERATED:
    bRet = WriteEnumeratedMsb( static_cast<const CEnumeratedMsb*>(pMsb), nIndentation );
    break;

  default:
    KMASSERTM( false, ("MSB type not implemented!") );
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteIntegerMsb(const CIntegerMsb* pIntegerMsb, unsigned nIndentation)
{
  bool bRet;
  bRet = WriteSpaces( nIndentation );
  if (bRet)
  {
    char szBuffer[15];
    _snprintf( szBuffer, sizeof(szBuffer), "%d\n", pIntegerMsb->GetInt() );
    bRet = m_pStream->WriteString( szBuffer );
  }
  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteNullMsb(const CNullMsb* pNullMsb, unsigned nIndentation)
{
  return true;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteRealMsb(const CRealMsb* pRealMsb, unsigned nIndentation)
{
  bool bRet;
  bRet = WriteSpaces( nIndentation );
  if (bRet)
  {
    std::string sBuffer;
    ConvertDoubleToString( pRealMsb->GetDouble(), &sBuffer );
    sBuffer += '\n';

    bRet = m_pStream->WriteString( sBuffer );
  }
  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteStringMsb(const CStringMsb* pStringMsb, unsigned nIndentation)
{
  bool bRet;
  bRet = WriteSpaces( nIndentation );
  bRet = bRet && m_pStream->WriteString( "\"" + pStringMsb->GetString("") + "\"" + "\n" );
  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteTaggedMsb(const CTaggedMsb* pTaggedMsb, unsigned nIndentation)
{
  bool bRet = true;

  // Opening bracket
  if (bRet)
  {
    bRet = WriteSpaces( nIndentation );
    bRet = bRet && m_pStream->WriteString( "[\n" );
  }

  // Children MSBs
  if (bRet)
  {
    CTaggedMsb::TMsbMap::iterator iter;
    CTaggedMsb::TMsbMap::iterator iterEnd;
    pTaggedMsb->GetChildrenIterators( &iter, &iterEnd );
    for (/*EMPTY*/; bRet && (iter != iterEnd); ++iter)
    {
      const std::string&    sTag        = iter->first;
      const kmbMsbPtr& ptrChildMsb = iter->second;
      // Write child indentation
      bRet = WriteSpaces( nIndentation + INDENTATION_INCREMENT );
      // Write tag
      bRet = bRet && m_pStream->WriteString( "." + sTag );
      // Write child MSB
      if (bRet)
      {
        if (ptrChildMsb->GetNumChildren() == 0)
        {
          if (ptrChildMsb->GetType() == kmbMsb::TYPE_TAGGED)
          {
            bRet = m_pStream->WriteString( " [ ]\n" );
          }
          else
          if (ptrChildMsb->GetType() == kmbMsb::TYPE_ENUMERATED)
          {
            bRet = m_pStream->WriteString( " ( )\n" );
          }
          else
          {
            bRet = WriteMsb( ptrChildMsb.Get(), 1 /*space after tag*/ );
          }
        }
        else
        {
          if ( (ptrChildMsb->GetType() == kmbMsb::TYPE_ENUMERATED) && IsVecQuat(static_cast<const CEnumeratedMsb*>(ptrChildMsb.Get())) )
          {
            bRet = WriteEnumeratedMsbAsVecQuat( static_cast<const CEnumeratedMsb*>(ptrChildMsb.Get()), 1 /*space after tag*/ );
          }
          else
          {
            bRet = m_pStream->WriteString( "\n" );
            bRet = bRet && WriteMsb( ptrChildMsb.Get(), nIndentation + INDENTATION_INCREMENT );
          }
        }
      }
    }
  }

  // Closing bracket
  if (bRet)
  {
    bRet = WriteSpaces( nIndentation );
    bRet = bRet && m_pStream->WriteString( "]\n" );
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteEnumeratedMsb(const CEnumeratedMsb* pEnumeratedMsb, unsigned nIndentation)
{
  bool bRet = true;

  // If this CEnumeratedMsb has 2, 3 or 4 children and all of them are CIntegerMsb or CRealMsb,
  // then assume its a 2D/3D vector or a quaternion and write them in a single line.
  // Otherwise, write one child per line.
  if ( IsVecQuat(pEnumeratedMsb) )
  {
    //
    // 2D/3D vector or quaternion - All children in a single line
    // 
    bRet = WriteEnumeratedMsbAsVecQuat( pEnumeratedMsb, nIndentation );
  }
  else
  {
    //
    // Normal enumeration - One children per line
    // 

    // Opening parenthesis
    if (bRet)
    {
      bRet = WriteSpaces( nIndentation );
      bRet = bRet && m_pStream->WriteString( "(\n" );
    }

    // Children MSBs
    if (bRet)
    {
      unsigned i = 0;
      while (bRet && (i < pEnumeratedMsb->GetNumChildren()))
      {
        bRet = WriteMsb( pEnumeratedMsb->GetChild(i).Get(), nIndentation + INDENTATION_INCREMENT );
        i++;
      }
    }

    // Closing parenthesis
    if (bRet)
    {
      bRet = WriteSpaces( nIndentation );
      bRet = bRet && m_pStream->WriteString( ")\n" );
    }
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteEnumeratedMsbAsVecQuat(const CEnumeratedMsb* pEnumeratedMsb, unsigned nIndentation)
{
  bool bRet;

  bRet = WriteSpaces( nIndentation );
  bRet = bRet && m_pStream->WriteString( "(" );

  unsigned i;
  std::string sBuffer;
  for (i = 0; bRet && (i < pEnumeratedMsb->GetNumChildren()); i++)
  {
    ConvertDoubleToString( pEnumeratedMsb->GetChild(i)->GetDouble(), &sBuffer );
    sBuffer.insert( sBuffer.begin(), ' ' );
    bRet = m_pStream->WriteString( sBuffer );
  }

  bRet = bRet && m_pStream->WriteString( " )\n" );

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::IsVecQuat(const CEnumeratedMsb* pEnumeratedMsb) const
{
  bool bRet = false;

  if ( (pEnumeratedMsb->GetNumChildren() >= 2) && (pEnumeratedMsb->GetNumChildren() <= 4) )
  {
    unsigned i;
    bRet = true;
    for (i = 0; bRet && (i < pEnumeratedMsb->GetNumChildren()); i++)
    {
      if ( (pEnumeratedMsb->GetChild(i)->GetType() != kmbMsb::TYPE_REAL) && (pEnumeratedMsb->GetChild(i)->GetType() != kmbMsb::TYPE_INTEGER) )
      {
        bRet = false;
      }
    }
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbWriter::WriteSpaces(unsigned nNumSpaces)
{
  bool     bRet = true;
  unsigned i    = 0;
  while ( bRet && (i < nNumSpaces) )
  {
    bRet = m_pStream->WriteString( " ", 1 );
    i++;
  }
  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbTextMsbWriter::ConvertDoubleToString(double dValue, std::string* pResult)
{
  if (pResult != NULL)
  {
    pResult->clear();

    std::ostringstream stream;
    stream << dValue;
    *pResult = stream.str();
  }
}
