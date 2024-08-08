#pragma once

namespace CPC
{
    class CMachine;
    class CMemoryBlock;
}

class ProgramAnnotations;
struct AddressRange;

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

    // Shows or hides the Program Analyzer UI.
    void SetVisible(bool visible) { m_visible = visible; }
    // Whether the Program Analyzer UI is visible or not.
    bool IsVisible() const { return m_visible; }

    // Indicates whether the Program Analyzer is collecting data or not.
    bool IsActive() const { return m_collectCodeSegments; }

    // Called before executing a new instruction.
    void Update();

    void DrawGui();

private:

    void ResetVars();
    void FreeVars();

    void WriteProgramCode(std::string* outputCode) const;
    void WriteSegmentCode(const AddressRange& codeSegment, const CPC::CMemoryBlock* memoryBlocks[4], std::string* outputCode) const;

    static void AppendStringFormat(std::string* str, const char* format, ...);

    bool m_bOk;
    bool m_visible;
    CPC::CMachine* m_machine;
    ProgramAnnotations* m_annotations;
    bool m_collectCodeSegments;
    std::string m_programCode;
    bool m_programCodeNeedsRewrite;
    bool m_programCodeRewriteEnabled;   // TODO: Hack to prevent program code rewrite from slowing down the emulation. To be removed.
};
