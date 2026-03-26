//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbOutputStream.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbOutputStream::Init()
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Initialize class members
  if (bRet)
  {
    m_uWrittenBytesCount = 0;
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
/*virtual*/ void kmbOutputStream::End()
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
void kmbOutputStream::ResetVars()
{
  m_uWrittenBytesCount = 0;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbOutputStream::FreeVars()
{
  //...
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbOutputStream::WriteString(const char* pszString, unsigned nCharCount)
{
  if (pszString != NULL)
  {
    return Write( pszString, nCharCount );    // 1 byte per character (ASCII encoding)
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbOutputStream::WriteString(const std::string& sString)
{
  return WriteString( sString.c_str(), static_cast<unsigned>(sString.length()) );
}
