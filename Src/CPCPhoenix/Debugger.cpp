//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Debugger.h"
#include "Application.h"
#include "TextureVideoOutput.h"
#include "cpcMachine.h"
#include "cpcCpu.h"


bool Debugger::Init()
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
        //...
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
    m_stopAtBreakpoint = false;
    m_breakpointAddress = 0xF340;
    m_stopAtInterrupt = false;
    m_stopAtHSync = false;
    m_stopAtVSync = false;
    m_showMonitorOverlay = true;
}

void Debugger::FreeVars()
{
    //...
}

void Debugger::SetActive(bool active)
{
    m_active = active;
    if (active)
    {
        // Keep pointer to the machine up-to-date, it can change at any time.
        m_machine = Application::Singleton()->GetEmulatedMachine();
        // Finish current instruction.
        ExecuteCurrentInstruction();
    }
}

void Debugger::RunMachine()
{
    // Keep pointer to the machine up-to-date, it can change at any time.
    m_machine = Application::Singleton()->GetEmulatedMachine();
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
        if ((m_stopAtBreakpoint && !cpu->IsExecutingInstruction() && (cpu->GetRegisters().PC.w == m_breakpointAddress)) ||
            (m_stopAtInterrupt && cpu->InterruptWasAcknowledged()) ||
            (m_stopAtHSync && !previousHSync && crtc->GetHSyncState()) ||
            (m_stopAtVSync && !previousVSync && crtc->GetVSyncState()))
        {
            // Stop condition met. Stop running.
            m_running = false;
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
}

void Debugger::RunSingleCycle()
{
    // Run the machine for one 1-MHz clock cycle.
    m_machine->Run(4);
    // Update the screen.
    Application::Singleton()->GetTextureVideoOutput()->CaptureVideoOutputMidFrame();
}

void Debugger::DrawGui()
{
    // Keep pointer to the machine up-to-date, it can change at any time.
    m_machine = Application::Singleton()->GetEmulatedMachine();

    ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    // CPU.
    DrawCpu();
    // System (CRTC, Gate Array, PSG, etc.).
    DrawSystem();

    ImGui::End();
}

void Debugger::DrawVideoOutputOverlays()
{
    TextureVideoOutput* textureVideoOuput = Application::Singleton()->GetTextureVideoOutput();
    // Monitor beam position.
    if (m_showMonitorOverlay)
    {
        float adjustedBeamX = float(textureVideoOuput->GetBeamX()) - float(TextureVideoOutput::VIEWPORT_LEFT);
        float adjustedBeamY = float(textureVideoOuput->GetBeamY()) - float(TextureVideoOutput::VIEWPORT_TOP);
        ImVec2 beamGuiPos(textureVideoOuput->GetGuiRectMin().x + adjustedBeamX,        // In screen space.
            textureVideoOuput->GetGuiRectMin().y + (adjustedBeamY * 2.f));
        static constexpr float BeamGuideAlpha = 0.4f;
        if ((adjustedBeamX >= 0.f) && (adjustedBeamX <= TextureVideoOutput::VIEWPORT_WIDTH - 1))        // If currently inside the visible area of the screen...
        {
            ImGui::GetWindowDrawList()->AddLine(ImVec2(beamGuiPos.x, textureVideoOuput->GetGuiRectMin().y),
                ImVec2(beamGuiPos.x, textureVideoOuput->GetGuiRectMax().y),
                ImColor(1.f, 1.f, 1.f, BeamGuideAlpha));
        }

        if ((adjustedBeamY >= 0.f) && (adjustedBeamY <= TextureVideoOutput::VIEWPORT_HEIGHT - 1))       // If currently inside the visible area of the screen...
        {
            ImGui::GetWindowDrawList()->AddLine(ImVec2(textureVideoOuput->GetGuiRectMin().x, beamGuiPos.y),
                ImVec2(textureVideoOuput->GetGuiRectMax().x, beamGuiPos.y),
                ImColor(1.f, 1.f, 1.f, BeamGuideAlpha));
        }
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
    // Run To group.
    ImGui::BeginGroup();
    // +- Run To: button.
    string runToButtonLabel = (m_running ? "Stop Running (Shift+F11)" : "Run To (Shift+F11)");
    if (ImGui::Button(runToButtonLabel.c_str(), buttonSize))
    {
        m_running = !m_running;
    }
    // +- Run To: stop conditions.
    ImGui::Checkbox("Breakpoint", &m_stopAtBreakpoint);
    if (m_stopAtBreakpoint)
    {
        ImGui::SameLine();
        ImGui::InputScalar("Address", ImGuiDataType_U16, &m_breakpointAddress, nullptr, nullptr, "%04hX", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
    }
    ImGui::Checkbox("Interrupt", &m_stopAtInterrupt);
    ImGui::Checkbox("HSync", &m_stopAtHSync);
    ImGui::Checkbox("VSync", &m_stopAtVSync);
    ImGui::EndGroup();
    ImGui::SameLine();
    // Advance Clock Cycle.
    if (ImGui::Button("Clock Cycle (Ctrl+F11)", buttonSize))
    {
        // Advance one cycle of a 1-MHz clock.
        RunSingleCycle();
    }
}

void Debugger::DrawDisassembly()
{
    const CPC::CCpu* cpu = m_machine->GetCpu();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ColumnsWidthFixed;
    if (ImGui::BeginTable("Disassembly", 3/*columns_count*/, tableFlags, ImVec2(500.f, 0.f)))
    {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None, 100.f);      // Address.
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None, 60.f);       // Operation.
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None, 300.f);      // Operands.

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
                ImGui::TableNextColumn();
                ImGui::Text("%04X", address);

                ImGui::TableNextColumn();
                ImGui::Text("%s", instruction.operation.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", instruction.operands.c_str());

                address += instruction.sizeBytes;
            }
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

    DrawInt("Screen Mode", (int)gateArray->GetScreenMode());
    bool flag;
    flag = gateArray->IsHSyncActive(); ImGui::Checkbox("HSync", const_cast<bool*>(&flag)); ImGui::SameLine();
    flag = gateArray->IsVSyncActive(); ImGui::Checkbox("VSync", const_cast<bool*>(&flag));
}

void Debugger::DrawMonitor()
{
    const CPC::CVideoOutput* monitor = m_machine->GetVideoOutput();

    ImGui::Checkbox("Show Overlay", &m_showMonitorOverlay);
    ImGui::Text("Beam: %d,%d", monitor->GetBeamX(), monitor->GetBeamY());
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
