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
class CWinVideoOutput;
class CWinSoundOutput;

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
    void                      ChangeScaleSetting(float scale);
    void                      ChangeDrawScanLinesSetting(bool bDrawScanLines);
    void                      ChangeEmulationSpeedSetting(float fEmulationSpeed);
    void                      SetDisk(unsigned nDrive, const std::string& sDiskImageFileName);

    /** Returns the machine being emulated. */
    CPC::CMachine*            GetEmulatedMachine() { return m_pMachine; }
    /** Returns the machine being emulated (const version). */
    const CPC::CMachine*      GetEmulatedMachine() const { return m_pMachine; }

    /** Returns the CWinVideoOutput currently in use. */
    CWinVideoOutput*          GetWinVideoOutput() { return m_pVideoOutput; }

    /** Returns the application window. */
    AppWindow*                GetAppWindow() { return m_pAppWindow; }

    /** Returns the RenderingApi object. */
    RenderingApi*             GetRenderingApi() { return m_renderingApi; }
    /** Returns the RenderingApi object (const version. */
    const RenderingApi*       GetRenderingApi() const { return m_renderingApi; }


    /** Runs the application. */
    void                      Run();

    // Notifications from the application window
    void                      _OnAppWindowCloseRequest(AppWindow* pAppWindow);


private:

    // 64 us per scan line * 312 scan lines = 19968 us per frame (50.08 frames/s)
    static constexpr double   FRAME_DURATION_USECS = 19968.0;

    using StringToMachineSpecificationsMap = map<string, CPC::MachineSpecifications>;
    using StringList = vector<string>;


    void                      ResetVars();
    void                      FreeVars();

    void                      InitializeMachineSpecifications();
    void                      CreateMachine();
    void                      DestroyMachine();
    void                      InitializeGui();
    void                      ShutdownGui();

    void                      ProcessWindowsMessages();
    void                      Render();

    bool                      m_bOk;

    bool                      m_bExitApp;               // True when a request to exit the application has been made.
    StringToMachineSpecificationsMap m_machineSpecifications;
    StringList                m_orderedMachineSpecificationsNames;
    Settings                  m_settings;
    AppWindow*                m_pAppWindow;
    RenderingApi*             m_renderingApi;

    CPC::CMachine*            m_pMachine;
    unsigned                  m_uFrameCount;
    WindowsKeyStateProvider*  m_pKeyStateProvider;
    CWinVideoOutput*          m_pVideoOutput;
    CWinSoundOutput*          m_pSoundOutput;

    kmbPrecisionTimer         m_executionTimer;
    kmbPrecisionTimer::Value  m_previousTimerValue;
    kmbPrecisionTimer::Value  m_currentTimerValue;

};

#endif // _APPLICATION_H_
