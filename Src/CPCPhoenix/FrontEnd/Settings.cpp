//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Settings.h"



/*static*/ const char* Settings::SETTINGS_FILE_NAME = "CPCPhoenix.cfg";



//----------------------------------------------------------------------------
/**
** Init
*/
bool Settings::Init()
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Set default values to all settings
  if (bRet)
  {
    RestoreDefaultValues();
  }

  // Initialize class members
  if (bRet)
  {
    //...
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
/*virtual*/ void Settings::End()
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
void Settings::ResetVars()
{
  m_eCpcModel      = CPC::CMachine::MODEL_INVALID;
  m_bDrawScanLines = false;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void Settings::FreeVars()
{
  //...
}

//----------------------------------------------------------------------------
/**
** 
*/
void Settings::LoadFromFile()
{
//************************************** TODO - TODO - TODO ********************************************
//************************************** TODO - TODO - TODO ********************************************
  // TODO - For now, just set default values
  RestoreDefaultValues();
//************************************** TODO - TODO - TODO ********************************************
//************************************** TODO - TODO - TODO ********************************************
}

//----------------------------------------------------------------------------
/**
** 
*/
void Settings::SaveToFile()
{
//************************************** TODO - TODO - TODO ********************************************
//************************************** TODO - TODO - TODO ********************************************
//************************************** TODO - TODO - TODO ********************************************
//************************************** TODO - TODO - TODO ********************************************
}

//----------------------------------------------------------------------------
/**
** 
*/
void Settings::RestoreDefaultValues()
{
  m_eCpcModel      = CPC::CMachine::MODEL_464;
  m_bDrawScanLines = false;
}
