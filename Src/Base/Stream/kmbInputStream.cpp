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
