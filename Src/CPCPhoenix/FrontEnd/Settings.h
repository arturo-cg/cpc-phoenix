//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#include "cpcMachine.h"
#include "cpcKeyboardTypes.h"


/**
** 
*/
class Settings
{
public:

                            Settings                  ()  { m_bOk = false; }
  virtual                  ~Settings                  ()  { End(); }

  bool                      Init                      ();
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  /** Loads settings from the settings file. If it doesn't exist, it assigns default values. */
  void                      LoadFromFile              ();
  /** Saves settings to the settings file. */
  void                      SaveToFile                ();

  /** Restores setting default values. */
  void                      RestoreDefaultValues      ();

  void                      SetCpcModel               (CPC::CMachine::EModel eNewModel)  { m_eCpcModel = eNewModel; }
  CPC::CMachine::EModel     GetCpcModel               () const                           { return m_eCpcModel; }

  void                      SetDrawScanLines          (bool bScanLines)  { m_bDrawScanLines = bScanLines; }
  bool                      GetDrawScanLines          () const           { return m_bDrawScanLines; }

  void                      SetEmulationSpeed         (float fSpeed)  { m_fEmulationSpeed = fSpeed; }
  float                     GetEmulationSpeed         () const        { return m_fEmulationSpeed; }

  int                       GetCpcKeyMapping          (CPC::ECpcKey eCpcKey) const  { return ( eCpcKey<CPC::CPCKEY_LAST ? m_anKeyMappings[eCpcKey] : 0 ); }


private:

  static const char*        SETTINGS_FILE_NAME;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  bool                      m_bOk;

  CPC::CMachine::EModel     m_eCpcModel;
  bool                      m_bDrawScanLines;
  float                     m_fEmulationSpeed;
  int                       m_anKeyMappings[CPC::CPCKEY_LAST];

};

#endif // _SETTINGS_H_
