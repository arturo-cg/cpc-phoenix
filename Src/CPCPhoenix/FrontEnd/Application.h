//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "Misc/kmbSingleton.h"

class AppWindow;
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

  void                      ResetVars                 ();
  void                      FreeVars                  ();

  void                      ProcessWindowsMessages    ();


  bool                      m_bOk;

  bool                      m_bExitApp;               // True when a request to exit the application has been made.
  AppWindow*                m_pAppWindow;

  CPC::CMachine*            m_pMachine;
  unsigned                  m_uFrameCount;

};

#endif // _APPLICATION_H_
