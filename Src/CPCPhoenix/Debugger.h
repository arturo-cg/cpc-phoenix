//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

#include "cpcFdcListener.h"

namespace CPC
{
    class CMachine;
    class CCpu;
    class CFdc;
}

/**
**
*/
class Debugger : CPC::IFdcListener
{
public:

    Debugger() { m_bOk = false; }
    virtual ~Debugger() { End(); }

    bool Init();
    virtual void End();
    bool IsOk() const { return m_bOk; }

    // Called by the application when the emulated machine changes, for example when the emulator is started or when the user changes the CPC model.
    void SetMachine(CPC::CMachine* newMachine);

    // Start or stop the debugger.
    void SetActive(bool active);
    // Whether the emulated machine is being debugged or not.
    bool IsActive() const { return m_active; }
    // Whether the debugger is running (i.e. Run To command) or not.
    bool IsRunning() const { return m_running; }

    void RunMachine();

    // From IFdcListener.
    virtual void OnFdcCommandReceived(const CPC::CFdc* fdc) override;
    virtual void OnFdcCommandFinished(const CPC::CFdc* fdc) override;

    void DrawGui();
    void DrawVideoOutputOverlays();

    // Notifications from the application window.
    bool _OnAppWindowKeyDown(unsigned virtualKey, bool shift, bool ctrl, bool alt);     // Returns true if it processed the key, or false otherwise.

private:

    static const unsigned INVALID_DRIVE_NUMBER = 0xFFFFFFFF;
    static const unsigned MAX_FDC_OPERATION_COUNT = 80;

    struct FdcOperation
    {
        int/*CPC::CFdc::ECommand*/ command;     // Stored as an int to avoid including cpcFdc.h here.
        string parameters;
        string result;
    };

    using FdcOperationDeque = std::deque<FdcOperation>;

    void ResetVars();
    void FreeVars();

    // It executes the instruction at PC, or finishes executing the current instruction if the CPU is still in the middle of one.
    // In both cases, PC will point at the start of the next instruction when this method finishes.
    void ExecuteCurrentInstruction();
    void RunSingleCycle();

    void DrawCpu();
    void DrawExecuteOptions();
    void DrawDisassembly();
    void DrawCpuRegisters();
    void DrawStack();

    void DrawSystem();
    void DrawCrtc();
    void DrawGateArray();
    void DrawMonitor();
    void DrawFdc();
    void DrawDiskDrive(unsigned driveNumber, const char* imguiChildName, float contentRegionAvailProportion);

    void ShowDiskStructure(unsigned drive, unsigned side, unsigned track);
    void HideDiskStructure();
    void DrawDiskStructure();

    void DrawUnsignedByte(const char* label, cpcByte byte, bool verticalLayout = false, const char* tooltip = nullptr, ...);
    void DrawUnsignedWord(const char* label, cpcWord word, bool verticalLayout = false, const char* tooltip = nullptr, ...);
    void DrawInt(const char* label, int n);
    void LastItemBox(float margin);

    void RequestScrollToAddress(cpcWord address);

    bool m_bOk;
    bool m_active;
    CPC::CMachine* m_machine;
    bool m_running;
    bool m_stopAtBreakpoint;
    cpcWord m_breakpointAddress;
    bool m_stopAtInterrupt;
    bool m_stopAtHSync;
    bool m_stopAtVSync;
    bool m_scrollToAddressRequested;
    cpcWord m_scrollToAddress;
    bool m_showMonitorBeam;

    FdcOperationDeque m_fdcOperations;
    unsigned m_fdcOperationCounter;

    // Disk Structure window.
    unsigned m_diskDrive;
    unsigned m_diskSide;
    unsigned m_diskTrack;
};
