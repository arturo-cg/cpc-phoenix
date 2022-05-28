//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Application.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"
#include "cpcDiskDrive.h"
#include "cpcDskDisk.h"
#include "AppWindow.h"
#include "RenderingApi.h"
#include "Debugger.h"
#include "Snapshot.h"
#include "SnaSnapshotReadWrite.h"
#include "WindowsKeyStateProvider.h"
#include "TextureVideoOutput.h"
#include "WinSoundOutput.h"
#include "Archive/kmbZipArchive.h"
#include "Window/kmbWindow.h"
#include "Msb/kmbMsbManager.h"
#include "Stream/kmbFileInputStream.h"
#include "Stream/kmbFileOutputStream.h"
#include "Stream/kmbMemoryInputStream.h"

#include <CommCtrl.h>


template<> Application* kmbSingleton<Application>::m_pSingleton = NULL;

/*static*/ const string Application::StandardCpc464SpecificationsName = "cpc_464";
/*static*/ const string Application::StandardCpc664SpecificationsName = "cpc_664";
/*static*/ const string Application::StandardCpc6128SpecificationsName = "cpc_6128";

static const string QuickSnapshotDirectory = "Snapshots";
static const string QuickSnapshotFile = "QuickSnapshot.sna";

static const float StatusBarHeight = 58.f;

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
    // Application main window.
    ImGui_ImplWin32_EnableDpiAwareness();
    kmbWindow::RegisterWindowClass();    // This must be called only once, before creating any kmbWindow
    m_pAppWindow = new AppWindow;
    m_pAppWindow->Init(m_settings.GetMainWindowX(), m_settings.GetMainWindowY(), m_settings.GetMainWindowWidth(), m_settings.GetMainWindowHeight());
    // Rendering API (Direct3D 11).
    m_renderingApi = new RenderingApi();
    bRet = m_renderingApi->Init(m_pAppWindow->GetHWnd());
    if (!bRet)
    {
        ::MessageBox(nullptr, "Failed to initialize Direct3D.", "Error", MB_OK | MB_ICONERROR);
    }
    // GUI (Dear ImGui).
    if (bRet)
    {
        m_showDearImGuiDemoWindow = false;
        InitializeGui();
    }
    // Emulated machine.
    if (bRet)
    {
        // Key state provider.
        m_pKeyStateProvider = new WindowsKeyStateProvider();
        // Video output.
        m_videoOutput = new TextureVideoOutput();
        m_videoOutput->Init();
        // Sound output.
        m_pSoundOutput = new CWinSoundOutput();
        m_pSoundOutput->Init();
        m_pSoundOutput->SetVolume(0.1f);   // TODO - Move volume to CSettings
        // Emulated machine.
        CreateMachine();
    }
    // Debugger.
    if (bRet)
    {
        m_debugger = new Debugger();
        bRet = m_debugger->Init();
    }
    // Others.
    if (bRet)
    {
        m_measuredEmulationSpeed = 0.f;
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
    m_pKeyStateProvider = NULL;
    m_videoOutput = NULL;
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
        m_pSoundOutput = NULL;
    }

    if (m_videoOutput != NULL)
    {
        m_pMachine->SetVideoOutput(NULL);
        m_videoOutput->End();
        delete m_videoOutput;
        m_videoOutput = NULL;
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
        // Create the machine.
        m_pMachine = new CPC::CMachine(*machineSpecifications, m_pKeyStateProvider);
        // Video & sound output.
        m_videoOutput->SetMachine(m_pMachine);
        m_pMachine->SetVideoOutput(m_videoOutput);
        m_pMachine->SetSoundOutput(m_pSoundOutput);
        // Monitor color output type (color, green).
        m_pMachine->GetGateArray()->SetRgbConversionTable(GetSettings()->GetMonitorType());
        // Insert disks into the drives, if required.
        SetDisk(0, m_settings.GetDiskImage(0), m_settings.GetDiskImageArchive(0));
        SetDisk(1, m_settings.GetDiskImage(1), m_settings.GetDiskImageArchive(1));

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
    io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = true;
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

bool Application::_OnAppWindowKeyDown(unsigned virtualKey, bool shift, bool ctrl, bool alt)
{
    bool ret = false;
    switch (virtualKey)
    {
    case VK_F3:
    {
        if (!shift && !ctrl && !alt)
        {
            // Emulation speed = 100%
            ChangeEmulationSpeedSetting(1.f);
            ret = true;
        }
        break;
    }
    case VK_F4:
    {
        if (!shift && !ctrl && !alt)
        {
            // Emulation speed = Unlimited
            ChangeEmulationSpeedSetting(-1.f);
            ret = true;
        }
        else if (shift && !ctrl && !alt)
        {
            // Close app.
            m_pAppWindow->RequestClose();
            ret = true;
        }
        break;
    }
    case VK_F5:
    {
        if (!shift && !ctrl && !alt)
        {
            // Debugger on/off.
            m_debugger->SetActive(!m_debugger->IsActive());
            ret = true;
        }
        else if (shift && !ctrl && !alt)
        {
            // Reset machine.
            m_pMachine->Reset();
            ret = true;
        }
        break;
    }
    case VK_F7:
    {
        if (!shift && !ctrl && !alt)
        {
            // Save snapshot (with file dialog).
            SaveSnapshotWithFileDialog();
            ret = true;
        }
        else if (shift && !ctrl && !alt)
        {
            // Save quick snapshot.
            SaveQuickSnapshot();
            ret = true;
        }
        break;
    }
    case VK_F8:
    {
        if (!shift && !ctrl && !alt)
        {
            // Load snapshot (with file dialog).
            LoadSnapshotWithFileDialog();
            ret = true;
        }
        else if (shift && !ctrl && !alt)
        {
            // Load quick snapshot.
            LoadQuickSnapshot();
            ret = true;
        }
        break;
    }
    default:
        break;
    }

    return ret;
}

void Application::_OnAppWindowMoved()
{
    // Remember new window position.
    RECT mainWindowRect;
    m_pAppWindow->GetRect(&mainWindowRect);
    m_settings.SetMainWindowPosition(mainWindowRect.left, mainWindowRect.top);
}

void Application::_OnAppWindowSizing()
{
    // Keep updating the window while the user is resizing it.
    Render();
}

void Application::_OnAppWindowSizeChanged()
{
    // Remember new window size.
    RECT mainWindowRect;
    m_pAppWindow->GetRect(&mainWindowRect);
    m_settings.SetMainWindowSize(mainWindowRect.right - mainWindowRect.left, mainWindowRect.bottom - mainWindowRect.top);
}

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
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::ChangeEmulationSpeedSetting(float fEmulationSpeed)
{
    // Change application settings
    GetSettings()->SetEmulationSpeed(fEmulationSpeed);
}

//----------------------------------------------------------------------------
/**
**
*/
void Application::MainLoop()
{
    // Set Windows timer resolution.
    // This improves the accuracy of the Sleep function.
    timeBeginPeriod(WINDOWS_TIMER_RESOLUTION);

    // Initialize timers.
    m_precisionTimer.Init();
    m_precisionTimer.Read(&m_previousPrecisionTimerValue);
    m_emulationTime = 0.0;
    m_renderTime = 0.0;
    m_windowsMessagesTime = 0.0;
    m_speedRealTime = 0.0;
    m_speedEmulatedTime = 0.0;

    // Main loop.
    while (!m_bExitApp)
    {
        // Measure elapsed real time.
        double elapsedRealTime = ComputeElapsedRealTime();
        // Emulated machine / debugger.
        if (m_debugger->IsActive())
        {
            // The machine gets updated by the debugger as required.
            m_debugger->RunMachine();
        }
        else
        {
            RunMachine(elapsedRealTime);
        }
        // Windows messages.
        ProcessWindowsMessagesIfNecessary(elapsedRealTime);
        // Render.
        RenderIfNecessary(elapsedRealTime);
        // Be nice with the host CPU and go to sleep while there's nothing to do.
        SleepIfIdle();
    }

    // Restore previous Windows timer resolution.
    timeEndPeriod(WINDOWS_TIMER_RESOLUTION);

    // Save user settings.
    m_settings.SaveToFile();
}

double Application::ComputeElapsedRealTime()
{
    kmbPrecisionTimer::Value currentPrecisionTimerValue;
    m_precisionTimer.Read(&currentPrecisionTimerValue);
    double ret = m_precisionTimer.ComputeElapsedSecs(m_previousPrecisionTimerValue, currentPrecisionTimerValue);
    // +- Clamp elapsed time to a maximum value.
    //    Elapsed time can get large for various reasons, for instance after resizing the window or resuming after a debug session.
    static constexpr double MAX_ELAPSED_TIME = 1.0 / 50.0;
    if (ret > MAX_ELAPSED_TIME)
    {
        ret = MAX_ELAPSED_TIME;
    }
    m_previousPrecisionTimerValue = currentPrecisionTimerValue;

    return ret;
}

void Application::RunMachine(double elapsedRealTime)
{
    m_emulationTime += elapsedRealTime;
    // Run the emulated machine.
    if (m_emulationTime >= 0.0)     // If emulated time is behind real time...
    {
        // Run the machine.
        static constexpr unsigned MACHINE_TIME_STEP_CYCLES = unsigned((FRAME_DURATION_SECS * 4000000.0) / 6.0);     // Time step in cycles of a 4-MHz clock.
        static constexpr double MACHINE_TIME_STEP_SECS = (double(MACHINE_TIME_STEP_CYCLES) / 4000000.0);            // Time step in seconds.
        m_pMachine->Run(MACHINE_TIME_STEP_CYCLES);
        // Advance emulated time.
        if (m_settings.GetEmulationSpeed() > 0.f)   // If emulation speed *not* set to unlimited...
        {
            m_emulationTime -= MACHINE_TIME_STEP_SECS / m_settings.GetEmulationSpeed();
        }
        else
        {
            m_emulationTime -= elapsedRealTime;
        }

        m_speedEmulatedTime += MACHINE_TIME_STEP_SECS;
    }
    // Measure emulation speed.
    m_speedRealTime += elapsedRealTime;
    static constexpr double SPEED_UPDATE_PERIOD = 0.4;
    if (m_speedRealTime >= SPEED_UPDATE_PERIOD)
    {
        m_measuredEmulationSpeed = float((m_speedEmulatedTime / m_speedRealTime) * 100.0);
        m_speedEmulatedTime = 0.0;
        m_speedRealTime = 0.0;
    }
}

void Application::ProcessWindowsMessagesIfNecessary(double elapsedRealTime)
{
    m_windowsMessagesTime += elapsedRealTime;
    if (m_windowsMessagesTime >= PROCESS_WINDOWS_MESSAGES_PERIOD)     // If it is time to process Windows messages...
    {
        // Process Windows messages.
        ProcessWindowsMessages();
        m_windowsMessagesTime = fmod(m_windowsMessagesTime - PROCESS_WINDOWS_MESSAGES_PERIOD, PROCESS_WINDOWS_MESSAGES_PERIOD);
    }
}

void Application::ProcessWindowsMessages()
{
    BOOL bGotMsg;
    MSG  Msg;

    // Dispatch all the pending window messages
    do
    {
        // Get the next pending message, if any.
        // If the application is not active, block until a message is received.
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

void Application::RenderIfNecessary(double elapsedRealTime)
{
    m_renderTime += elapsedRealTime;
    // Render at a lower rate while debugging to prevent the Run To command from being painfully slow.
    double renderPeriod = (m_debugger->IsActive() && m_debugger->IsRunning() ? RENDER_PERIOD_WHILE_DEBUGGER_RUNNING : RENDER_PERIOD);
    if (m_renderTime >= renderPeriod)     // If it is time to render a new frame...
    {
        // Render.
        Render();
        m_renderTime -= renderPeriod;
    }
}

void Application::Render()
{
    // Start a new Dear ImGui frame.
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Draw all the GUI.
    // This includes the emulator video output.
    DrawGui();

    // Render everything.
    ImGui::Render();
    m_renderingApi->PrepareForRender(RenderingApi::COLOR_MAGENTA);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Update and Render additional Platform Windows.
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // Make it visible.
    m_renderingApi->Present(false/*vsync*/);
}

void Application::SleepIfIdle()
{
    // Compute idle time.
    double emulationIdleTime = (m_debugger->IsActive() ? 12345.0/*arbitrary large number*/ : (m_emulationTime < 0.0 ? -m_emulationTime : 0.0));     // m_emulationTime < 0 if emulation is ahead of real time.
    double renderIdleTime = RENDER_PERIOD - m_renderTime;
    double windowsMessagesIdleTime = PROCESS_WINDOWS_MESSAGES_PERIOD - m_windowsMessagesTime;
    double idleTime = emulationIdleTime;
    idleTime = (renderIdleTime < emulationIdleTime ? renderIdleTime : emulationIdleTime);
    idleTime = (windowsMessagesIdleTime < idleTime ? windowsMessagesIdleTime : idleTime);
    idleTime *= 1000.0;     // Convert to milliseconds.
    // Block the thread for a while to free up the CPU.
    // Due to the limited resolution of Sleep, we'll sleep for just a little under the total idle time.
    static constexpr double MIN_IDLE_TIME = 0.1;
    if (idleTime > MIN_IDLE_TIME)
    {
        DWORD sleepDuration = (DWORD)(idleTime - MIN_IDLE_TIME);
        Sleep(sleepDuration);
    }
}

void Application::DrawGui()
{
    // Main window.
    DrawMainWindowGui();
    // Debugger.
    if (m_debugger->IsActive())
    {
        m_debugger->DrawGui();
    }
    // Dear ImGui demo window.
    // It should be removed at some point.
    if (m_showDearImGuiDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDearImGuiDemoWindow);
    }
}

void Application::DrawMainWindowGui()
{
    RECT mainWindowRect;
    m_pAppWindow->GetClientRect(&mainWindowRect);
    m_pAppWindow->ClientToScreen((POINT*)&mainWindowRect.left);     // (left, top) = position of client area in screen coordinates; (right, bottom) = size of client area.
    ImGui::SetNextWindowPos(ImVec2((float)mainWindowRect.left, (float)mainWindowRect.top));
    ImGui::SetNextWindowSize(ImVec2(float(mainWindowRect.right), float(mainWindowRect.bottom)));
    // Main Dear ImGui window is always inside the application OS window.
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
    // Main window begin.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();
    // Main menu.
    DrawMainMenuGui();
    // Emulator video output.
    m_videoOutput->DrawGui(StatusBarHeight);
    if (m_debugger->IsActive())
    {
        m_debugger->DrawVideoOutputOverlays();
    }
    // Status bar.
    ImGui::SetCursorPosY(ImGui::GetWindowViewport()->WorkSize.y - StatusBarHeight);
    DrawStatusBarGui();
    // Main window end.
    ImGui::End();
}

void Application::DrawMainMenuGui()
{
    if (ImGui::BeginMenuBar())
    {
        //
        // "File" menu.
        //
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Drive A"))
            {
                DrawDiskDriveMenuGui(0);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Drive B"))
            {
                DrawDiskDriveMenuGui(1);
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save Quick Snapshot", "Shift+F7"))
            {
                SaveQuickSnapshot();
            }
            if (ImGui::MenuItem("Load Quick Snapshot", "Shift+F8"))
            {
                LoadQuickSnapshot();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save Snapshot...", "F7"))
            {
                SaveSnapshotWithFileDialog();
            }
            if (ImGui::MenuItem("Load Snapshot...", "F8"))
            {
                LoadSnapshotWithFileDialog();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                RequestExitApp();
            }
            ImGui::EndMenu();
        }
        //
        // "Settings" menu.
        //
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginCombo("CPC Machine", m_settings.GetMachineSpecificationName().c_str()))
            {
                unsigned currentSpecsIndex = FindMachineSpecificationsOrderedPosition(m_settings.GetMachineSpecificationName());
                unsigned i = 0;
                for (StringList::iterator iter = m_orderedMachineSpecificationsNames.begin(); iter != m_orderedMachineSpecificationsNames.end(); ++iter)
                {
                    string specsName = *iter;
                    if (ImGui::Selectable(specsName.c_str(), i == currentSpecsIndex))
                    {
                        if (i != currentSpecsIndex)
                        {
                            ChangeMachineSpecificationName(specsName);
                        }
                    }
                    i++;
                }
                ImGui::EndCombo();
            }
            int selectedMonitorType = (m_settings.GetMonitorType() == CPC::CGateArray::RGBCONVERSIONTABLE_COLOR ? 0 : 1);
            if (ImGui::Combo("Monitor", &selectedMonitorType, "Color\0Green\0"))
            {
                ChangeMonitorTypeSetting(selectedMonitorType == 0 ? CPC::CGateArray::RGBCONVERSIONTABLE_COLOR : CPC::CGateArray::RGBCONVERSIONTABLE_GREEN);
            }
            ImGui::Separator();

            if (ImGui::Button("25%"))
            {
                ChangeEmulationSpeedSetting(0.25f);
            }
            ImGui::SameLine();
            if (ImGui::Button("50%"))
            {
                ChangeEmulationSpeedSetting(0.5f);
            }
            ImGui::SameLine();
            if (ImGui::Button("100%"))
            {
                ChangeEmulationSpeedSetting(1.f);
            }
            ImGui::SameLine();
            if (ImGui::Button("~120%"))     // = 19968.0 (50.08 fps) / 16666.6667 (60 fps)  <- It speeds up the emulation so that it completes a new frame at a 60 Hz rate. Ideal to appreciate smooth scrolling on a 60 Hz host monitor.
            {
                ChangeEmulationSpeedSetting(1.19808f);
            }
            ImGui::SameLine();
            if (ImGui::Button("Unlimited"))
            {
                ChangeEmulationSpeedSetting(-1.f);
            }
            ImGui::SameLine();
            ImGui::Text("Emulation Speed");
            ImGui::Separator();
            if (ImGui::MenuItem("Reset", "Shift+F5"))
            {
                m_pMachine->Reset();
            }
            ImGui::EndMenu();
        }
        //
        // "Debugger" option.
        //
        bool debuggerActive = m_debugger->IsActive();
        if (ImGui::Checkbox("Debugger", &debuggerActive))
        {
            m_debugger->SetActive(debuggerActive);
        }
        //
        // "Help" menu.
        //
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::BeginMenu("Developer"))
            {
                if (ImGui::MenuItem("Show Dear ImGui Demo Window", nullptr/*shorcut*/, false/*selected*/, !m_showDearImGuiDemoWindow/*enabled*/))
                {
                    m_showDearImGuiDemoWindow = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Application::DrawDiskDriveMenuGui(int driveNumber)
{
    if (ImGui::MenuItem("Insert Disk..."))
    {
        string fullFilePath;
        if (ShowLoadSaveFileDialog(true/*isLoad*/, "\\Disks", "All supported formats (*.dsk, *.zip)\0*.dsk;*.zip\0DSK disk images (*.dsk)\0*.dsk\0ZIP archives (*.zip)\0*.zip\0\0", &fullFilePath))
        {
            // DSK or ZIP file selected?
            if (StringEndsWith(fullFilePath, ".zip"))
            {
                SetDiskFromArchive(driveNumber, fullFilePath);
            }
            else
            {
                SetDiskFromFile(driveNumber, fullFilePath);
            }
        }
    }
    if (ImGui::MenuItem("Eject Disk"))
    {
        EjectDisk(driveNumber);
    }
}

void Application::DrawStatusBarGui()
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    ImGui::BeginChild("StatusBar", ImVec2(0, 0)/*size*/, true/*border*/);
    ImGui::PopStyleColor();

    // Disk drives.
    ImGui::BeginGroup();
    DrawDiskDriveBarGui('A', 0);
    DrawDiskDriveBarGui('B', 1);
    ImGui::EndGroup();
    // Emulation speed.
    ImGui::SameLine();
    ImGui::Text("Speed: ");
    ImGui::SameLine();
    ImGui::TextDisabled("%.1f%%", m_measuredEmulationSpeed);

    ImGui::EndChild();
}

void Application::DrawDiskDriveBarGui(char driveLetter, int driveNumber)
{
    ImGui::BeginGroup();

    string label = "Drive ";
    label += driveLetter;
    label += ":";
    if (ImGui::Button(label.c_str()))
    {
        ImGui::OpenPopup(label.c_str());
    }
    if (ImGui::BeginPopup(label.c_str()))
    {
        DrawDiskDriveMenuGui(driveNumber);
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    string diskImage = m_settings.GetDiskImage(driveNumber);
    if (diskImage.empty())
    {
        diskImage = "<EMPTY>";
    }
    else if (!m_settings.GetDiskImageArchive(driveNumber).empty())
    {
        diskImage = m_settings.GetDiskImageArchive(driveNumber) + " (" + diskImage + ")";
    }

    ImGui::TextDisabled(diskImage.c_str());
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 200.f);       // This aligns the disk drive row to the right and leaves some space for the Speed field.

    ImGui::EndGroup();
    ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_Border));
}

void Application::InsertDisk(unsigned driveNumber, kmbInputStream& diskImageStream, const std::string& diskImageFileName, const std::string& archiveFileName)
{
    KMASSERT(driveNumber < CPC::CMachine::DRIVE_COUNT);
    KMASSERT(diskImageStream.IsOk());

    // Eject current disk.
    EjectDisk(driveNumber);

    // Load the new disk image.
    CPC::CDskDisk* disk;
    disk = new CPC::CDskDisk;
    if (disk->LoadImageFromStream(&diskImageStream))
    {
        // Insert the disk in the drive.
        m_pMachine->GetDiskDrive(driveNumber)->SetDisk(disk);
        // Remember settings.
        GetSettings()->SetDiskImageAndArchive(driveNumber, diskImageFileName, archiveFileName);
    }
    else
    {
        delete disk;
        disk = nullptr;
        ::MessageBox(NULL, "Unknown disk image format.", "Disk image error", MB_OK | MB_ICONEXCLAMATION);
    }
}

void Application::EjectDisk(unsigned driveNumber)
{
    CPC::CDiskDrive* drive;
    drive = m_pMachine->GetDiskDrive(driveNumber);

    CPC::CDisk* disk;
    disk = drive->GetDisk();

    if (disk != nullptr)
    {
        drive->SetDisk(nullptr);
        delete disk;

        GetSettings()->SetDiskImageAndArchive(driveNumber, "", "");
    }
}

void Application::SetDiskFromFile(unsigned driveNumber, const std::string& diskImageFilePath)
{
    kmbFileInputStream diskImageStream;
    if (diskImageStream.Init(diskImageFilePath))
    {
        InsertDisk(driveNumber, diskImageStream, diskImageFilePath, "");
    }
    else
    {
        ::MessageBox(NULL, "Could not open the disk image.", "Disk image error", MB_OK | MB_ICONEXCLAMATION);
    }
}

void Application::SetDiskFromArchive(unsigned driveNumber, const std::string& archiveFilePath)
{
    // Open the ZIP file.
    kmbFileInputStream fileStream;
    if (fileStream.Init(archiveFilePath))
    {
        kmbZipArchive archive;
        if (archive.Init(fileStream))
        {
            //
            // TODO: If multiple disks found, let user select one.
            //

            // Find the first disk image file in the archive.
            for (unsigned i = 0; i < archive.GetNumFiles(); i++)
            {
                string diskImageFileName = archive.GetFileName(i);
                if (StringEndsWith(diskImageFileName, ".dsk"))
                {
                    // Disk image found.
                    SetDiskFromArchive(driveNumber, archive, archiveFilePath, i, diskImageFileName);
                    break;
                }
            }
        }
        else
        {
            KMASSERTM(false, ("Invalid ZIP archive file: '%0'", archiveFilePath.c_str()));
        }
    }
}

void Application::SetDiskFromArchive(unsigned driveNumber, kmbZipArchive& archive, const std::string& archiveFilePath, unsigned diskImageFileIndex, const std::string& diskImageFileName)
{
    // Extract the compressed file.
    unsigned size = archive.GetUncompressedFileSize(diskImageFileIndex);
    char* uncompressedData = new char[size];
    archive.ExtractFileToMemory(diskImageFileIndex, uncompressedData, size);
    // Insert the disk into the emulated machine.
    kmbMemoryInputStream diskDataStream;
    diskDataStream.Init(uncompressedData, size);
    InsertDisk(driveNumber, diskDataStream, diskImageFileName, archiveFilePath);
    // Clean up.
    delete[] uncompressedData;
}

void Application::SetDisk(unsigned driveNumber, const std::string& diskImageFileName, const std::string& archiveFilePath)
{
    if (archiveFilePath.empty())
    {
        // Plain disk image file.
        SetDiskFromFile(driveNumber, diskImageFileName);
    }
    else
    {
        // Compressed disk image file inside an archive.
        // Open the ZIP file.
        kmbFileInputStream fileStream;
        if (fileStream.Init(archiveFilePath))
        {
            kmbZipArchive archive;
            if (archive.Init(fileStream))
            {
                // Find the disk image file index.
                unsigned fileIndex = archive.FindFileByName(diskImageFileName);
                if (fileIndex != kmbArchive::InvalidFileIndex)
                {
                    SetDiskFromArchive(driveNumber, archive, archiveFilePath, fileIndex, diskImageFileName);
                }
                else
                {
                    KMASSERTM(false, ("Compressed disk image file '%0' not found in archive '%1'.", diskImageFileName.c_str(), archiveFilePath.c_str()));
                }
            }
            else
            {
                KMASSERTM(false, ("Invalid ZIP archive file: '%0'", archiveFilePath.c_str()));
            }
        }
    }
}

void Application::LoadQuickSnapshot()
{
    char directoryFullPath[MAX_PATH];
    GetFullPathName(QuickSnapshotDirectory.c_str(), sizeof(directoryFullPath), directoryFullPath, nullptr);
    string fileFullPath = string(directoryFullPath) + '\\' + QuickSnapshotFile;

    LoadSnapshot(fileFullPath);
}

void Application::SaveQuickSnapshot()
{
    char directoryFullPath[MAX_PATH];
    GetFullPathName(QuickSnapshotDirectory.c_str(), sizeof(directoryFullPath), directoryFullPath, nullptr);
    // Create the directory if necessary.
    CreateDirectory(directoryFullPath, nullptr);
    string fileFullPath = string(directoryFullPath) + '\\' + QuickSnapshotFile;

    SaveSnapshot(fileFullPath);
}

void Application::LoadSnapshotWithFileDialog()
{
    string fullFilePath;
    if (ShowLoadSaveFileDialog(true/*isLoad*/, "\\Snapshots", "SNA Snapshots (*.sna)\0*.sna\0\0", &fullFilePath))
    {
        // Load the selected snapshot.
        LoadSnapshot(fullFilePath);
    }
}

void Application::SaveSnapshotWithFileDialog()
{
    string fullFilePath;
    if (ShowLoadSaveFileDialog(false/*isLoad*/, "\\Snapshots", "SNA Snapshots (*.sna)\0*.sna\0\0", &fullFilePath))
    {
        // Save the selected snapshot.
        SaveSnapshot(fullFilePath);
    }
}

void Application::LoadSnapshot(string fullFilePath)
{
    // Open the file.
    kmbFileInputStream stream;
    if (stream.Init(fullFilePath))
    {
        // Load the snapshot.
        CPC::Snapshot snapshot;
        if (SnaSnapshotReadWrite::LoadSnapshot(stream, &snapshot))
        {
            // Create the new machine.
            string machineSpecsName;
            switch (snapshot.GetCpcType())
            {
                case CPC::Snapshot::CpcType::Cpc464: machineSpecsName = StandardCpc464SpecificationsName; break;
                case CPC::Snapshot::CpcType::Cpc664: machineSpecsName = StandardCpc664SpecificationsName; break;
                case CPC::Snapshot::CpcType::Cpc6128: machineSpecsName = StandardCpc6128SpecificationsName; break;
                default: machineSpecsName.clear(); break;
            }

            if (!machineSpecsName.empty())        // If the emulator supports this machine...
            {
                const CPC::MachineSpecifications* machineSpecs = FindMachineSpecificationsByName(machineSpecsName);
                GetSettings()->SetMachineSpecificationName(machineSpecsName);
                // TODO: Insert the disk images from the snapshot?
                CreateMachine();

                // Apply the snapshot.
                m_pMachine->ApplySnapshot(snapshot);
            }
        }
    }
}

void Application::SaveSnapshot(string fullFilePath)
{
    // Take a snapshot of the machine.
    CPC::Snapshot snapshot;
    m_pMachine->TakeSnapshot(&snapshot);

    // Save the snapshot to file.
    kmbFileOutputStream stream;
    if (stream.Init(fullFilePath))
    {
        SnaSnapshotReadWrite::SaveSnapshot(stream, snapshot);
    }
}

bool Application::ShowLoadSaveFileDialog(bool isLoad, string relativeInitialDir, const char* filter, string* outFullFilePath) const
{
    bool ret;

    // Show the File Dialog to let the user pick a file.
    char currentDir[2000];
    ::GetCurrentDirectory(sizeof(currentDir), currentDir);

    string initialDir;
    initialDir = currentDir + relativeInitialDir;

    char fileFullPath[2000];
    fileFullPath[0] = '\0';
    char fileName[2000];

    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(openFileName));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.hwndOwner = m_pAppWindow->GetHWnd();
    openFileName.lpstrFilter = filter;
    openFileName.lpstrFile = fileFullPath;
    openFileName.nMaxFile = sizeof(fileFullPath);
    openFileName.lpstrFileTitle = fileName;
    openFileName.nMaxFileTitle = sizeof(fileName);
    openFileName.lpstrInitialDir = initialDir.c_str();
    openFileName.Flags = (isLoad ? OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR :  // The flag OFN_NOCHANGEDIR is ignored on Windows XP and below.
                                   OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT);
    openFileName.FlagsEx = OFN_EX_NOPLACESBAR;

    BOOL result;
    if (isLoad)
    {
        result = ::GetOpenFileName(&openFileName);
    }
    else
    {
        result = ::GetSaveFileName(&openFileName);
    }

    ::SetCurrentDirectory(currentDir);          // Restore the working directory, changed by the Open File Dialog

    ret = (result != FALSE);
    if (ret)
    {
        outFullFilePath->assign(fileFullPath);
    }
    else
    {
        outFullFilePath->clear();
    }

    return ret;
}

bool Application::StringEndsWith(string str, string ending)
{
    bool ret = false;
    const size_t strLength = str.length();
    const size_t endingLength = ending.length();
    if (strLength >= endingLength)
    {
        ret = true;
        for (size_t i = 0; i < endingLength; i++)
        {
            if (std::tolower(str.at(strLength - endingLength + i)) != std::tolower(ending.at(i)))
            {
                ret = false;
                break;
            }
        }
    }

    return ret;
}
