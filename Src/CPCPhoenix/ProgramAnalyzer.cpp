#include "stdafx.h"
#include "ProgramAnalyzer.h"
#include "Application.h"
#include "cpcMachine.h"
#include "cpcMemoryBlock.h"
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
    CPC::CGateArray* gateArray = m_machine->GetGateArray();

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
        // Collect it only if it is code stored in RAM. Ignore code stored in ROM.
        // In the future, it would be a nice feature to also consider code stored in ROM.
        cpcWord address = cpu->GetRegisters().PC.w;
        unsigned block = (address >> 14);   // The two most significant bits indicate the 16 Kb memory block.
        bool isRam = ((block == 0) && !gateArray->IsLowerRomVisible()) ||
            (block == 1) ||
            (block == 2) ||
            ((block == 3 && !gateArray->IsUpperRomVisible()));

        if (isRam)
        {
            // Add the memory addresses taken by the instruction as a code segment.
            CPC::CCpu::AssemblyInstruction instruction;
            cpu->DisassembleInstruction(address, &instruction);     // TODO - No need for a full disassembly, we just need the length of the instruction.
            AddressRange codeSegment;
            codeSegment.start = address;
            codeSegment.end = codeSegment.start + instruction.sizeBytes - 1;
            if (m_annotations->AddCodeSegment(codeSegment))
            {
                m_programCodeNeedsRewrite = true;
            }
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

    // For now, always read instructions from the first 64 Kb of RAM.
    const CPC::CMemoryBlock* memoryBlocks[4];
    CPC::CMemory* memory = m_machine->GetMemory();
    memoryBlocks[0] = memory->GetRamBlock(0);
    memoryBlocks[1] = memory->GetRamBlock(1);
    memoryBlocks[2] = memory->GetRamBlock(2);
    memoryBlocks[3] = memory->GetRamBlock(3);

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

        WriteSegmentCode(codeSegment, memoryBlocks, outputCode);
    }
}

void ProgramAnalyzer::WriteSegmentCode(const AddressRange& codeSegment, const CPC::CMemoryBlock* memoryBlocks[4], std::string* outputCode) const
{
    // Header and ORG directive.
    AppendStringFormat(outputCode, "; ======= #%04X - #%04X =======\n", codeSegment.start, codeSegment.end);
    AppendStringFormat(outputCode, "ORG #%04X\n\n", codeSegment.start);

    // Read bytes from the specified memory blocks.
    std::function<cpcByte(cpcWord)> readByteFromBlocks = [memoryBlocks](cpcWord address)
        {
            // Bits 15,14 of nAddress determine which one of the four given blocks to use
            // Bits 13-0 of nAddress determine the offset into the selected block
            int blockIndex = address >> 14;
            cpcWord offset = address & 0x3FFF;
            return memoryBlocks[blockIndex]->ReadByte(offset);
        };

    // Instructions.
    CPC::CCpu* cpu = m_machine->GetCpu();
    CPC::CCpu::AssemblyInstruction instruction;
    cpcWord address = codeSegment.start;
    cpcWord previousAddress = address;
    while ((address <= codeSegment.end) &&      // If segment end not reached yet...
           (address >= previousAddress))        // If address didn't wrapped around...
    {
        // Disassemble current instruction.
        cpu->DisassembleInstruction(address, readByteFromBlocks, &instruction);
        // Write instruction.
        AppendStringFormat(outputCode, "%s %s\n", instruction.operation.c_str(), instruction.operands.c_str());
        // Next instruction.
        previousAddress = address;
        address += instruction.sizeBytes;
    }

    KMASSERT((address - 1) == codeSegment.end);
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
