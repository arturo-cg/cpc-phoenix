//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

namespace CPC
{
    class CMachine;
    class CCpu;
}

/**
**
*/
class Debugger
{
public:

    Debugger() { m_bOk = false; }
    virtual ~Debugger() { End(); }

    bool Init();
    virtual void End();
    bool IsOk() const { return m_bOk; }

    // Start or stop the debugger.
    void SetActive(bool active);
    // Whether the emulated machine is being debugged or not.
    bool IsActive() const { return m_active; }
    // Whether the debugger is running (i.e. Run To command) or not.
    bool IsRunning() const { return m_running; }

    void RunMachine();

    void DrawGui();
    void DrawVideoOutputOverlays();

    // Notifications from the application window.
    bool _OnAppWindowKeyDown(unsigned virtualKey, bool shift, bool ctrl, bool alt);     // Returns true if it processed the key, or false otherwise.

private:

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
    bool m_showMonitorOverlay;
};
