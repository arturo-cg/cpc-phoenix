//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Application.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"
#include "cpcDiskDrive.h"
#include "cpcDskDisk.h"
#include "AppWindow.h"
#include "DisplayWindow.h"
#include "RenderingApi.h"
#include "StatusBar.h"
#include "WindowsKeyStateProvider.h"
#include "WinVideoOutput.h"
#include "WinSoundOutput.h"
#include "Window/kmbWindow.h"
#include "Msb/kmbMsbManager.h"
#include "Stream/kmbFileInputStream.h"

#include <CommCtrl.h>


template<> Application* kmbSingleton<Application>::m_pSingleton = NULL;

/*static*/ const string Application::StandardCpc464SpecificationsName = "cpc_464";
/*static*/ const string Application::StandardCpc664SpecificationsName = "cpc_664";
/*static*/ const string Application::StandardCpc6128SpecificationsName = "cpc_6128";



//----------------------------------------------------------------------------
/**
** Init
*/
bool Application::Init(HINSTANCE hInstance)
{
    bool bRet = true;

    End();
    ResetVars();

    // MSB manager.
    new kmbMsbManager;
    kmbMsbManager::Singleton()->Init();
    // Machine specifications.
    InitializeMachineSpecifications();
    // Load user settings.
    m_settings.Init();
    m_settings.LoadFromFile();
    // Key state provider.
    m_pKeyStateProvider = new WindowsKeyStateProvider();
    // Sound output.
    m_pSoundOutput = new CWinSoundOutput();
    m_pSoundOutput->Init();
    m_pSoundOutput->SetVolume(0.1f);   // TODO - Move volume to CSettings
    // Emulated machine (emulator).
    CreateMachine();
    // Application main window.
    ImGui_ImplWin32_EnableDpiAwareness();
    kmbWindow::RegisterWindowClass();    // This must be called only once, before creating any kmbWindow
    m_pAppWindow = new AppWindow;
    m_pAppWindow->Init();
    // Rendering API (Direct3D 11).
    m_renderingApi = new RenderingApi();
    bRet = m_renderingApi->Init(m_pAppWindow->GetDisplayWindow()->GetHWnd());
    if (!bRet)
    {
        ::MessageBox(nullptr, "Failed to initialize Direct3D.", "Error", MB_OK | MB_ICONERROR);
    }
    // GUI (Dear ImGui).
    if (bRet)
    {
        InitializeGui();
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
    m_renderingApi = nullptr;
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
    ShutdownGui();

    if (m_pSoundOutput != NULL)
    {
        m_pMachine->SetSoundOutput(NULL);
        m_pSoundOutput->End();
        delete m_pSoundOutput;
    }

    delete m_pKeyStateProvider;
    delete m_renderingApi;
    delete m_pAppWindow;
    DestroyMachine();
    m_settings.End();
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::InitializeMachineSpecifications()
{
    m_machineSpecifications.clear();
    m_orderedMachineSpecificationsNames.clear();

    //
    // Built-in machine specifications.
    //
    CPC::MachineSpecifications machineSpecifications;
    // +- Standard CPC 464.
    CPC::CMachine::GetStandardCpc464Specifications(&machineSpecifications);
    m_machineSpecifications.insert({ StandardCpc464SpecificationsName, machineSpecifications });
    m_orderedMachineSpecificationsNames.push_back(StandardCpc464SpecificationsName);
    // +- Standard CPC 664.
    CPC::CMachine::GetStandardCpc664Specifications(&machineSpecifications);
    m_machineSpecifications.insert({ StandardCpc664SpecificationsName, machineSpecifications });
    m_orderedMachineSpecificationsNames.push_back(StandardCpc664SpecificationsName);
    // +- Standard CPC 6128.
    CPC::CMachine::GetStandardCpc6128Specifications(&machineSpecifications);
    m_machineSpecifications.insert({ StandardCpc6128SpecificationsName, machineSpecifications });
    m_orderedMachineSpecificationsNames.push_back(StandardCpc6128SpecificationsName);
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
    // +- Look up the machine specifications.
    const CPC::MachineSpecifications* machineSpecifications = FindMachineSpecificationsByName(GetSettings()->GetMachineSpecificationName());
    if (machineSpecifications != nullptr)
    {
        // +- Create the machine.
        m_pMachine = new CPC::CMachine(*machineSpecifications, m_pKeyStateProvider);

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
    else
    {
        // Machine specifications doesn't exist. Change it to one of the built-in specifications.
        // This causes this method to be called again.
        ChangeMachineSpecificationName(StandardCpc6128SpecificationsName);
    }
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

void Application::InitializeGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;
    //io.ConfigViewportsNoDefaultParent = true;
    //io.ConfigDockingAlwaysTabBar = true;
    //io.ConfigDockingTransparentPayload = true;
//#if 1
//    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
//    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI
//#endif

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(m_pAppWindow->GetHWnd());
    ImGui_ImplDX11_Init(m_renderingApi->GetDevice(), m_renderingApi->GetDeviceContext());
}

void Application::ShutdownGui()
{
    // Dear ImGuy.
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
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
const CPC::MachineSpecifications* Application::FindMachineSpecificationsByName(string name) const
{
    StringToMachineSpecificationsMap::const_iterator iter = m_machineSpecifications.find(name);
    return (iter != m_machineSpecifications.end() ? &iter->second : nullptr);
}

//----------------------------------------------------------------------------
/**
**
*/
const string* Application::GetMachineSpecificationsNameAtPosition(unsigned position) const
{
    return (position < m_orderedMachineSpecificationsNames.size() ? &m_orderedMachineSpecificationsNames[position] : nullptr);
}

//----------------------------------------------------------------------------
/**
**
*/
unsigned Application::FindMachineSpecificationsOrderedPosition(string name) const
{
    unsigned ret;
    for (ret = 0; ret < m_orderedMachineSpecificationsNames.size(); ret++)
    {
        if (name == m_orderedMachineSpecificationsNames[ret])
        {
            // Found.
            break;
        }
    }

    return ret;
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::ChangeMachineSpecificationName(string machineSpecificationName)
{
    // Change application settings
    GetSettings()->SetMachineSpecificationName(machineSpecificationName);

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
            // Process Windows messages.
            // Note: ideally, this wouldn't be tied to a frame of the emulated frame, but rather it would have its own real time counter that triggered Windows message processing
            //       at regular real time intervals. In practice, the emulator runs at full speed in most scenarios (or fast enough in Debug) so this will do.
            ProcessWindowsMessages();

            // Start the Dear ImGui frame.
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Show the big demo window (most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            bool show_demo_window = true;
            ImGui::ShowDemoWindow(&show_demo_window);

            // Draw new video output.
            m_pAppWindow->DrawVideoOutput();
            m_uFrameCount = m_pVideoOutput->GetFrameCount();

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

            // Render.
            Render();
        }
    }

    // Restore previous Windows timer resolution.
    timeEndPeriod(WINDOWS_TIMER_RESOLUTION);

    // Save user settings.
    m_settings.SaveToFile();
}

void Application::Render()
{
    ImGui::Render();
    m_renderingApi->PrepareForRender(RenderingApi::COLOR_MAGENTA);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows.
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    m_renderingApi->Present(false/*vsync*/);
}