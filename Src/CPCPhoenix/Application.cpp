//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Application.h"
#include "cpcMachine.h"
#include "cpcDisplay.h"
#include "cpcGateArray.h"
#include "cpcDiskDrive.h"
#include "cpcDskDisk.h"
#include "AppWindow.h"
#include "StatusBar.h"
#include "WindowsKeyStateProvider.h"
#include "WinSoundOutput.h"
#include "Window/kmbWindow.h"
#include "Stream/kmbFileInputStream.h"

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

  // Key state provider and sound output
  if (bRet)
  {
    m_pKeyStateProvider = new WindowsKeyStateProvider();

    m_pSoundOutput = new CWinSoundOutput();
    m_pSoundOutput->Init();
    m_pSoundOutput->SetVolume( 0.1f );   // TODO - Move volume to CSettings
  }

  // Emulator
  if (bRet)
  {
    m_pMachine = new CPC::CMachine( GetSettings()->GetCpcModel(), m_pKeyStateProvider );
    m_pMachine->SetSoundOutput( m_pSoundOutput );
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
  m_pSoundOutput = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void Application::FreeVars()
{
  if (m_pSoundOutput != NULL)
  {
    m_pMachine->SetSoundOutput( NULL );
    m_pSoundOutput->End();
    delete m_pSoundOutput;
  }

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
void Application::ChangeMonitorTypeSetting(CPC::CGateArray::ERgbConversionTableType eMonitorType)
{
  // Change application settings
  GetSettings()->SetMonitorType( eMonitorType );

  // Set the new monitor type
  m_pMachine->GetGateArray()->SetRgbConversionTable( eMonitorType );

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
void Application::SetDisk(unsigned nDrive, const std::string& sDiskImageFileName)
{
  if (nDrive < CPC::CMachine::DRIVE_COUNT)
  {
    CPC::CDiskDrive* pDrive;
    pDrive = m_pMachine->GetDiskDrive( nDrive );

    // Eject current disk
    CPC::CDisk* pOldDisk;
    pOldDisk = pDrive->GetDisk();
    if (pOldDisk != NULL)
    {
      pDrive->SetDisk( NULL );
      delete pOldDisk;

      GetSettings()->SetDiskImage( nDrive, "" );
    }

    // Load the image, if any
    if ( !sDiskImageFileName.empty() )
    {
      kmbFileInputStream stream;
      if ( stream.Init(sDiskImageFileName) )
      {
        CPC::CDskDisk* pDisk;
        pDisk = new CPC::CDskDisk;
        if ( pDisk->LoadImage(&stream) )
        {
          m_pMachine->GetDiskDrive(0)->SetDisk( pDisk );
          GetSettings()->SetDiskImage( nDrive, sDiskImageFileName );
        }
        else
        {
          delete pDisk;
          pDisk = NULL;
          ::MessageBox( NULL, "Unknown disk image format.", "Disk image error", MB_OK | MB_ICONEXCLAMATION );
        }
      }
      else
      {
        ::MessageBox( NULL, "Could not open the disk image.", "Disk image error", MB_OK | MB_ICONEXCLAMATION );
      }
    }

    // Notify the application window
    m_pAppWindow->OnApplicationSettingsChanged();
  }
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
