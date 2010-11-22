//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "Misc/kmbSingleton.h"
#include "Settings.h"
#include "Timer/kmbPrecisionTimer.h"

class AppWindow;
class WindowsKeyStateProvider;
namespace CPC
{
  class CMachine;
}


/**
** 
*/
class Application : public kmbSingleton<Application>
{
public:

                            Application               ()  { m_bOk = false; }
  virtual                  ~Application               ()  { End(); }

  bool                      Init                      (HINSTANCE hInstance);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  /** Requests to exit the application. The exit is carried at the end of the current frame. */
  void                      RequestExitApp            ()  { m_bExitApp = true; }

  /** Returns the application settings. Do not change settings directly, use specific methods like Application::ChangeCpcModelSetting instead. */
  Settings*                 GetSettings               ()        { return &m_settings; }
  /** Returns the application settings (const version). */
  const Settings*           GetSettings               () const  { return &m_settings; }

  void                      ChangeCpcModelSetting     (CPC::CMachine::EModel eNewModel);
  void                      ChangeDrawScanLinesSetting(bool bDrawScanLines);
  void                      ChangeEmulationSpeedSetting (float fEmulationSpeed);

  /** Returns the machine being emulated. */
  CPC::CMachine*            GetEmulatedMachine        ()        { return m_pMachine; }
  /** Returns the machine being emulated (const version). */
  const CPC::CMachine*      GetEmulatedMachine        () const  { return m_pMachine; }

  /** Returns the application window. */
  AppWindow*                GetAppWindow              ()  { return m_pAppWindow; }


  /** Runs the application. */
  void                      Run                       ();

  // Notifications from the application window
  void                      _OnAppWindowCloseRequest  (AppWindow* pAppWindow);


private:

  static const unsigned     FRAME_DURATION_USECS = 20000;


  void                      ResetVars                 ();
  void                      FreeVars                  ();

  void                      ProcessWindowsMessages    ();
  void                      UpdateMenu                ();


  bool                      m_bOk;

  bool                      m_bExitApp;               // True when a request to exit the application has been made.
  Settings                  m_settings;
  AppWindow*                m_pAppWindow;

  CPC::CMachine*            m_pMachine;
  unsigned                  m_uFrameCount;
  WindowsKeyStateProvider*  m_pKeyStateProvider;

  kmbPrecisionTimer         m_executionTimer;
  kmbPrecisionTimer::Value  m_previousTimerValue;
  kmbPrecisionTimer::Value  m_currentTimerValue;

};

#endif // _APPLICATION_H_
