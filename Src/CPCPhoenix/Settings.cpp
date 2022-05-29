//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Settings.h"
#include "Application.h"
#include "Stream/kmbFileInputStream.h"
#include "Stream/kmbFileOutputStream.h"
#include "Msb/kmbMsbManager.h"
#include "Msb/kmbTextMsbWriter.h"
#include "cpcVideoOutput.h"


/*static*/ const char* Settings::SETTINGS_FILE_NAME = "CPCPhoenix.cfg";

/*static*/ const Settings::SMappedKey Settings::DEFAULT_KEY_MAPPINGS[CPC::CPCKEY_LAST] =
{
  { VK_ESCAPE, MODIFIERKEY_ANY },  /*CPCKEY_ESC*/
  { '1', MODIFIERKEY_ANY },  /*CPCKEY_1*/
  { '2', MODIFIERKEY_ANY },  /*CPCKEY_2*/
  { '3', MODIFIERKEY_ANY },  /*CPCKEY_3*/
  { '4', MODIFIERKEY_ANY },  /*CPCKEY_4*/
  { '5', MODIFIERKEY_ANY },  /*CPCKEY_5*/
  { '6', MODIFIERKEY_ANY },  /*CPCKEY_6*/
  { '7', MODIFIERKEY_ANY },  /*CPCKEY_7*/
  { '8', MODIFIERKEY_ANY },  /*CPCKEY_8*/
  { '9', MODIFIERKEY_ANY },  /*CPCKEY_9*/
  { '0', MODIFIERKEY_ANY },  /*CPCKEY_0*/
  { VK_OEM_MINUS, MODIFIERKEY_ANY },  /*CPCKEY_MINUS*/
  { VK_OEM_PLUS, MODIFIERKEY_ANY },  /*CPCKEY_ARROW*/
  { '<', MODIFIERKEY_ANY },  /*CPCKEY_CLR*/
  { VK_BACK, MODIFIERKEY_ANY },  /*CPCKEY_DEL*/
  { VK_TAB, MODIFIERKEY_ANY },  /*CPCKEY_TAB*/
  { 'Q', MODIFIERKEY_ANY },  /*CPCKEY_Q*/
  { 'W', MODIFIERKEY_ANY },  /*CPCKEY_W*/
  { 'E', MODIFIERKEY_ANY },  /*CPCKEY_E*/
  { 'R', MODIFIERKEY_ANY },  /*CPCKEY_R*/
  { 'T', MODIFIERKEY_ANY },  /*CPCKEY_T*/
  { 'Y', MODIFIERKEY_ANY },  /*CPCKEY_Y*/
  { 'U', MODIFIERKEY_ANY },  /*CPCKEY_U*/
  { 'I', MODIFIERKEY_ANY },  /*CPCKEY_I*/
  { 'O', MODIFIERKEY_ANY },  /*CPCKEY_O*/
  { 'P', MODIFIERKEY_ANY },  /*CPCKEY_P*/
  { VK_OEM_4, MODIFIERKEY_ANY },  /*CPCKEY_AT*/
  { '+', MODIFIERKEY_ANY },  /*CPCKEY_BRACKET_OPEN*/
  { VK_RETURN, MODIFIERKEY_ANY },  /*CPCKEY_RETURN*/
  { VK_CAPITAL/*CAPS LOCK key*/, MODIFIERKEY_ANY },  /*CPCKEY_CAPS_LOCK*/
  { 'A', MODIFIERKEY_ANY },  /*CPCKEY_A*/
  { 'S', MODIFIERKEY_ANY },  /*CPCKEY_S*/
  { 'D', MODIFIERKEY_ANY },  /*CPCKEY_D*/
  { 'F', MODIFIERKEY_ANY },  /*CPCKEY_F*/
  { 'G', MODIFIERKEY_ANY },  /*CPCKEY_G*/
  { 'H', MODIFIERKEY_ANY },  /*CPCKEY_H*/
  { 'J', MODIFIERKEY_ANY },  /*CPCKEY_J*/
  { 'K', MODIFIERKEY_ANY },  /*CPCKEY_K*/
  { 'L', MODIFIERKEY_ANY },  /*CPCKEY_L*/
  { VK_OEM_1, MODIFIERKEY_ANY },  /*CPCKEY_COLON*/
  { VK_OEM_7, MODIFIERKEY_ANY },  /*CPCKEY_SEMICOLON*/
  { VK_OEM_6, MODIFIERKEY_ANY },  /*CPCKEY_BRACKET_CLOSE*/
  { VK_SHIFT, MODIFIERKEY_ANY },  /*CPCKEY_SHIFT*/
  { 'Z', MODIFIERKEY_ANY },  /*CPCKEY_Z*/
  { 'X', MODIFIERKEY_ANY },  /*CPCKEY_X*/
  { 'C', MODIFIERKEY_ANY },  /*CPCKEY_C*/
  { 'V', MODIFIERKEY_ANY },  /*CPCKEY_V*/
  { 'B', MODIFIERKEY_ANY },  /*CPCKEY_B*/
  { 'N', MODIFIERKEY_ANY },  /*CPCKEY_N*/
  { 'M', MODIFIERKEY_ANY },  /*CPCKEY_M*/
  { VK_OEM_COMMA, MODIFIERKEY_ANY },  /*CPCKEY_COMMA*/
  { VK_OEM_PERIOD, MODIFIERKEY_ANY },  /*CPCKEY_PERIOD*/
  { VK_OEM_2, MODIFIERKEY_ANY },  /*CPCKEY_SLASH*/
  { VK_OEM_5, MODIFIERKEY_ANY },  /*CPCKEY_INVERTED_SLASH*/
  { VK_SPACE, MODIFIERKEY_ANY },  /*CPCKEY_SPACE*/
  { VK_LCONTROL/*Left CTRL key*/, MODIFIERKEY_ANY },  /*CPCKEY_CTRL*/
  { VK_UP, MODIFIERKEY_ON },  /*CPCKEY_CURSOR_UP*/
  { VK_DOWN, MODIFIERKEY_ON },  /*CPCKEY_CURSOR_DOWN*/
  { VK_LEFT, MODIFIERKEY_ON },  /*CPCKEY_CURSOR_LEFT*/
  { VK_RIGHT, MODIFIERKEY_ON },  /*CPCKEY_CURSOR_RIGHT*/
  { VK_RMENU/*Right ALT key*/, MODIFIERKEY_ANY },  /*CPCKEY_COPY*/
  { VK_NUMPAD0, MODIFIERKEY_ON },  /*CPCKEY_NUM_0*/
  { VK_NUMPAD1, MODIFIERKEY_ON },  /*CPCKEY_NUM_1*/
  { VK_NUMPAD2, MODIFIERKEY_ON },  /*CPCKEY_NUM_2*/
  { VK_NUMPAD3, MODIFIERKEY_ON },  /*CPCKEY_NUM_3*/
  { VK_NUMPAD4, MODIFIERKEY_ON },  /*CPCKEY_NUM_4*/
  { VK_NUMPAD5, MODIFIERKEY_ON },  /*CPCKEY_NUM_5*/
  { VK_NUMPAD6, MODIFIERKEY_ON },  /*CPCKEY_NUM_6*/
  { VK_NUMPAD7, MODIFIERKEY_ON },  /*CPCKEY_NUM_7*/
  { VK_NUMPAD8, MODIFIERKEY_ON },  /*CPCKEY_NUM_8*/
  { VK_NUMPAD9, MODIFIERKEY_ON },  /*CPCKEY_NUM_9*/
  { VK_DECIMAL, MODIFIERKEY_ON },  /*CPCKEY_NUM_PERIOD*/
  { VK_SEPARATOR, MODIFIERKEY_ANY },  /*CPCKEY_NUM_ENTER*/
  { VK_UP, MODIFIERKEY_OFF },  /*CPCKEY_JOY_UP*/
  { VK_DOWN, MODIFIERKEY_OFF },  /*CPCKEY_JOY_DOWN*/
  { VK_LEFT, MODIFIERKEY_OFF },  /*CPCKEY_JOY_LEFT*/
  { VK_RIGHT, MODIFIERKEY_OFF },  /*CPCKEY_JOY_RIGHT*/
  { VK_DELETE, MODIFIERKEY_OFF },  /*CPCKEY_JOY_FIRE1*/
  { VK_END, MODIFIERKEY_OFF },  /*CPCKEY_JOY_FIRE2*/
};



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
    if (IsOk())
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
    m_mainWindowX = 0;
    m_mainWindowY = 0;
    m_mainWindowWidth = 0;
    m_mainWindowHeight = 0;
    m_machineSpecificationName.clear();
    m_eMonitorType = CPC::CGateArray::RGBCONVERSIONTABLE_INVALID;
    m_displayScale = 0.0f;
    m_bDrawScanLines = false;
    m_fEmulationSpeed = 0.f;
    for (unsigned i = 0; i < CPC::CMachine::DRIVE_COUNT; i++)
    {
        m_diskImages[i].clear();
        m_diskImageArchives[i].clear();
    }
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
    kmbMsbPtr settingsMsb;
    // Read the settings file.
    kmbFileInputStream stream;
    if (stream.Init(SettingsFileName))
    {
        settingsMsb = kmbMsbManager::Singleton()->CreateMsbFromStream(&stream);
    }
    else
    {
        // The settings file doesn't exist -> Use an empty MSB and let the code below assign the default values.
        settingsMsb = kmbMsbManager::Singleton()->GetUniqueNullMsb();
    }
    // Set default values.
    // Not all the settings are saved to the file so this ensures that all our variables get sensible values.
    RestoreDefaultValues();
    // Get values from the MSB.
    m_mainWindowX = settingsMsb["MainWindowX"]->GetInt(m_mainWindowX);
    m_mainWindowY = settingsMsb["MainWindowY"]->GetInt(m_mainWindowY);
    m_mainWindowWidth = settingsMsb["MainWindowWidth"]->GetInt(m_mainWindowWidth);
    m_mainWindowHeight = settingsMsb["MainWindowHeight"]->GetInt(m_mainWindowHeight);
    m_machineSpecificationName = settingsMsb["MachineSpecificationsName"]->GetString(m_machineSpecificationName);
    m_eMonitorType = (CPC::CGateArray::ERgbConversionTableType) settingsMsb["MonitorColorOutput"]->GetInt(m_eMonitorType);
    m_displayScale = settingsMsb["DisplayScale"]->GetFloat(m_displayScale);
    m_diskImages[0] = settingsMsb["DriveA"]->GetString(m_diskImages[0]);
    m_diskImages[1] = settingsMsb["DriveB"]->GetString(m_diskImages[1]);
    m_diskImageArchives[0] = settingsMsb["DriveA_Archive"]->GetString(m_diskImageArchives[0]);
    m_diskImageArchives[1] = settingsMsb["DriveB_Archive"]->GetString(m_diskImageArchives[1]);
}

//----------------------------------------------------------------------------
/**
**
*/
void Settings::SaveToFile()
{
    // Store settings in a new MSB.
    kmbMsbPtr settingsMsb = kmbMsbManager::Singleton()->CreateTaggedMsb();
    settingsMsb->AddChild("MainWindowX", kmbMsbManager::Singleton()->CreateIntegerMsb(m_mainWindowX));
    settingsMsb->AddChild("MainWindowY", kmbMsbManager::Singleton()->CreateIntegerMsb(m_mainWindowY));
    settingsMsb->AddChild("MainWindowWidth", kmbMsbManager::Singleton()->CreateIntegerMsb(m_mainWindowWidth));
    settingsMsb->AddChild("MainWindowHeight", kmbMsbManager::Singleton()->CreateIntegerMsb(m_mainWindowHeight));
    settingsMsb->AddChild("MachineSpecificationsName", kmbMsbManager::Singleton()->CreateStringMsb(m_machineSpecificationName));
    settingsMsb->AddChild("MonitorColorOutput", kmbMsbManager::Singleton()->CreateIntegerMsb(m_eMonitorType));
    settingsMsb->AddChild("DisplayScale", kmbMsbManager::Singleton()->CreateRealMsb(m_displayScale));
    settingsMsb->AddChild("DriveA", kmbMsbManager::Singleton()->CreateStringMsb(m_diskImages[0]));
    settingsMsb->AddChild("DriveB", kmbMsbManager::Singleton()->CreateStringMsb(m_diskImages[1]));
    settingsMsb->AddChild("DriveA_Archive", kmbMsbManager::Singleton()->CreateStringMsb(m_diskImageArchives[0]));
    settingsMsb->AddChild("DriveB_Archive", kmbMsbManager::Singleton()->CreateStringMsb(m_diskImageArchives[1]));
    // Write the MSB to file.
    kmbFileOutputStream stream;
    if (stream.Init(SettingsFileName))
    {
        kmbTextMsbWriter msbWriter;
        msbWriter.Init(settingsMsb, &stream);
        msbWriter.Write();
    }
    else
    {
        KMASSERTM( false, ("Could not open the settings file ('%s') for writing.\n", SettingsFileName));
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void Settings::RestoreDefaultValues()
{
    static constexpr int ExtraWidth = 60;
    static constexpr int ExtraHeight = 140;
    m_mainWindowX = 0;
    m_mainWindowY = 0;
    m_mainWindowWidth = int(CPC::CVideoOutput::VIEWPORT_WIDTH) + ExtraWidth;
    m_mainWindowHeight = int(CPC::CVideoOutput::VIEWPORT_HEIGHT * 2) + ExtraHeight;

    m_machineSpecificationName = Application::StandardCpc6128SpecificationsName;
    m_eMonitorType = CPC::CGateArray::RGBCONVERSIONTABLE_COLOR;
    m_displayScale = -1.f/*Fit to Window*/;
    m_bDrawScanLines = false;
    m_fEmulationSpeed = 1.f;
    for (unsigned i = 0; i < CPC::CMachine::DRIVE_COUNT; i++)
    {
        m_diskImages[i].clear();
        m_diskImageArchives[i].clear();
    }
    ::memcpy(m_aKeyMappings, DEFAULT_KEY_MAPPINGS, sizeof(m_aKeyMappings));
}
