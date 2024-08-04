#include "stdafx.h"
#include "ProgramAnalyzer.h"
#include "Application.h"
#include "cpcMachine.h"
#include "ProgramAnnotations.h"

bool ProgramAnalyzer::Init()
{
    bool bRet = true;

    End();
    ResetVars();

    // Check parameters
    if (bRet)
    {
        //...
    }

    // Initialize class members
    if (bRet)
    {
        m_annotations = new ProgramAnnotations();
        m_annotations->Init();
    }


    if (bRet)
    {
        m_bOk = true;
    }
    else
    {
        FreeVars();
    }

    return bRet;
}

/*virtual*/ void ProgramAnalyzer::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

void ProgramAnalyzer::ResetVars()
{
    m_visible = false;
    m_machine = nullptr;
    m_annotations = nullptr;
    m_collectCodeSegments = false;
    m_programCode.clear();
    m_programCodeNeedsRewrite = false;
    m_programCodeRewriteEnabled = true;
}

void ProgramAnalyzer::FreeVars()
{
    delete m_annotations;
    m_annotations = nullptr;
}

void ProgramAnalyzer::Update()
{
    CPC::CCpu* cpu = m_machine->GetCpu();

    // This function should only be called in-between instructions, never in the middle of an instruction.
    KMASSERT(!cpu->IsExecutingInstruction());
    // Call ProgramAnalyzer::Update only if it is active.
    KMASSERT(IsActive());

    //
    // TODO: To prevent the Program Analyzer from slowing down the emulation, have the main thread (the emulator) simply store
    // the new address in a list, and a worker thread (the Program Analyzer) consume these addresses and do the actual work of decoding
    // and analyzing the instruction, updating the program code and so on.
    //

    if (m_collectCodeSegments)
    {
        // Add the memory addresses taken by the instruction as a code segment.
        CPC::CCpu::AssemblyInstruction instruction;
        cpu->DisassembleInstruction(cpu->GetRegisters().PC.w, &instruction);     // TODO - No need for a full disassembly, we just need the length of the instruction.
        AddressRange codeSegment;
        codeSegment.start = cpu->GetRegisters().PC.w;
        codeSegment.end = codeSegment.start + instruction.sizeBytes - 1;
        if (m_annotations->AddCodeSegment(codeSegment))
        {
            m_programCodeNeedsRewrite = true;
        }
    }

    if (m_programCodeNeedsRewrite && m_programCodeRewriteEnabled)
    {
        WriteProgramCode(&m_programCode);
        m_programCodeNeedsRewrite = false;
    }
}

void ProgramAnalyzer::WriteProgramCode(std::string* outputCode) const
{
    KMASSERT(outputCode != nullptr);

    outputCode->clear();

    // Iterate over the known code segments and generate the code for each one of them.
    bool isFirstSegment = true;
    for (const AddressRange& codeSegment : m_annotations->GetCodeSegments())
    {
        if (isFirstSegment)
        {
            isFirstSegment = false;
        }
        else
        {
            // Blank line.
            outputCode->append("\n");
        }

        WriteSegmentCode(codeSegment, outputCode);
    }
}

void ProgramAnalyzer::WriteSegmentCode(const AddressRange& codeSegment, std::string* outputCode) const
{
    AppendStringFormat(outputCode, "; ======= Memory address range #%04X - #%04X =======\n", codeSegment.start, codeSegment.end);
    AppendStringFormat(outputCode, "ORG #%04X\n", codeSegment.start);
}

void ProgramAnalyzer::DrawGui()
{
    bool keepOpen = IsVisible();
    if (ImGui::Begin("Program Analyzer", &keepOpen/*, ImGuiWindowFlags_AlwaysAutoResize*/))
    {
        // Enable/disable data collection.
        ImGui::Checkbox("Collect Code", &m_collectCodeSegments);

        // Program code.
        ImGui::Checkbox("Hack - Enable program code update", &m_programCodeRewriteEnabled);
        ImGui::Text(m_programCode.c_str());

        // Code segments.
        // TODO: For debugging only, to be removed at some point.
        ImGui::Separator();
        ImGui::Text("%d code segments:", m_annotations->GetCodeSegments().size());

        ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit;
        if (ImGui::BeginTable("CodeSegments", 1/*columns_count*/, tableFlags/*, ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 8.f)*/))
        {
            for (AddressRangeList::const_iterator iter = m_annotations->GetCodeSegments().cbegin(); iter != m_annotations->GetCodeSegments().cend(); ++iter)
            {
                ImGui::TableNextRow();
                const AddressRange& addressRange = *iter;

                ImGui::TableNextColumn();
                ImGui::Text("#%04X - #%04X", addressRange.start, addressRange.end);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();

    if (!keepOpen)
    {
        SetVisible(false);
    }
}

void ProgramAnalyzer::AppendStringFormat(std::string* str, const char* format, ...)
{
    va_list argList;
    va_start(argList, format);

    // Format the string and store the result in 'buffer'.
    static char buffer[1000];
    vsprintf_s(buffer, sizeof(buffer), format, argList);
    // Append to 'str'.
    str->append(buffer);

    va_end(argList);
}
