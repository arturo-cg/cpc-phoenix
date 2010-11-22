//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Application.h"
#include "cpcMachine.h"
#include "cpcDisplay.h"
#include "AppWindow.h"
#include "StatusBar.h"
#include "WindowsKeyStateProvider.h"
#include "Window/kmbWindow.h"

#include <CommCtrl.h>


template<> Application* kmbSingleton<Application>::m_pSingleton = NULL;



//----------------------------------------------------------------------------
/**
** Init
*/
bool Application::Init(HINSTANCE hInstance)
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    //...
  }

  // Load the application settings
  if (bRet)
  {
    m_settings.Init();
  }

  // Key state provider
  if (bRet)
  {
    m_pKeyStateProvider = new WindowsKeyStateProvider();
  }

  // Emulator
  if (bRet)
  {
    m_pMachine = new CPC::CMachine( GetSettings()->GetCpcModel(), m_pKeyStateProvider );
    m_pMachine->Reset();
  }

  // Application main window
  if (bRet)
  {
    // Register window classes
    kmbWindow::RegisterWindowClass();    // This must be called only once, before creating any kmbWindow

    // Create the application window
    m_pAppWindow = new AppWindow;
    m_pAppWindow->Init();
  }


  if (bRet)
    m_bOk = true;
  else
    FreeVars();

  return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void Application::End()
{
  if( IsOk() )
  {
    FreeVars();
    m_bOk = false;
  }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void Application::ResetVars()
{
  m_bExitApp    = false;
  m_pAppWindow  = NULL;
  m_pMachine    = NULL;
  m_uFrameCount = 0;
  m_pKeyStateProvider = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void Application::FreeVars()
{
  delete m_pKeyStateProvider;
  delete m_pAppWindow;
  delete m_pMachine;
  m_settings.End();
}

//----------------------------------------------------------------------------
/**
** 
*/
void Application::_OnAppWindowCloseRequest(AppWindow* pAppWindow)
{
  if ( (pAppWindow != NULL) && (pAppWindow == m_pAppWindow) )
  {
    RequestExitApp();
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
void Application::ChangeCpcModelSetting(CPC::CMachine::EModel eNewModel)
{
  // Change application settings
  GetSettings()->SetCpcModel( eNewModel );

  // Delete the current machine and create the new one
  delete m_pMachine;
  m_pMachine = new CPC::CMachine( eNewModel, m_pKeyStateProvider );
  m_pMachine->Reset();
  m_uFrameCount = 0;

  // Notify the application window
  m_pAppWindow->OnApplicationSettingsChanged();
}

//----------------------------------------------------------------------------
/**
** 
*/
void Application::ChangeDrawScanLinesSetting(bool bDrawScanLines)
{
  // Change application settings
  GetSettings()->SetDrawScanLines( bDrawScanLines );

  // Apply/remove the effect
  m_pMachine->GetDisplay()->SetScanLineEffectActivated( bDrawScanLines );

  // Notify the application window
  m_pAppWindow->OnApplicationSettingsChanged();
}

//----------------------------------------------------------------------------
/**
** 
*/
void Application::ChangeEmulationSpeedSetting(float fEmulationSpeed)
{
  // Change application settings
  GetSettings()->SetEmulationSpeed( fEmulationSpeed );

  // Notify the application window
  m_pAppWindow->OnApplicationSettingsChanged();
}

//----------------------------------------------------------------------------
/**
** 
*/
void Application::ProcessWindowsMessages()
{
  BOOL bGotMsg;
  MSG  Msg;

  // Dispatch all the pending window messages
  do 
  {
    // Get the next pending message, if any. If the application is not active,
    // block until a message is received.
    if ( true/*IsAppActive()*/ )
    {
      bGotMsg = ::PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE );
    }
    else
    {
      bGotMsg = ::GetMessage( &Msg, NULL, 0U, 0U );
    }

    if (bGotMsg)
    {
      // Translate keystrokes to accelerator commands
      if (::TranslateAccelerator(m_pAppWindow->GetHWnd(), m_pAppWindow->GetAccelerators(), &Msg) == 0)  // If this message doesn't translates to any accelerator...
      {
        // Translate virtual-key messages into character messages
        ::TranslateMessage( &Msg );
      }

      // Dispatch the message
      ::DispatchMessage( &Msg );
    }
  } while( bGotMsg );
}

//----------------------------------------------------------------------------
/**
** 
*/
void Application::Run()
{
  double                   dLogicTimeAccum = 0.f;

  // Initialize the execution timer, which is used to control the execution of the emulator
  m_executionTimer.Init();
  m_executionTimer.Read( &m_currentTimerValue );

  // Enter the main loop
  while ( !m_bExitApp )
  {
    // Process Windows messages
    ProcessWindowsMessages();

    // Run the emulated machine
    static const unsigned TIME_STEP_USECS = 10;

    m_pMachine->Run( TIME_STEP_USECS );

    // Has the emulated machine completed a new video frame?
    if (m_uFrameCount < m_pMachine->GetFrameCount())
    {
      // Grab the new display image
      m_pAppWindow->UpdateDisplayImage();
      m_pAppWindow->InvalidateAll( false );

      m_uFrameCount = m_pMachine->GetFrameCount();

      // Limit the emulation speed
      double dDeltaTimeSecs;
      double dDeltaTimeUSecs;
      do
      {
        m_executionTimer.Read( &m_currentTimerValue );
        dDeltaTimeSecs  = m_executionTimer.ComputeElapsedSecs( m_previousTimerValue, m_currentTimerValue );
        dDeltaTimeUSecs = dDeltaTimeSecs * 1000000.0;
      } while ( (GetSettings()->GetEmulationSpeed() > 0.f) && ((dDeltaTimeUSecs * GetSettings()->GetEmulationSpeed()) < double(FRAME_DURATION_USECS)) );


      // Measure the emulation speed
      static unsigned s_nStatusBarUpdateDelay = 0;
      if (s_nStatusBarUpdateDelay == 0)
      {
        float fEmulationSpeed;
        fEmulationSpeed = (float) ( (double(FRAME_DURATION_USECS) * 100.0) / dDeltaTimeUSecs );
        GetAppWindow()->GetStatusBar()->SetEmulationSpeed( fEmulationSpeed );
        s_nStatusBarUpdateDelay = 25;
      }
      s_nStatusBarUpdateDelay--;

      m_previousTimerValue = m_currentTimerValue;
    }
  }

  // TODO - Save application settings
  // TODO - Save application settings
  // TODO - Save application settings
}
