//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <sstream>
#include "Debugger.h"
#include "Application.h"
#include "TextureVideoOutput.h"
#include "cpcCpu.h"
#include "cpcDisk.h"
#include "cpcDiskDrive.h"
#include "cpcFdc.h"
#include "cpcMachine.h"
#include "cpcMemoryBlock.h"


bool Debugger::Init()
{
    bool bRet = true;

    End();
    ResetVars();

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

/*virtual*/ void Debugger::End()
{
    if (IsOk())
    {
        FreeVars();
        m_bOk = false;
    }
}

void Debugger::ResetVars()
{
    m_active = false;
    m_machine = nullptr;
    m_running = false;
    m_codeBreakpoints.clear();
    m_stopAtInterrupt = false;
    m_stopAtHSync = false;
    m_stopAtVSync = false;
    m_scrollToAddressRequested = false;
    m_scrollToAddress = 0x0000;
    m_showMonitorBeam = true;
    m_fdcOperations.clear();
    m_fdcOperationCounter = 0;
    m_diskDrive = INVALID_DRIVE_NUMBER;
    m_diskSide = 0;
    m_diskTrack = 0;
}

void Debugger::FreeVars()
{
    if (m_machine != nullptr)
    {
        m_machine->GetFdc()->SetListener(nullptr);
    }
}

void Debugger::SetMachine(CPC::CMachine* newMachine)
{
    m_machine = newMachine;

    if (newMachine != nullptr)
    {
        newMachine->GetFdc()->SetListener(this);
    }
}

void Debugger::SetActive(bool active)
{
    m_active = active;
    if (active)
    {
        // Finish current instruction.
        ExecuteCurrentInstruction();
        // Scroll to PC.
        RequestScrollToAddress(m_machine->GetCpu()->GetRegisters().PC.w);
    }
}

void Debugger::RunMachine()
{
    // If "Run To" is active, run the machine for a 4-MHz clock cycle.
    if (m_running)
    {
        CPC::CCpu* cpu = m_machine->GetCpu();
        CPC::CCrtc* crtc = m_machine->GetCrtc();
        // Get state before.
        bool previousHSync = crtc->GetHSyncState();
        bool previousVSync = crtc->GetVSyncState();
        // Run machine for a 4-Mhz clock cycle.
        m_machine->Run(1);
        // Get state after and check stop conditions.
        if ((!cpu->IsExecutingInstruction() && HasCodeBreakpointAtAddress(cpu->GetRegisters().PC.w)) ||
            (m_stopAtInterrupt && cpu->InterruptWasAcknowledged()) ||
            (m_stopAtHSync && !previousHSync && crtc->GetHSyncState()) ||
            (m_stopAtVSync && !previousVSync && crtc->GetVSyncState()))
        {
            // Stop condition met. Stop running.
            m_running = false;
            // Scroll to PC.
            RequestScrollToAddress(m_machine->GetCpu()->GetRegisters().PC.w);
        }
        // Update the screen.
        if (!m_running ||                                   // If we just stopped running...
            (!previousHSync && crtc->GetHSyncState()))      // If at start of HSYNC (to update the screen at the end of every scan line)...
        {
            Application::Singleton()->GetTextureVideoOutput()->CaptureVideoOutputMidFrame();
        }
    }
}

void Debugger::ExecuteCurrentInstruction()
{
    // Run the machine repeatedly, one 4-MHz cycle at a time, until the current instruction is completed.
    do
    {
        m_machine->Run(1);
    } while (m_machine->GetCpu()->IsExecutingInstruction());
    // Update the screen.
    Application::Singleton()->GetTextureVideoOutput()->CaptureVideoOutputMidFrame();
    // Scroll to PC.
    RequestScrollToAddress(m_machine->GetCpu()->GetRegisters().PC.w);
}

void Debugger::RunSingleCycle()
{
    // Run the machine for one 1-MHz clock cycle.
    m_machine->Run(4);
    // Update the screen.
    Application::Singleton()->GetTextureVideoOutput()->CaptureVideoOutputMidFrame();
    // Scroll to PC.
    RequestScrollToAddress(m_machine->GetCpu()->GetRegisters().PC.w);
}

bool Debugger::HasCodeBreakpointAtAddress(cpcWord address) const
{
    return (m_codeBreakpoints.count(address) >= 1);
}

void Debugger::SetCodeBreakpointAtAddress(cpcWord address, bool enabled)
{
    if (enabled)
    {
        m_codeBreakpoints.insert(address);
    }
    else
    {
        m_codeBreakpoints.erase(address);
    }
}

void Debugger::OnFdcCommandReceived(const CPC::CFdc* fdc)
{
    KMASSERT(fdc == m_machine->GetFdc());

    FdcOperation operation;
    operation.command = (int)fdc->GetCommand();
    fdc->GetCurrentParametersLog(&operation.parameters);
    operation.result.clear();

    m_fdcOperations.push_front(operation);
    m_fdcOperationCounter++;

    if (m_fdcOperations.size() > MAX_FDC_OPERATION_COUNT)
    {
        // Remove oldest.
        m_fdcOperations.pop_back();
    }
}

void Debugger::OnFdcCommandFinished(const CPC::CFdc* fdc)
{
    KMASSERT(fdc == m_machine->GetFdc());

    fdc->GetCurrentResultLog(&m_fdcOperations.front().result);
}

void Debugger::DrawGui()
{
    // Keep pointer to the machine up-to-date, it can change at any time.
    m_machine = Application::Singleton()->GetEmulatedMachine();

    ImGui::Begin("Debugger", nullptr/*, ImGuiWindowFlags_AlwaysAutoResize*/);
    // CPU.
    DrawCpu();
    // System (CRTC, Gate Array, PSG, etc.).
    DrawSystem();
    // Disk Structure.
    DrawDiskStructure();

    ImGui::End();
}

void Debugger::DrawVideoOutputOverlays()
{
    TextureVideoOutput* textureVideoOutput = Application::Singleton()->GetTextureVideoOutput();
    // Monitor beam position.
    if (m_showMonitorBeam)
    {
        // Beam in viewport space.
        float adjustedBeamX = float(textureVideoOutput->GetBeamX()) - float(TextureVideoOutput::VIEWPORT_LEFT);
        float adjustedBeamY = float(textureVideoOutput->GetBeamY()) - float(TextureVideoOutput::VIEWPORT_TOP);
        // Beam in UI space.
        float guiRectWidth = textureVideoOutput->GetGuiRectMax().x - textureVideoOutput->GetGuiRectMin().x;
        float guiRectHeight = textureVideoOutput->GetGuiRectMax().y - textureVideoOutput->GetGuiRectMin().y;
        float beamGuiPosX = textureVideoOutput->GetGuiRectMin().x + (adjustedBeamX / float(TextureVideoOutput::VIEWPORT_WIDTH - 1) * guiRectWidth);
        float beamGuiPosY = textureVideoOutput->GetGuiRectMin().y + (adjustedBeamY / float(TextureVideoOutput::VIEWPORT_HEIGHT - 1) * guiRectHeight);
        //float thicknessX = 1.f / float(TextureVideoOutput::VIEWPORT_WIDTH - 1) * guiRectWidth;
        //float thicknessY = 1.f / float(TextureVideoOutput::VIEWPORT_HEIGHT - 1) * guiRectHeight;

        ImGui::BeginChild(TextureVideoOutput::DisplayImGuiWindowName);  // Append to the Dear ImGui window that contains the display output.

        static constexpr float BeamAlpha = 0.4f;
        static constexpr float BeamThickness = 2.f;
        if ((adjustedBeamX >= 0.f) && (adjustedBeamX <= TextureVideoOutput::VIEWPORT_WIDTH - 1))        // If currently inside the visible area of the screen...
        {
            ImGui::GetWindowDrawList()->AddLine(ImVec2(beamGuiPosX, textureVideoOutput->GetGuiRectMin().y),
                                                ImVec2(beamGuiPosX, textureVideoOutput->GetGuiRectMax().y),
                                                ImColor(1.f, 1.f, 1.f, BeamAlpha),
                                                BeamThickness);
        }

        if ((adjustedBeamY >= 0.f) && (adjustedBeamY <= TextureVideoOutput::VIEWPORT_HEIGHT - 1))       // If currently inside the visible area of the screen...
        {
            ImGui::GetWindowDrawList()->AddLine(ImVec2(textureVideoOutput->GetGuiRectMin().x, beamGuiPosY),
                                                ImVec2(textureVideoOutput->GetGuiRectMax().x, beamGuiPosY),
                                                ImColor(1.f, 1.f, 1.f, BeamAlpha),
                                                BeamThickness);
        }

        ImGui::EndChild();
    }
}

void Debugger::DrawCpu()
{
    // Execute options.
    DrawExecuteOptions();
    // Disassembly.
    DrawDisassembly();
    // Registers.
    ImGui::SameLine();
    ImGui::BeginGroup();
    DrawCpuRegisters();
    ImGui::Spacing();
    DrawStack();
    ImGui::EndGroup();
}

void Debugger::DrawExecuteOptions()
{
    static ImVec2 buttonSize = ImVec2(150.f, 20.f);
    // Step.
    if (ImGui::Button("Step (F11)", buttonSize))
    {
        // Execute instruction at PC, or finish current one.
        ExecuteCurrentInstruction();
    }
    ImGui::SameLine();
    // Run To.
    string runToButtonLabel = (m_running ? "Stop Running (Shift+F11)" : "Run To (Shift+F11)");
    if (ImGui::Button(runToButtonLabel.c_str(), buttonSize))
    {
        m_running = !m_running;
    }
    ImGui::SameLine();
    // Advance Clock Cycle.
    if (ImGui::Button("Clock Cycle (Ctrl+F11)", buttonSize))
    {
        // Advance one cycle of a 1-MHz clock.
        RunSingleCycle();
    }

    // Stop conditions.
    if (ImGui::CollapsingHeader("Stop Conditions"))
    {
        ImGui::Checkbox("Interrupt", &m_stopAtInterrupt);
        ImGui::Checkbox("HSync", &m_stopAtHSync);
        ImGui::Checkbox("VSync", &m_stopAtVSync);

        if (ImGui::TreeNode("CodeBreakpoints", "Code Breakpoints (currently: %d)", m_codeBreakpoints.size()))
        {
            std::vector<cpcWord> sortedCodeBreakpoints(m_codeBreakpoints.begin(), m_codeBreakpoints.end());
            std::sort(sortedCodeBreakpoints.begin(), sortedCodeBreakpoints.end());

            ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit;
            if (ImGui::BeginTable("CodeBreakpoints", 2/*columns_count*/, tableFlags, ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 4.f)))
            {
                for (std::vector<cpcWord>::const_iterator iter = sortedCodeBreakpoints.cbegin(); iter != sortedCodeBreakpoints.cend(); ++iter)
                {
                    ImGui::TableNextRow();
                    cpcWord address = *iter;
                    ImGui::PushID(address);

                    ImGui::TableNextColumn();
                    ImGui::Text("#%04X", address);

                    ImGui::TableNextColumn();
                    if (ImGui::SmallButton("Show"))
                    {
                        RequestScrollToAddress(address);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        m_codeBreakpoints.erase(address);
                    }

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            if (ImGui::Button("Remove All"))
            {
                m_codeBreakpoints.clear();
            }

            ImGui::TreePop();
        }
    }
}

void Debugger::DrawDisassembly()
{
    const CPC::CCpu* cpu = m_machine->GetCpu();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("Disassembly", 4/*columns_count*/, tableFlags, ImVec2(340.f, 0.f)))
    {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None, 15.f);      // Breakpoint.
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None, 60.f);      // Address.
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None, 30.f);      // Operation.
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None, 250.f);     // Operands.

        ImGuiListClipper clipper;
        clipper.Begin(1 << 16/*items_count: 64 KB*/, ImGui::GetTextLineHeightWithSpacing()/*items_height*/);
        while (clipper.Step())
        {
            // Disassemble as many instructions as there are visible lines.
            int lineCount = clipper.DisplayEnd - clipper.DisplayStart;
            cpcWord address = (cpcWord)clipper.DisplayStart;
            std::vector<CPC::CCpu::AssemblyInstruction> instructions;
            instructions.resize(lineCount);
            for (int i = 0; i < lineCount; i++)
            {
                ImGui::TableNextRow();
                // Disassemble instruction.
                CPC::CCpu::AssemblyInstruction& instruction = instructions.at(i);
                cpu->DisassembleInstruction(address, &instruction);
                // Print instruction.
                ImGui::PushID(i);

                ImGui::TableNextColumn();
                bool hasBreakpoint = HasCodeBreakpointAtAddress(address);
                if (BreakpointToggleButton("CodeBreakpointToggle", address, &hasBreakpoint))
                {
                    SetCodeBreakpointAtAddress(address, hasBreakpoint);
                }

                if (cpu->GetRegisters().PC.w == address)
                {
                    ImVec2 rectMin = ImGui::GetItemRectMin();
                    ImVec2 rectMax = ImGui::GetItemRectMax();
                    DrawCurrentInstructionArrow(rectMin, rectMax);
                }

                ImGui::TableNextColumn();
                ImGui::Text("%04X", address);

                ImGui::TableNextColumn();
                ImGui::Text("%s", instruction.operation.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", instruction.operands.c_str());

                ImGui::PopID();

                address += instruction.sizeBytes;
            }
        }

        if (m_scrollToAddressRequested)
        {
            ImGui::SetScrollY(clipper.ItemsHeight * m_scrollToAddress);
            m_scrollToAddressRequested = false;
        }

        ImGui::EndTable();
    }
}

void Debugger::DrawCpuRegisters()
{
    const CPC::CCpu::Registers& registers = m_machine->GetCpu()->GetRegisters();

    ImGui::BeginGroup();
    ImGui::BeginGroup();
    DrawUnsignedWord("AF", registers.AF.w);
    DrawUnsignedWord("BC", registers.BC.w);
    DrawUnsignedWord("DE", registers.DE.w);
    DrawUnsignedWord("HL", registers.HL.w);
    ImGui::EndGroup(); 
    ImGui::SameLine();
    ImGui::BeginGroup();
    DrawUnsignedWord("AF'", registers.altAF.w);
    DrawUnsignedWord("BC'", registers.altBC.w);
    DrawUnsignedWord("DE'", registers.altDE.w);
    DrawUnsignedWord("HL'", registers.altHL.w);
    ImGui::EndGroup();
    ImGui::Separator();
    ImGui::BeginGroup();
    DrawUnsignedWord("IX", registers.IX.w);
    DrawUnsignedWord("IY", registers.IY.w);
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    DrawUnsignedWord("PC", registers.PC.w);
    DrawUnsignedWord("SP", registers.SP.w);
    ImGui::EndGroup();
    DrawUnsignedByte("R", registers.R());
    DrawInt("IM", registers.IM);
    ImGui::SameLine();
    DrawUnsignedByte("I", registers.I());
    bool flag;
    flag = registers.IFF1; ImGui::Checkbox("IFF1", const_cast<bool*>(&flag)); ImGui::SameLine();
    flag = registers.IFF2; ImGui::Checkbox("IFF2", const_cast<bool*>(&flag)); ImGui::SameLine();
    ImGui::EndGroup();
}

void Debugger::DrawStack()
{
    // Label.
    ImGui::Text("Stack");
    // Stack entries.
    ImGui::BeginGroup();
    const CPC::CCpu::Registers& registers = m_machine->GetCpu()->GetRegisters();
    const CPC::CGateArray* gateArray = m_machine->GetGateArray();
    cpcWord address = registers.SP.w;
    static constexpr int NumStackEntries = 20;
    for (int i = 0; i < NumStackEntries; i++)
    {
        ImGui::TextDisabled("%04hX", address);
        ImGui::SameLine();
        cpcWord content = gateArray->ReadByteFromMemory(address) |
                          (gateArray->ReadByteFromMemory(address + 1) << 8);
        ImGui::Text("%04hX", content);

        address += 2;
    }
    ImGui::EndGroup();
    LastItemBox(2.f);
}

void Debugger::DrawSystem()
{
    ImGui::Begin("System", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::CollapsingHeader("CRTC", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawCrtc();
    }

    if (ImGui::CollapsingHeader("Gate-Array", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawGateArray();
    }

    if (ImGui::CollapsingHeader("Monitor", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawMonitor();
    }

    if (ImGui::CollapsingHeader("PSG", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawPsg();
    }

    if (ImGui::CollapsingHeader("FDC", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawFdc();
    }

    ImGui::End();
}

void Debugger::DrawCrtc()
{
    const CPC::CCrtc* crtc = m_machine->GetCrtc();
    cpcByte byte;
    cpcWord word;
    cpcWord address;
    constexpr bool labelOverValue = true;

    // Registers.
    byte = crtc->GetRegisterValue(CPC::CCrtc::HORIZONTAL_TOTAL); DrawUnsignedByte("R0", byte, labelOverValue, "Horizontal Total = %d", byte + 1); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::HORIZONTAL_DISPLAYED); DrawUnsignedByte("R1", byte, labelOverValue, "Horizontal Displayed = %d", byte); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::HORIZONTAL_SYNC_POSITION); DrawUnsignedByte("R2", byte, labelOverValue, "Horizontal Sync Position = %d", byte); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::SYNC_WIDTHS); DrawUnsignedByte("R3", byte, labelOverValue, "HSYNC Width = %d\nVSYNC Width = %d", byte & 0x0F, (byte & 0xF0) >> 4); ImGui::SameLine();

    byte = crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_TOTAL); DrawUnsignedByte("R4", byte, labelOverValue, "Vertical Total = %d", byte + 1); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_TOTAL_ADJUST); DrawUnsignedByte("R5", byte, labelOverValue, "Vertical Total Adjust = %d", byte); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_DISPLAYED); DrawUnsignedByte("R6", byte, labelOverValue, "Vertical Displayed = %d", byte); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_SYNC_POSITION); DrawUnsignedByte("R7", byte, labelOverValue, "Vertical Sync Position = %d", byte); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::INTERLACE_MODE_AND_SKEW); DrawUnsignedByte("R8", byte, labelOverValue, "Interlace Mode and Skew"); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::MAXIMUM_SCAN_LINE_ADDRESS); DrawUnsignedByte("R9", byte, labelOverValue, "Scan Lines Per Character Row = %d", byte + 1); ImGui::SameLine();

    byte = crtc->GetRegisterValue(CPC::CCrtc::CURSOR_START_RASTER); DrawUnsignedByte("R10", byte, labelOverValue, "Cursor Start Raster", byte); ImGui::SameLine();
    byte = crtc->GetRegisterValue(CPC::CCrtc::CURSOR_END_RASTER); DrawUnsignedByte("R11", byte, labelOverValue, "Cursor End Raster", byte); ImGui::SameLine();
    word = (crtc->GetRegisterValue(CPC::CCrtc::START_ADDRESS_HIGH) << 8) | crtc->GetRegisterValue(CPC::CCrtc::START_ADDRESS_LOW);
    address = CPC::CGateArray::ConvertCrtcAddressToCpc(word, 0);
    DrawUnsignedWord("R12|R13", word, labelOverValue, "Start Address = %04hX", address); ImGui::SameLine();
    word = (crtc->GetRegisterValue(CPC::CCrtc::CURSOR_ADDRESS_HIGH) << 8) | crtc->GetRegisterValue(CPC::CCrtc::CURSOR_ADDRESS_LOW);
    address = CPC::CGateArray::ConvertCrtcAddressToCpc(word, 0);
    DrawUnsignedWord("R14|R15", word, labelOverValue, "Cursor Address = %04hX", address); ImGui::SameLine();
    word = (crtc->GetRegisterValue(CPC::CCrtc::LIGHTPEN_ADDRESS_HIGH) << 8) | crtc->GetRegisterValue(CPC::CCrtc::LIGHTPEN_ADDRESS_LOW);
    address = CPC::CGateArray::ConvertCrtcAddressToCpc(word, 0);
    DrawUnsignedWord("R16|R17", word, labelOverValue, "Lightpen Address = %04hX", address); /*ImGui::SameLine();*/
    // Internal counters.
    byte = crtc->GetCurrentHCharacter(); DrawUnsignedByte("HCC", byte, labelOverValue, "Horizontal Character Counter (C0) = %d", byte); ImGui::SameLine();
    byte = crtc->GetCurrentVCharacter(); DrawUnsignedByte("VCC", byte, labelOverValue, "Vertical Character Counter (C4) = %d", byte); ImGui::SameLine();
    byte = crtc->GetCurrentScanLine(); DrawUnsignedByte("VLC", byte, labelOverValue, "Vertical Scan Line Counter (C9) = %d", byte); ImGui::SameLine();
    // HSync and VSync states.
    bool flag;
    flag = crtc->GetHSyncState(); ImGui::Checkbox("HSync", const_cast<bool*>(&flag)); ImGui::SameLine();
    flag = crtc->GetVSyncState(); ImGui::Checkbox("VSync", const_cast<bool*>(&flag));
    ImGui::Separator();
    // Summary.
    constexpr float Spacing = 20.f;
    ImGui::Text("Total: %dx%d", int(crtc->GetRegisterValue(CPC::CCrtc::HORIZONTAL_TOTAL) + 1), int(crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_TOTAL) + 1));
    ImGui::SameLine(0.f, Spacing);
    ImGui::Text("Scan Lines: %d", (int(crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_TOTAL) + 1) * int(crtc->GetRegisterValue(CPC::CCrtc::MAXIMUM_SCAN_LINE_ADDRESS) + 1)) + int(crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_TOTAL_ADJUST)));

    ImGui::Text("Displayed: %dx%d", int(crtc->GetRegisterValue(CPC::CCrtc::HORIZONTAL_DISPLAYED)), int(crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_DISPLAYED)));
    ImGui::SameLine(0.f, Spacing);
    int val = int(crtc->GetRegisterValue(CPC::CCrtc::HORIZONTAL_SYNC_POSITION));
    ImGui::Text("HSync: [%d-%d]", val, val + int(crtc->GetRegisterValue(CPC::CCrtc::SYNC_WIDTHS) & 0x0F));
    ImGui::SameLine(0.f, Spacing);
    ImGui::Text("VSync: %d, for %d scan lines", int(crtc->GetRegisterValue(CPC::CCrtc::VERTICAL_SYNC_POSITION)), int((crtc->GetRegisterValue(CPC::CCrtc::SYNC_WIDTHS) & 0xF0) >> 4));
}

void Debugger::DrawGateArray()
{
    const CPC::CGateArray* gateArray = m_machine->GetGateArray();

    // Video.
    DrawInt("Screen Mode", (int)gateArray->GetScreenMode()); ImGui::SameLine();
    bool flag;
    flag = gateArray->IsHSyncActive(); ImGui::Checkbox("HSync", const_cast<bool*>(&flag)); ImGui::SameLine();
    flag = gateArray->IsVSyncActive(); ImGui::Checkbox("VSync", const_cast<bool*>(&flag));

    // TODO: Show pen and border colors.

    // Memory.
    if (ImGui::BeginTable("MemoryLayout", 3/*column count*/))
    {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Read", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Write", ImGuiTableColumnFlags_None);

        ImGui::TableHeadersRow();

        cpcWord address = 0;
        for (int row = 0; row < 4; row++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("#%.4X-#%.4X", address, address + 0x4000 - 1);
            ImGui::TableNextColumn();
            ImGui::Text(gateArray->GetVisibleReadMemoryBlock(row)->GetLabel().c_str());
            ImGui::TableNextColumn();
            ImGui::Text(gateArray->GetVisibleWriteMemoryBlock(row)->GetLabel().c_str());

            address += 0x4000;
        }
        ImGui::EndTable();
    }
}

void Debugger::DrawMonitor()
{
    const CPC::CVideoOutput* monitor = m_machine->GetVideoOutput();

    ImGui::Checkbox("Show Beam", &m_showMonitorBeam);
    ImGui::Text("Beam: %d,%d", monitor->GetBeamX(), monitor->GetBeamY());
}

void Debugger::DrawPsg()
{
    const CPC::CPsg* psg = m_machine->GetPsg();

    std::stringstream mixerInfo;
    mixerInfo << "Mixer\n\n";
    mixerInfo << "Tone enabled: ";
    for (int channel = 0; channel < 3; channel++)
    {
        mixerInfo << (psg->IsChannelToneEnabled(channel) ? char('A' + channel) : '-');
    }
    mixerInfo << "\n";
    mixerInfo << "Noise enabled: ";
    for (int channel = 0; channel < 3; channel++)
    {
        mixerInfo << (psg->IsChannelNoiseEnabled(channel) ? char('A' + channel) : '-');
    }

    DrawUnsignedWord("R0|R1", (psg->GetRegisterValue(CPC::CPsg::REG_A_TONE_PERIOD_HIGH) << 8) | psg->GetRegisterValue(CPC::CPsg::REG_A_TONE_PERIOD_LOW), true, "Channel A tone period = %d", psg->GetChannelTonePeriod(0)); ImGui::SameLine();
    DrawUnsignedWord("R2|R3", (psg->GetRegisterValue(CPC::CPsg::REG_B_TONE_PERIOD_HIGH) << 8) | psg->GetRegisterValue(CPC::CPsg::REG_B_TONE_PERIOD_LOW), true, "Channel B tone period = %d", psg->GetChannelTonePeriod(1)); ImGui::SameLine();
    DrawUnsignedWord("R4|R5", (psg->GetRegisterValue(CPC::CPsg::REG_C_TONE_PERIOD_HIGH) << 8) | psg->GetRegisterValue(CPC::CPsg::REG_C_TONE_PERIOD_LOW), true, "Channel C tone period = %d", psg->GetChannelTonePeriod(2)); ImGui::SameLine();
    DrawUnsignedByte("R6", psg->GetRegisterValue(CPC::CPsg::REG_NOISE_PERIOD), true, "Noise period = %d", psg->GetNoisePeriod()); ImGui::SameLine();
    DrawUnsignedByte("R7", psg->GetRegisterValue(CPC::CPsg::REG_MIXER), true, mixerInfo.str().c_str()); ImGui::SameLine();
    std::stringstream channelAmplitude;
    GetPsgChannelAmplitudeString(0, &channelAmplitude);
    DrawUnsignedByte("R8", psg->GetRegisterValue(CPC::CPsg::REG_A_AMPLITUDE), true, "Channel A amplitude = %s", channelAmplitude.str().c_str()); ImGui::SameLine();
    GetPsgChannelAmplitudeString(1, &channelAmplitude);
    DrawUnsignedByte("R9", psg->GetRegisterValue(CPC::CPsg::REG_B_AMPLITUDE), true, "Channel B amplitude = %s", channelAmplitude.str().c_str()); ImGui::SameLine();
    GetPsgChannelAmplitudeString(2, &channelAmplitude);
    DrawUnsignedByte("R10", psg->GetRegisterValue(CPC::CPsg::REG_C_AMPLITUDE), true, "Channel C amplitude = %s", channelAmplitude.str().c_str()); ImGui::SameLine();
    DrawUnsignedWord("R11|R12", (psg->GetRegisterValue(CPC::CPsg::REG_ENVELOPE_PERIOD_HIGH) << 8) | psg->GetRegisterValue(CPC::CPsg::REG_ENVELOPE_PERIOD_LOW), true, "Envelope period = %d", psg->GetEnvelopePeriod()); ImGui::SameLine();
    DrawUnsignedByte("R13", psg->GetRegisterValue(CPC::CPsg::REG_ENVELOPE_SHAPE), true, "Envelope shape"); /*ImGui::SameLine();*/
}

void Debugger::GetPsgChannelAmplitudeString(int channel, std::stringstream* amplitudeString) const
{
    amplitudeString->str("");
    amplitudeString->clear();

    const CPC::CPsg* psg = m_machine->GetPsg();
    if (psg->IsChannelAmplitudeControlledByEnvelope(channel))
    {
        *amplitudeString << "Envelope";
    }
    else
    {
        *amplitudeString << psg->GetChannelConstantAmplitude(channel);
    }
}

void Debugger::DrawFdc()
{
    // FDC.
    CPC::CFdc* fdc = m_machine->GetFdc();

    ImGui::Text("Phase: %s", CPC::CFdc::GetPhaseName(fdc->GetPhase()));
    if (ImGui::Button("Clear Operations"))
    {
        // If the FDC is in Executing or Result phases, leave the latest operation. Otherwise, remove all operations.
        if ((fdc->GetPhase() == CPC::CFdc::PHASE_EXECUTION) || (fdc->GetPhase() == CPC::CFdc::PHASE_RESULT))
        {
            if (m_fdcOperations.size() >= 2)
            {
                m_fdcOperations.erase(++m_fdcOperations.begin(), m_fdcOperations.end());
            }
        }
        else
        {
            m_fdcOperations.clear();
        }
    }

    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("FdcOperations", 4/*columns_count*/, tableFlags, ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 5.f)))
    {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Result", ImGuiTableColumnFlags_None);

        ImGui::TableHeadersRow();

        unsigned operationNumber = m_fdcOperationCounter;
        for (FdcOperationDeque::const_iterator iter = m_fdcOperations.begin(); iter != m_fdcOperations.end(); ++iter)
        {
            const FdcOperation& operation = *iter;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", operationNumber);
            ImGui::TableNextColumn();
            ImGui::Text("%s", CPC::CFdc::GetCommandName((CPC::CFdc::ECommand)operation.command));
            ImGui::TableNextColumn();
            ImGui::Text("%s", operation.parameters.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", operation.result.c_str());

            operationNumber--;
        }

        ImGui::EndTable();
    }

    // Disk drives.
    DrawDiskDrive(0, "Drive0", 0.5f);
    ImGui::SameLine();
    DrawDiskDrive(1, "Drive1", 1.f);
}

void Debugger::DrawDiskDrive(unsigned driveNumber, const char* imguiChildName, float contentRegionAvailProportion)
{
    const CPC::CDiskDrive* drive = m_machine->GetDiskDrive(driveNumber);
    const CPC::CDisk* disk = (drive != nullptr ? drive->GetDisk() : nullptr);

    ImGui::BeginChild(imguiChildName, ImVec2(ImGui::GetContentRegionAvail().x * contentRegionAvailProportion, ImGui::GetTextLineHeightWithSpacing() * 3.f + ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_None);

    // Header.
    ImGui::TextDisabled("Drive %d (%c:)", driveNumber, driveNumber == 0 ? 'A' : 'B');
    // Current side and track.
    if (disk != nullptr)
    {
        ImGui::Text("Track: %d (total: %d)", drive->GetTrack(), disk->GetTrackCount());
        if (ImGui::Button("Disk Structure"))
        {
            ShowDiskStructure(driveNumber, 0/*side*/, drive->GetTrack());
        }
    }
    else
    {
        ImGui::Text("No disk");
    }

    ImGui::EndChild();
}

void Debugger::ShowDiskStructure(unsigned drive, unsigned side, unsigned track)
{
    m_diskDrive = drive;
    m_diskSide = side;
    m_diskTrack = track;
}

void Debugger::HideDiskStructure()
{
    m_diskDrive = INVALID_DRIVE_NUMBER;
    m_diskSide = 0;
    m_diskTrack = 0;
}

void Debugger::DrawDiskStructure()
{
    const CPC::CDisk* disk = nullptr;
    if (m_diskDrive != INVALID_DRIVE_NUMBER)
    {
        disk = m_machine->GetDiskDrive(m_diskDrive)->GetDisk();

        if (disk == nullptr)
        {
            // The disk has been ejected with the Disk Structure window open.
            // Hide the window.
            HideDiskStructure();
        }
    }

    if (disk != nullptr)
    {
        bool keepOpen = true;
        if (ImGui::Begin("DiskStructure", &keepOpen, ImGuiWindowFlags_AlwaysAutoResize))
        {
            // Side and track selectors.
            ImGui::PushItemWidth(50.f);

            char label[10];
            snprintf(label, sizeof(label), "%d", m_diskSide);
            if (ImGui::BeginCombo("Side", label, ImGuiComboFlags_HeightSmall))
            {
                for (unsigned i = 0; i < disk->GetSideCount(); i++)
                {
                    snprintf(label, sizeof(label), "%d", i);
                    if (ImGui::Selectable(label, i == m_diskSide))
                    {
                        m_diskSide = i;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();

            snprintf(label, sizeof(label), "%d", m_diskTrack);
            if (ImGui::BeginCombo("Track", label, ImGuiComboFlags_HeightLargest))
            {
                for (unsigned i = 0; i < disk->GetTrackCount(); i++)
                {
                    snprintf(label, sizeof(label), "%d", i);
                    if (ImGui::Selectable(label, i == m_diskTrack))
                    {
                        m_diskTrack = i;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::PopItemWidth();

            // Sectors in current track.
            ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
            if (ImGui::BeginTable("DiskInfo", 7/*columns_count*/, tableFlags, ImVec2(0.f, 0.f)))
            {
                ImGui::TableSetupColumn("Side", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Track", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Status1", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Status2", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("DataLength", ImGuiTableColumnFlags_None);

                ImGui::TableHeadersRow();

                unsigned sectorCount = disk->GetSectorCount(m_diskSide, m_diskTrack);
                for (unsigned sector = 0; sector < sectorCount; sector++)
                {
                    const CPC::CDisk::SSectorInfo* sectorInfo = disk->GetSectorInfo(m_diskSide, m_diskTrack, sector);

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", sectorInfo->nSide);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", sectorInfo->nTrack);
                    ImGui::TableNextColumn();
                    ImGui::Text("%02hhX", sectorInfo->nId);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", sectorInfo->nSize);
                    ImGui::TableNextColumn();
                    ImGui::Text("%02hhX", sectorInfo->nStatusRegister1);
                    ImGui::TableNextColumn();
                    ImGui::Text("%02hhX", sectorInfo->nStatusRegister2);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", sectorInfo->nDataLength);
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();

        if (!keepOpen)
        {
            HideDiskStructure();
        }
    }
}

void Debugger::DrawUnsignedByte(const char* label, cpcByte byte, bool verticalLayout, const char* tooltip, ...)
{
    ImGui::BeginGroup();
    // Label.
    ImGui::TextDisabled(label);
    // Value.
    if (!verticalLayout)    // If horizontal layout requested...
    {
        ImGui::SameLine(/*0.f, 5.f*/);
    }
    ImGui::Text("%02hhX", byte);
    // Value border.
    static constexpr float BorderMargin = 2.f;
    LastItemBox(BorderMargin);

    ImGui::EndGroup();

    // Tooltip, if any.
    if (tooltip != nullptr)
    {
        va_list args;
        va_start(args, tooltip);

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextV(tooltip, args);
            ImGui::EndTooltip();
        }

        va_end(args);
    }
}

void Debugger::DrawUnsignedWord(const char* label, cpcWord word, bool verticalLayout, const char* tooltip, ...)
{
    ImGui::BeginGroup();
    // Label.
    ImGui::TextDisabled(label);
    // Value.
    if (!verticalLayout)    // If horizontal layout requested...
    {
        ImGui::SameLine(/*0.f, 5.f*/);
    }
    ImGui::Text("%04hX", word);
    // Value border.
    static constexpr float BorderMargin = 2.f;
    LastItemBox(BorderMargin);

    ImGui::EndGroup();

    // Tooltip, if any.
    if (tooltip != nullptr)
    {
        va_list args;
        va_start(args, tooltip);

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextV(tooltip, args);
            ImGui::EndTooltip();
        }

        va_end(args);
    }
}

void Debugger::DrawInt(const char* label, int n)
{
    // Label.
    ImGui::TextDisabled(label);
    // Value.
    ImGui::SameLine(/*0.f, 5.f*/);
    ImGui::Text("%d", n);
    // Value border.
    static constexpr float BorderMargin = 2.f;
    LastItemBox(BorderMargin);
}

void Debugger::LastItemBox(float margin)
{
    ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetItemRectMin().x - margin, ImGui::GetItemRectMin().y - margin),
                                        ImVec2(ImGui::GetItemRectMax().x + margin, ImGui::GetItemRectMax().y + margin),
                                        ImGui::GetColorU32(ImGuiCol_Border));
}

bool Debugger::BreakpointToggleButton(const char* str_id, cpcWord address, bool* hasBreakpoint)
{
    bool ret = false;

    ImGui::PushID(address);

    // Use an invisible button to detect clicks and mouse hover.
    static const ImVec2 totalSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight());
    if (ImGui::InvisibleButton(str_id, totalSize))
    {
        *hasBreakpoint = !*hasBreakpoint;
        ret = true;
    }

    bool isHovered = ImGui::IsItemHovered();

    ImGui::PopID();

    // Draw the breakpoint.
    if (*hasBreakpoint || isHovered)
    {
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();
        ImVec2 center = ImVec2((rectMin.x + rectMax.x) * 0.5f,
                               (rectMin.y + rectMax.y) * 0.5f);
        static const float Radius = 6.f;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (*hasBreakpoint)
        {
            drawList->AddCircleFilled(center, Radius, IM_COL32(255, 0, 0, 255));
        }
        else
        {
            drawList->AddCircle(center, Radius, IM_COL32(255, 0, 0, 255));
        }
    }

    return ret;
}

void Debugger::DrawCurrentInstructionArrow(const ImVec2& rectMin, const ImVec2& rectMax)
{
    ImVec2 center = ImVec2((rectMin.x + rectMax.x) * 0.5f,
                           (rectMin.y + rectMax.y) * 0.5f);
    static const float HalfWidth = 3.f;
    static const float HalfHeight = 4.f;
    ImVec2 p1 = ImVec2(center.x - HalfWidth, center.y - HalfHeight);
    ImVec2 p2 = ImVec2(center.x - HalfWidth, center.y + HalfHeight);
    ImVec2 p3 = ImVec2(center.x + HalfWidth, center.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 255, 0, 255));
}

void Debugger::RequestScrollToAddress(cpcWord address)
{
    m_scrollToAddress = address;
    m_scrollToAddressRequested = true;
}

bool Debugger::_OnAppWindowKeyDown(unsigned virtualKey, bool shift, bool ctrl, bool alt)
{
    bool ret = false;
    switch (virtualKey)
    {
    case VK_F11:
    {
        if (!shift && !ctrl && !alt)
        {
            // Step: Execute instruction at PC, or finish current one.
            ExecuteCurrentInstruction();
            ret = true;
        }
        else if (shift && !ctrl && !alt)
        {
            // Run To (start or stop).
            m_running = !m_running;
            ret = true;
        }
        else if (!shift && ctrl && !alt)
        {
            // Run Single Clock Cycle.
            RunSingleCycle();
            ret = true;
        }
        break;
    }
    }

    return ret;
}
