//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Application.h"
#include "cpcMachine.h"
#include "AppWindow.h"
#include "Window/kmbWindow.h"
#include "Timer/kmbPrecisionTimer.h"

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

  // Emulator
  if (bRet)
  {
    m_pMachine = new CPC::CMachine( CPC::CMachine::MODEL_464/*MODEL_6128*/ );
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
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void Application::FreeVars()
{
  delete m_pAppWindow;
  delete m_pMachine;
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
      //////if ( (GetAccelerators() == NULL) ||                                             // If there is no accelerators loaded...
      //////     !ShouldProcessAccelerator(Msg) ||                                          // If the window doesn't want to process this particular accelerator...
      //////     (::TranslateAccelerator(m_pAppWindow->GetHWnd(), GetAccelerators(), &Msg) == 0) )  // If this message doesn't translates to any accelerator...
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
  kmbPrecisionTimer        executionTimer;
  kmbPrecisionTimer::Value previousTimerValue;
  kmbPrecisionTimer::Value currentTimerValue;
  double                   dLogicTimeAccum = 0.f;

  // Initialize the execution timer, which is used to control the execution of the emulator
  executionTimer.Init();
  executionTimer.Read( &currentTimerValue );

  // Enter the main loop
  while ( !m_bExitApp )
  {
    // Process Windows messages
    ProcessWindowsMessages();

    // Update the execution timer
    previousTimerValue = currentTimerValue;
    executionTimer.Read( &currentTimerValue );

    double dDeltaTime;
    dDeltaTime = executionTimer.ComputeElapsedSecs( previousTimerValue, currentTimerValue );

    // Run the emulated machine
    static const unsigned TIME_STEP = 64;    // 64ms is the time taken by the monitor to raster one scan line.

//****************************************** TODO - TODO - TODO ************************************************
//****************************************** TODO - TODO - TODO ************************************************
    m_pMachine->Run( TIME_STEP );

    // Has the emulated machine completed a new video frame?
    if ( (m_uFrameCount < m_pMachine->GetFrameCount()) || (::GetAsyncKeyState(VK_SPACE) & 0x8000)/***PRUEBAS***/ )
    {
      // Grab the new display image
      m_pAppWindow->UpdateDisplayImage();
      m_pAppWindow->InvalidateAll( false );

      m_uFrameCount = m_pMachine->GetFrameCount();
    }
//****************************************** TODO - TODO - TODO ************************************************
//****************************************** TODO - TODO - TODO ************************************************
  }
}
