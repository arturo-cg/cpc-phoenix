//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbInputStream.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbInputStream::Init()
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
    m_uReadBytesCount = 0;
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
/*virtual*/ void kmbInputStream::End()
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
void kmbInputStream::ResetVars()
{
  m_uReadBytesCount = 0;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbInputStream::FreeVars()
{
  //...
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbInputStream::ReadChar(char* pChar)
{
  if (pChar != NULL)
  {
    return Read( pChar, 1 );   // 1 byte per character (ASCII encoding)
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
bool kmbInputStream::ReadLine(string* psLine)
{
  bool bRet = false;
  if (psLine != NULL)
  {
    bRet = true;
    psLine->clear();

    // Read characters until new line or EOF
    char currChar;
    bRet = ReadChar( &currChar );
    while ( bRet && !IsAtEnd() && (currChar != '\n'/*Unix style*/) && (currChar != '\r'/*Windows style*/) )
    {
      *psLine += currChar;
      bRet = ReadChar( &currChar );
    }

    if ( bRet && !IsAtEnd() && (currChar == '\r'/*Windows style*/) )
    {
      // Windows uses the sequence \r\n to mark new lines so we still need to read the \n
      ReadChar( &currChar );
      ASSERT( currChar == '\n' );
    }
  }

  return bRet;
}
