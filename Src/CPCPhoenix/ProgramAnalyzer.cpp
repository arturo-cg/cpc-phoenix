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

    if (m_collectCodeSegments)
    {
        // Add the memory addresses taken by the instruction as a code segment.
        CPC::CCpu::AssemblyInstruction instruction;
        cpu->DisassembleInstruction(cpu->GetRegisters().PC.w, &instruction);     // TODO - No need for a full disassembly, we just need the length of the instruction.
        ProgramAnnotations::AddressRange codeSegment;
        codeSegment.start = cpu->GetRegisters().PC.w;
        codeSegment.end = codeSegment.start + instruction.sizeBytes - 1;
        m_annotations->AddCodeSegment(codeSegment);
    }
}

void ProgramAnalyzer::DrawGui()
{
    bool keepOpen = IsVisible();
    if (ImGui::Begin("Program Analyzer", &keepOpen/*, ImGuiWindowFlags_AlwaysAutoResize*/))
    {
        // Enable/disable data collection.
        ImGui::Checkbox("Collect Code Segments", &m_collectCodeSegments);

        ImGui::Separator();
        ImGui::Text("%d code segments:", m_annotations->GetCodeSegments().size());

        ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit;
        if (ImGui::BeginTable("CodeSegments", 1/*columns_count*/, tableFlags/*, ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 8.f)*/))
        {
            for (ProgramAnnotations::AddressRangeList::const_iterator iter = m_annotations->GetCodeSegments().cbegin(); iter != m_annotations->GetCodeSegments().cend(); ++iter)
            {
                ImGui::TableNextRow();
                const ProgramAnnotations::AddressRange& addressRange = *iter;

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
