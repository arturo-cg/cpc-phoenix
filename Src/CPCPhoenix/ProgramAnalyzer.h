#pragma once

namespace CPC
{
    class CMachine;
    class CMemoryBlock;
}

class ProgramAnnotations;
class ProgramCode;
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

    // Indicates whether the Program Analyzer is collecting code from the program currently running or not.
    bool IsCodeCollectionEnabled() const { return m_codeCollectionEnabled; }
    // Enables or disables the collection of the code from the program currently running.
    void SetCodeCollectionEnabled(bool enabled);

    // Returns the program code.
    ProgramCode* GetProgramCode() { return m_programCode; }
    const ProgramCode* GetProgramCode() const { return m_programCode; }

    // Regenerates the code if dirty, i.e. new code was collected since the last code generation.
    bool RegenerateCodeIfNeeded();

    // Called before executing a new instruction.
    void Update();

private:

    void ResetVars();
    void FreeVars();

    void CollectAddressesOfInstructionAt(cpcWord address);

    void GenerateProgramCode();
    void GenerateSegmentCode(const AddressRange& codeSegment, const CPC::CMemoryBlock* memoryBlocks[4]);

    static std::string& AssignStringFormat(std::string* str, const char* format, ...);
    static void AppendStringFormat(std::string* str, const char* format, ...);

    bool m_bOk;
    bool m_visible;
    CPC::CMachine* m_machine;
    ProgramAnnotations* m_annotations;
    bool m_codeCollectionEnabled;
    ProgramCode* m_programCode;
    bool m_programCodeIsDirty;
};
