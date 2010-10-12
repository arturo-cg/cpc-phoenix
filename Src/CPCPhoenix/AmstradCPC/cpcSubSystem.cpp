//-------------------------------------------------------------------------------------------
// File:        SubSystem.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcSubSystem.h"


namespace CPC {



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CSubSystem::CSubSystem(CMachine *pMachine)
  {
    m_pMachine = pMachine;
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CSubSystem::ResetVars()
  {
    m_pMachine = NULL;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CSubSystem::FreeVars()
  {

  }

} //namespace CPC
