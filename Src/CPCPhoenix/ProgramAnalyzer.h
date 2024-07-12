#pragma once

namespace CPC
{
    class CMachine;
}

/**
** Set of tools that help the user analyze and understand (aka reverse-engineer) the program that is running in the emulated machine.
*/
class ProgramAnalyzer
{
public:

    ProgramAnalyzer() { m_bOk = false; }
    virtual ~ProgramAnalyzer() { End(); }

    bool Init();
    virtual void End();
    bool IsOk() const { return m_bOk; }

    // Called by the application when the emulated machine changes, for example when the emulator is started or when the user changes the CPC model.
    void SetMachine(CPC::CMachine* newMachine) { m_machine = newMachine; }

    // Start or stop the debugger.
    void SetActive(bool active);
    // Whether the emulated machine is being debugged or not.
    bool IsActive() const { return m_active; }

    // Called before executing a new instruction.
    void Update();

    void DrawGui();

private:

    void ResetVars();
    void FreeVars();

    bool m_bOk;
    bool m_active;
    CPC::CMachine* m_machine;
};
