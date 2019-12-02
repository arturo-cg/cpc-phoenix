//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Application.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"
#include "cpcDiskDrive.h"
#include "cpcDskDisk.h"
#include "AppWindow.h"
#include "StatusBar.h"
#include "WindowsKeyStateProvider.h"
#include "WinVideoOutput.h"
#include "WinSoundOutput.h"
#include "Window/kmbWindow.h"
#include "Msb/kmbMsbManager.h"
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

    // Initialize MSB manager.
    if (bRet)
    {
        new kmbMsbManager;
        bRet = kmbMsbManager::Singleton()->Init();
    }

    // Load the user settings
    if (bRet)
    {
        m_settings.Init();
        m_settings.LoadFromFile();
    }

    // Key state provider, video and sound output
    if (bRet)
    {
        m_pKeyStateProvider = new WindowsKeyStateProvider();

        m_pSoundOutput = new CWinSoundOutput();
        m_pSoundOutput->Init();
        m_pSoundOutput->SetVolume(0.1f);   // TODO - Move volume to CSettings
    }

    // Emulated machine (emulator)
    if (bRet)
    {
        CreateMachine();
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
void Application::ResetVars()
{
    m_bExitApp = false;
    m_pAppWindow = NULL;
    m_pMachine = NULL;
    m_uFrameCount = 0;
    m_pKeyStateProvider = NULL;
    m_pVideoOutput = NULL;
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
        m_pMachine->SetSoundOutput(NULL);
        m_pSoundOutput->End();
        delete m_pSoundOutput;
    }

    delete m_pKeyStateProvider;
    delete m_pAppWindow;
    DestroyMachine();
    m_settings.End();
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::CreateMachine()
{
    // Destroy current machine, if any
    DestroyMachine();

    // Create new machine.
    m_pMachine = new CPC::CMachine(GetSettings()->GetCpcModel(), m_pKeyStateProvider);

    // Video output.
    m_pVideoOutput = new CWinVideoOutput(m_pMachine);
    m_pVideoOutput->Init();
    m_pMachine->SetVideoOutput(m_pVideoOutput);

    // Sound output.
    m_pMachine->SetSoundOutput(m_pSoundOutput);

    // Monitor color output type (color, green).
    m_pMachine->GetGateArray()->SetRgbConversionTable(GetSettings()->GetMonitorType());

    // Insert disks into the drives, if required.
    SetDisk(0, m_settings.GetDiskImage(0));
    SetDisk(1, m_settings.GetDiskImage(1));

    m_pMachine->Reset();
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::DestroyMachine()
{
    if (m_pMachine != NULL)
    {
        delete m_pMachine;
        m_pMachine = NULL;
    }

    if (m_pVideoOutput != NULL)
    {
        delete m_pVideoOutput;
        m_pVideoOutput = NULL;
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::_OnAppWindowCloseRequest(AppWindow* pAppWindow)
{
    if ((pAppWindow != NULL) && (pAppWindow == m_pAppWindow))
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
    GetSettings()->SetCpcModel(eNewModel);

    // Delete the current machine and create the new one
    CreateMachine();
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
    GetSettings()->SetMonitorType(eMonitorType);

    // Set the new monitor type
    m_pMachine->GetGateArray()->SetRgbConversionTable(eMonitorType);

    // Notify the application window
    m_pAppWindow->OnApplicationSettingsChanged();
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::ChangeScaleSetting(float scale)
{
    // Change application settings
    GetSettings()->SetScale(scale);

    // Resize the window.
    m_pAppWindow->ResizeToScale(scale);

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
    GetSettings()->SetDrawScanLines(bDrawScanLines);

    // TODO: Apply/remove the effect
    //////m_pMachine->GetVideoOutput()->SetScanLineEffectActivated( bDrawScanLines );

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
    GetSettings()->SetEmulationSpeed(fEmulationSpeed);

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
        pDrive = m_pMachine->GetDiskDrive(nDrive);

        // Eject current disk
        CPC::CDisk* pOldDisk;
        pOldDisk = pDrive->GetDisk();
        if (pOldDisk != NULL)
        {
            pDrive->SetDisk(NULL);
            delete pOldDisk;

            GetSettings()->SetDiskImage(nDrive, "");
        }

        // Load the image, if any
        if (!sDiskImageFileName.empty())
        {
            kmbFileInputStream stream;
            if (stream.Init(sDiskImageFileName))
            {
                CPC::CDskDisk* pDisk;
                pDisk = new CPC::CDskDisk;
                if (pDisk->LoadImageFromStream(&stream))
                {
                    m_pMachine->GetDiskDrive(nDrive)->SetDisk(pDisk);
                    GetSettings()->SetDiskImage(nDrive, sDiskImageFileName);
                }
                else
                {
                    delete pDisk;
                    pDisk = NULL;
                    ::MessageBox(NULL, "Unknown disk image format.", "Disk image error", MB_OK | MB_ICONEXCLAMATION);
                }
            }
            else
            {
                ::MessageBox(NULL, "Could not open the disk image.", "Disk image error", MB_OK | MB_ICONEXCLAMATION);
            }
        }
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
        if (true/*IsAppActive()*/)
        {
            bGotMsg = ::PeekMessage(&Msg, NULL, 0U, 0U, PM_REMOVE);
        }
        else
        {
            bGotMsg = ::GetMessage(&Msg, NULL, 0U, 0U);
        }

        if (bGotMsg)
        {
            // Translate keystrokes to accelerator commands
            if (::TranslateAccelerator(m_pAppWindow->GetHWnd(), m_pAppWindow->GetAccelerators(), &Msg) == 0)  // If this message doesn't translate to any accelerator...
            {
                // Translate virtual-key messages into character messages
                ::TranslateMessage(&Msg);
            }

            // Dispatch the message
            ::DispatchMessage(&Msg);
        }
    } while (bGotMsg);
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::Run()
{
    // Set Windows timer resolution.
    // This improves the accuracy of the Sleep function.
    static const UINT WINDOWS_TIMER_RESOLUTION = 1;
    timeBeginPeriod(WINDOWS_TIMER_RESOLUTION);

    // Initialize the execution timer, which is used to control the execution of the emulator
    m_executionTimer.Init();
    m_executionTimer.Read(&m_currentTimerValue);
    m_previousTimerValue = m_currentTimerValue;

    double dLeftOverDeltaTimeUsecs = 0.0;

    // Enter the main loop
    while (!m_bExitApp)
    {
        // Run the emulated machine
        static const unsigned TIME_STEP_USECS = 1;
        m_pMachine->Run(TIME_STEP_USECS);

        // Has the emulated machine completed a new video frame?
        if (m_uFrameCount < m_pVideoOutput->GetFrameCount())
        {
            // Draw new video output.
            m_pAppWindow->DrawVideoOutput();
            m_uFrameCount = m_pVideoOutput->GetFrameCount();

            // Process Windows messages.
            // Note: ideally, this wouldn't be tied to a frame of the emulated frame, but rather it would have its own real time counter that triggered Windows message processing
            //       at regular real time intervals. In practice, the emulator runs at full speed in most scenarios (or fast enough in Debug) so this will do.
            ProcessWindowsMessages();

            // Limit the emulation speed.
            double deltaTimeUsecs;
            double adjustedDeltaTimeUsecs;
            bool wait = true;
            while (wait)
            {
                m_executionTimer.Read(&m_currentTimerValue);
                deltaTimeUsecs = m_executionTimer.ComputeElapsedUsecs(m_previousTimerValue, m_currentTimerValue);   // Actual elapsed time during this frame so far.
                adjustedDeltaTimeUsecs = (dLeftOverDeltaTimeUsecs + deltaTimeUsecs) * GetSettings()->GetEmulationSpeed();   // Carry over timing error from the previous frame, and scale by the desired emulation speed.
                if ((GetSettings()->GetEmulationSpeed() <= 0.f) ||       // If Emulation Speed is set to Unlimited...
                    (adjustedDeltaTimeUsecs >= FRAME_DURATION_USECS))    // If enough time has already passed...
                {
                    wait = false;
                }
                else
                {
                    // Block the thread for a while to free up the CPU.
                    // Due to the limited resolution of Sleep, we'll only sleep for a fraction of the total time we need to wait and then do an active wait the rest of the way.
                    double waitTimeMsecs = (FRAME_DURATION_USECS - adjustedDeltaTimeUsecs) / 1000.0;
                    if (waitTimeMsecs > WINDOWS_TIMER_RESOLUTION)
                    {
                        DWORD sleepDuration = (DWORD) (waitTimeMsecs - WINDOWS_TIMER_RESOLUTION);
                        Sleep(sleepDuration);
                    }
                }
            }

            m_previousTimerValue = m_currentTimerValue;

            // Measure timing error in this frame and remember it for the next frame.
            dLeftOverDeltaTimeUsecs = adjustedDeltaTimeUsecs - FRAME_DURATION_USECS;
            static const double MAX_LEFT_OVER_DELTA_TIME_USECS = FRAME_DURATION_USECS * 0.2;
            if (dLeftOverDeltaTimeUsecs > MAX_LEFT_OVER_DELTA_TIME_USECS)
            {
                dLeftOverDeltaTimeUsecs = MAX_LEFT_OVER_DELTA_TIME_USECS;
            }

            // Measure the emulation speed
            static unsigned s_nStatusBarUpdateDelay = 0;
            if (s_nStatusBarUpdateDelay == 0)
            {
                float fEmulationSpeed;
                fEmulationSpeed = (float)((FRAME_DURATION_USECS * 100.0) / deltaTimeUsecs);
                GetAppWindow()->GetStatusBar()->SetEmulationSpeed(fEmulationSpeed);
                s_nStatusBarUpdateDelay = 25;
            }
            s_nStatusBarUpdateDelay--;
        }
    }

    // Restore previous Windows timer resolution.
    timeEndPeriod(WINDOWS_TIMER_RESOLUTION);

    // Save user settings.
    m_settings.SaveToFile();
}
