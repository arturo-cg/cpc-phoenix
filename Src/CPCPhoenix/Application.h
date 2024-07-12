//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "Misc/kmbSingleton.h"
#include "Settings.h"
#include "Timer/kmbPrecisionTimer.h"

class AppWindow;
class RenderingApi;
class WindowsKeyStateProvider;
class TextureVideoOutput;
class CWinSoundOutput;
class Debugger;
class ProgramAnalyzer;
class SoundAnalyzer;
class kmbZipArchive;

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

    static const string StandardCpc464SpecificationsName;
    static const string StandardCpc664SpecificationsName;
    static const string StandardCpc6128SpecificationsName;

                              Application() { m_bOk = false; }
    virtual                  ~Application() { End(); }

    bool                      Init(HINSTANCE hInstance);
    virtual void              End();
    bool                      IsOk() const { return m_bOk; }

    /** Requests to exit the application. The exit is carried at the end of the current frame. */
    void                      RequestExitApp() { m_bExitApp = true; }

    /** Finds the MachineSpecifications with the specified name. */
    const CPC::MachineSpecifications* FindMachineSpecificationsByName(string name) const;
    /** Gets the name of the MachineSpecifications that occupies the specified ordered position, or nullptr if the specified position is out of range. */
    const string* GetMachineSpecificationsNameAtPosition(unsigned position) const;
    /** Finds the ordered position of the specified MachineSpecifications' name. */
    unsigned                  FindMachineSpecificationsOrderedPosition(string name) const;
    /** Returns the application settings. Do not change settings directly, use specific methods like Application::ChangeCpcModelSetting instead. */
    Settings*                 GetSettings() { return &m_settings; }
    /** Returns the application settings (const version). */
    const Settings*           GetSettings() const { return &m_settings; }

    void                      ChangeMachineSpecificationName(string machineSpecificationName);
    void                      ChangeMonitorTypeSetting(CPC::CGateArray::ERgbConversionTableType eMonitorType);
    void                      ChangeDisplayScaleSetting(float scale);
    void                      ChangeDrawScanLinesSetting(bool bDrawScanLines);
    void                      ChangeEmulationSpeedSetting(float fEmulationSpeed);

    /** Returns the machine being emulated. */
    CPC::CMachine*            GetEmulatedMachine() { return m_pMachine; }
    /** Returns the machine being emulated (const version). */
    const CPC::CMachine*      GetEmulatedMachine() const { return m_pMachine; }

    /** Returns the application window. */
    AppWindow*                GetAppWindow() { return m_pAppWindow; }
    /** Returns the application window (const version). */
    const AppWindow*          GetAppWindow() const { return m_pAppWindow; }

    TextureVideoOutput*       GetTextureVideoOutput() { return m_videoOutput; }
    const TextureVideoOutput* GetTextureVideoOutput() const { return m_videoOutput; }

    CWinSoundOutput*          GetSoundOutput() { return m_pSoundOutput; }
    const CWinSoundOutput*    GetSoundOutput() const { return m_pSoundOutput; }

    /** Returns the debugger. */
    Debugger*                 GetDebugger() { return m_debugger; }
    /** Returns the debugger (const version). */
    const Debugger*           GetDebugger() const { return m_debugger; }

    /** Returns the program analyzer. */
    ProgramAnalyzer*          GetProgramAnalyzer() { return m_programAnalyzer; }
    /** Returns the program analyzer (const version). */
    const ProgramAnalyzer*    GetProgramAnalyzer() const { return m_programAnalyzer; }

    /** Returns the RenderingApi object. */
    RenderingApi*             GetRenderingApi() { return m_renderingApi; }
    /** Returns the RenderingApi object (const version). */
    const RenderingApi*       GetRenderingApi() const { return m_renderingApi; }


    /** Runs the application. */
    void                      MainLoop();

    // Notifications from the application window.
    bool                      _OnAppWindowKeyDown(unsigned virtualKey, bool shift, bool ctrl, bool alt);     // Returns true if it processed the key, or false otherwise.
    void                      _OnAppWindowMoved();
    void                      _OnAppWindowSizing();
    void                      _OnAppWindowSizeChanged();
    void                      _OnAppWindowCloseRequest(AppWindow* pAppWindow);


private:

    // 64 us per scan line * 312 scan lines = 19968 us per frame (50.08 frames/s)
    static constexpr double   FRAME_DURATION_USECS = 19968.0;
    static constexpr double   FRAME_DURATION_SECS = FRAME_DURATION_USECS / 1000000.0;
    // Windows timer resolution to improve accuracy of Sleep().
    static constexpr UINT     WINDOWS_TIMER_RESOLUTION = 1;
    // How often Windows messages are processed.
    static constexpr double   PROCESS_WINDOWS_MESSAGES_PERIOD = 1.0 / 50.0;
    // How often a new frame is rendered.
    static constexpr double   RENDER_PERIOD = 1.0 / 60.0;       // TODO - Dynamically calculate this using the host monitor's refresh rate.
    static constexpr double   RENDER_PERIOD_WHILE_DEBUGGER_RUNNING = 1.0 / 20.0;

    using StringToMachineSpecificationsMap = map<string, CPC::MachineSpecifications>;
    using StringList = vector<string>;


    void                      ResetVars();
    void                      FreeVars();

    void                      InitializeMachineSpecifications();
    void                      CreateMachine();
    void                      DestroyMachine();
    void                      InitializeGui();
    void                      ShutdownGui();

    double                    ComputeElapsedRealTime();
    void                      RunMachine(double elapsedRealTime);
    void                      ProcessWindowsMessagesIfNecessary(double elapsedRealTime);
    void                      ProcessWindowsMessages();
    void                      RenderIfNecessary(double elapsedRealTime);
    void                      Render();
    void                      SleepIfIdle();

    float                     CalculateStatusBarHeight();
    void                      DrawGui();
    void                      DrawMainWindowGui();
    void                      DrawMainMenuGui();
    void                      DrawDiskDriveMenuGui(int driveNumber);
    void                      DrawTapeDeckMenuGui();
    void                      DrawStatusBarGui();
    void                      DrawDiskDriveBarGui(char driveLetter, int driveNumber);
    void                      DrawTapeDeckBarGui();

    void                      InsertDisk(unsigned driveNumber, kmbInputStream& diskImageStream, const std::string& diskImageFileName, const std::string& archiveFilePath);
    void                      EjectDisk(unsigned driveNumber);
    void                      SetDiskFromFile(unsigned driveNumber, const std::string& diskImageFilePath);
    void                      SetDiskFromArchive(unsigned driveNumber, const std::string& archiveFilePath);
    void                      SetDiskFromArchive(unsigned driveNumber, kmbZipArchive& archive, const std::string& archiveFilePath, unsigned diskImageFileIndex, const std::string& diskImageFileName);
    void                      SetDisk(unsigned driveNumber, const std::string& diskImageFileName, const std::string& archiveFilePath);

    void                      InsertTape(kmbInputStream& tapeImageStream, const std::string& tapeImageFileName, const std::string& archiveFilePath);
    void                      EjectTape();
    void                      SetTapeFromFile(const std::string& tapeImageFilePath);
    void                      SetTapeFromArchive(const std::string& archiveFilePath);
    void                      SetTapeFromArchive(kmbZipArchive& archive, const std::string& archiveFilePath, unsigned tapeImageFileIndex, const std::string& tapeImageFileName);
    void                      SetTape(const std::string& tapeImageFileName, const std::string& archiveFilePath);

    void                      LoadQuickSnapshot();
    void                      SaveQuickSnapshot();
    void                      LoadSnapshotWithFileDialog();
    void                      SaveSnapshotWithFileDialog();
    void                      LoadSnapshot(string fullFilePath);
    void                      SaveSnapshot(string fullFilePath);

    bool                      ShowLoadSaveFileDialog(bool isLoad, string relativeInitialDir, const char* filter, string* outFullFilePath) const;

    static bool               StringEndsWith(string str, string ending);

    bool                      m_bOk;

    bool                      m_bExitApp;               // True when a request to exit the application has been made.
    StringToMachineSpecificationsMap m_machineSpecifications;
    StringList                m_orderedMachineSpecificationsNames;
    Settings                  m_settings;
    AppWindow*                m_pAppWindow;
    RenderingApi*             m_renderingApi;
    Debugger*                 m_debugger;
    ProgramAnalyzer*          m_programAnalyzer;
    SoundAnalyzer*            m_soundAnalyzer;

    CPC::CMachine*            m_pMachine;
    WindowsKeyStateProvider*  m_pKeyStateProvider;
    TextureVideoOutput*       m_videoOutput;
    CWinSoundOutput*          m_pSoundOutput;

    kmbPrecisionTimer         m_precisionTimer;
    kmbPrecisionTimer::Value  m_previousPrecisionTimerValue;
    double                    m_emulationTime;                         // To control when to advance the emulation.
    double                    m_renderTime;                            // To control when to render a new frame.
    double                    m_windowsMessagesTime;                   // To control when to process Windows messages.
    double                    m_speedRealTime;                         // Used to keep track of how fast the emulation is running.
    double                    m_speedEmulatedTime;                     // Used to keep track of how fast the emulation is running.
    float                     m_measuredEmulationSpeed;

    bool                      m_showDearImGuiDemoWindow;
};

#endif // _APPLICATION_H_
