//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbPrecisionTimer.h"




//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbPrecisionTimer::Init()
{
  bool bRet = true;

  End();
  ResetVars();

  // Initialize class members
  if (bRet)
  {
    if( !QueryPerformanceFrequency((LARGE_INTEGER *)&m_i64TimerFrequency) )
    {
      ASSERTM( false, ("The system does not support high-precision timers.") );
      bRet = false;
    }
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
/*virtual*/ void kmbPrecisionTimer::End()
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
void kmbPrecisionTimer::ResetVars()
{
  m_i64TimerFrequency = 0;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbPrecisionTimer::FreeVars()
{

}
