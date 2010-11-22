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
  m_eCpcModel       = CPC::CMachine::MODEL_INVALID;
  m_bDrawScanLines  = false;
  m_fEmulationSpeed = 0.f;
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
  m_eCpcModel       = CPC::CMachine::MODEL_464;
  m_bDrawScanLines  = false;
  m_fEmulationSpeed = 1.f;

  m_anKeyMappings[CPC::CPCKEY_ESC] = VK_ESCAPE;
  m_anKeyMappings[CPC::CPCKEY_1] = '1';
  m_anKeyMappings[CPC::CPCKEY_2] = '2';
  m_anKeyMappings[CPC::CPCKEY_3] = '3';
  m_anKeyMappings[CPC::CPCKEY_4] = '4';
  m_anKeyMappings[CPC::CPCKEY_5] = '5';
  m_anKeyMappings[CPC::CPCKEY_6] = '6';
  m_anKeyMappings[CPC::CPCKEY_7] = '7';
  m_anKeyMappings[CPC::CPCKEY_8] = '8';
  m_anKeyMappings[CPC::CPCKEY_9] = '9';
  m_anKeyMappings[CPC::CPCKEY_0] = '0';
  m_anKeyMappings[CPC::CPCKEY_MINUS] = VK_OEM_MINUS;
  m_anKeyMappings[CPC::CPCKEY_ARROW] = VK_OEM_PLUS;
  m_anKeyMappings[CPC::CPCKEY_CLR] = '<';
  m_anKeyMappings[CPC::CPCKEY_DEL] = VK_BACK;
  m_anKeyMappings[CPC::CPCKEY_TAB] = VK_TAB;
  m_anKeyMappings[CPC::CPCKEY_Q] = 'Q';
  m_anKeyMappings[CPC::CPCKEY_W] = 'W';
  m_anKeyMappings[CPC::CPCKEY_E] = 'E';
  m_anKeyMappings[CPC::CPCKEY_R] = 'R';
  m_anKeyMappings[CPC::CPCKEY_T] = 'T';
  m_anKeyMappings[CPC::CPCKEY_Y] = 'Y';
  m_anKeyMappings[CPC::CPCKEY_U] = 'U';
  m_anKeyMappings[CPC::CPCKEY_I] = 'I';
  m_anKeyMappings[CPC::CPCKEY_O] = 'O';
  m_anKeyMappings[CPC::CPCKEY_P] = 'P';
  m_anKeyMappings[CPC::CPCKEY_AT] = VK_OEM_4;
  m_anKeyMappings[CPC::CPCKEY_BRACKET_OPEN] = '+';
  m_anKeyMappings[CPC::CPCKEY_RETURN] = VK_RETURN;
  m_anKeyMappings[CPC::CPCKEY_CAPS_LOCK] = VK_CAPITAL/*CAPS LOCK key*/;
  m_anKeyMappings[CPC::CPCKEY_A] = 'A';
  m_anKeyMappings[CPC::CPCKEY_S] = 'S';
  m_anKeyMappings[CPC::CPCKEY_D] = 'D';
  m_anKeyMappings[CPC::CPCKEY_F] = 'F';
  m_anKeyMappings[CPC::CPCKEY_G] = 'G';
  m_anKeyMappings[CPC::CPCKEY_H] = 'H';
  m_anKeyMappings[CPC::CPCKEY_J] = 'J';
  m_anKeyMappings[CPC::CPCKEY_K] = 'K';
  m_anKeyMappings[CPC::CPCKEY_L] = 'L';
  m_anKeyMappings[CPC::CPCKEY_COLON] = VK_OEM_1;
  m_anKeyMappings[CPC::CPCKEY_SEMICOLON] = VK_OEM_7;
  m_anKeyMappings[CPC::CPCKEY_BRACKET_CLOSE] = VK_OEM_6;
  m_anKeyMappings[CPC::CPCKEY_SHIFT] = VK_SHIFT;
  m_anKeyMappings[CPC::CPCKEY_Z] = 'Z';
  m_anKeyMappings[CPC::CPCKEY_X] = 'X';
  m_anKeyMappings[CPC::CPCKEY_C] = 'C';
  m_anKeyMappings[CPC::CPCKEY_V] = 'V';
  m_anKeyMappings[CPC::CPCKEY_B] = 'B';
  m_anKeyMappings[CPC::CPCKEY_N] = 'N';
  m_anKeyMappings[CPC::CPCKEY_M] = 'M';
  m_anKeyMappings[CPC::CPCKEY_COMMA] = VK_OEM_COMMA;
  m_anKeyMappings[CPC::CPCKEY_PERIOD] = VK_OEM_PERIOD;
  m_anKeyMappings[CPC::CPCKEY_SLASH] = VK_OEM_2;
  m_anKeyMappings[CPC::CPCKEY_INVERTED_SLASH] = VK_OEM_5;
  m_anKeyMappings[CPC::CPCKEY_SPACE] = VK_SPACE;
  m_anKeyMappings[CPC::CPCKEY_CTRL] = VK_LCONTROL/* Left CTRL key*/;
  m_anKeyMappings[CPC::CPCKEY_CURSOR_UP] = VK_UP;
  m_anKeyMappings[CPC::CPCKEY_CURSOR_DOWN] = VK_DOWN;
  m_anKeyMappings[CPC::CPCKEY_CURSOR_LEFT] = VK_LEFT;
  m_anKeyMappings[CPC::CPCKEY_CURSOR_RIGHT] = VK_RIGHT;
  m_anKeyMappings[CPC::CPCKEY_COPY] = VK_RMENU/*Right ALT key*/;
  m_anKeyMappings[CPC::CPCKEY_NUM_0] = VK_NUMPAD0;
  m_anKeyMappings[CPC::CPCKEY_NUM_1] = VK_NUMPAD1;
  m_anKeyMappings[CPC::CPCKEY_NUM_2] = VK_NUMPAD2;  // With Num Lock ON (special case)
  m_anKeyMappings[CPC::CPCKEY_NUM_3] = VK_NUMPAD3;
  m_anKeyMappings[CPC::CPCKEY_NUM_4] = VK_NUMPAD4;  // With Num Lock ON (special case)
  m_anKeyMappings[CPC::CPCKEY_NUM_5] = VK_NUMPAD5;
  m_anKeyMappings[CPC::CPCKEY_NUM_6] = VK_NUMPAD6;  // With Num Lock ON (special case)
  m_anKeyMappings[CPC::CPCKEY_NUM_7] = VK_NUMPAD7;
  m_anKeyMappings[CPC::CPCKEY_NUM_8] = VK_NUMPAD8;  // With Num Lock ON (special case)
  m_anKeyMappings[CPC::CPCKEY_NUM_9] = VK_NUMPAD9;
  m_anKeyMappings[CPC::CPCKEY_NUM_PERIOD] = VK_DECIMAL;
  m_anKeyMappings[CPC::CPCKEY_NUM_ENTER] = VK_SEPARATOR;
  m_anKeyMappings[CPC::CPCKEY_JOY_UP] = VK_NUMPAD8;  // With Num Lock OFF (special case)
  m_anKeyMappings[CPC::CPCKEY_JOY_DOWN] = VK_NUMPAD2;  // With Num Lock OFF (special case)
  m_anKeyMappings[CPC::CPCKEY_JOY_LEFT] = VK_NUMPAD4;  // With Num Lock OFF (special case)
  m_anKeyMappings[CPC::CPCKEY_JOY_RIGHT] = VK_NUMPAD6;  // With Num Lock OFF (special case)
  m_anKeyMappings[CPC::CPCKEY_JOY_FIRE1] = VK_DELETE;
  m_anKeyMappings[CPC::CPCKEY_JOY_FIRE2] = VK_END;
}
