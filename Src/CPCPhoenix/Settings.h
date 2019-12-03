//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#include "cpcMachine.h"
#include "cpcGateArray.h"
#include "cpcKeyboardTypes.h"


/**
**
*/
class Settings
{
public:

    static constexpr char* SettingsFileName = "Settings.msb";

    enum EModifierKeyState
    {
        MODIFIERKEY_ON = 0x01,
        MODIFIERKEY_OFF = 0x02,
        MODIFIERKEY_ANY = 0x03 /*MODIFIERKEY_ON | MODIFIERKEY_OFF*/,
    };

    struct SMappedKey
    {
        int               nWindowsKey;
        EModifierKeyState eNumLock;
    };


    Settings() { m_bOk = false; }
    virtual                  ~Settings() { End(); }

    bool                      Init();
    virtual void              End();
    bool                      IsOk() const { return m_bOk; }

    /** Loads settings from the settings file. If it doesn't exist, it assigns default values. */
    void                      LoadFromFile();
    /** Saves settings to the settings file. */
    void                      SaveToFile();

    /** Restores setting default values. */
    void                      RestoreDefaultValues();

    void                      SetMachineSpecificationName(string name) { m_machineSpecificationName = name; }
    string                    GetMachineSpecificationName() const { return m_machineSpecificationName; }

    void                                     SetMonitorType(CPC::CGateArray::ERgbConversionTableType eType) { m_eMonitorType = eType; }
    CPC::CGateArray::ERgbConversionTableType GetMonitorType() const { return m_eMonitorType; }

    void                      SetScale(float scale) { m_scale = scale; }
    float                     GetScale() const { return m_scale; }

    void                      SetDrawScanLines(bool bScanLines) { m_bDrawScanLines = bScanLines; }
    bool                      GetDrawScanLines() const { return m_bDrawScanLines; }

    void                      SetEmulationSpeed(float fSpeed) { m_fEmulationSpeed = fSpeed; }
    float                     GetEmulationSpeed() const { return m_fEmulationSpeed; }

    const SMappedKey&         GetCpcKeyMapping(CPC::ECpcKey eCpcKey) const { return m_aKeyMappings[eCpcKey]; }

    void                      SetDiskImage(unsigned nDrive, const std::string& sDiskImageFileName) { m_asDiskImages[nDrive] = sDiskImageFileName; }
    const std::string&        GetDiskImage(unsigned nDrive) const { return m_asDiskImages[nDrive]; }


private:

    static const char*        SETTINGS_FILE_NAME;
    static const SMappedKey   DEFAULT_KEY_MAPPINGS[CPC::CPCKEY_LAST];


    void                      ResetVars();
    void                      FreeVars();


    bool                      m_bOk;

    string                    m_machineSpecificationName;
    CPC::CGateArray::ERgbConversionTableType m_eMonitorType;
    float                     m_scale;
    bool                      m_bDrawScanLines;
    float                     m_fEmulationSpeed;
    SMappedKey                m_aKeyMappings[CPC::CPCKEY_LAST];
    std::string               m_asDiskImages[CPC::CMachine::DRIVE_COUNT];

};

#endif // _SETTINGS_H_
