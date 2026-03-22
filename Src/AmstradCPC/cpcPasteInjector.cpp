#include "stdafx.h"
#include "cpcCpu.h"
#include "cpcMachine.h"
#include "cpcPasteInjector.h"

CPC::PasteInjector::PasteInjector(CMachine* owningMachine)
{
    ResetVars();

    m_machine = owningMachine;
}

CPC::PasteInjector::~PasteInjector()
{
}

void CPC::PasteInjector::ResetVars()
{
    m_machine = nullptr;
    m_pendingCharacters.clear();
}

void CPC::PasteInjector::FreeVars()
{
}

void CPC::PasteInjector::PasteCharacters(const std::deque<cpcByte>& cpcCharacters)
{
    m_pendingCharacters.insert(m_pendingCharacters.end(), cpcCharacters.begin(), cpcCharacters.end());
}

void CPC::PasteInjector::PasteEnter()
{
    m_pendingCharacters.push_back(CpcAscii_Enter);
}

std::deque<cpcByte> CPC::PasteInjector::ConvertHostToCpcAscii(const std::string& hostText)
{
    std::deque<cpcByte> cpcCharacters;
    for (size_t i = 0; i < hostText.size(); i++)
    {
        char c = hostText[i];
        if ((c >= 0x20) && (c <= 0x7E))   // If a printable ASCII code that doesn't need translation...
        {
            cpcCharacters.push_back(cpcByte(c));
        }
        else
        {
            switch (c)
            {
                case '\r':
                {
                    cpcCharacters.push_back(CpcAscii_Enter);
                    break;
                }

                case '\n':
                {
                    // Add an Enter character if and only if the \n is not preceded by a \r, otherwise we would incorrectly introduce an extra new line.
                    if ((i == 0) || (hostText[i - 1] != '\r'))
                    {
                        cpcCharacters.push_back(CpcAscii_Enter);
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    return cpcCharacters;
}

void CPC::PasteInjector::Run()
{
    if (!m_pendingCharacters.empty())     // If there are characters waiting to be injected...
    {
        // Is the CPU just about to start executing the firmware routine KM WAIT CHAR?
        const CCpu* cpu = m_machine->GetCpu();
        cpcWord routineAddress_KM_WAIT_CHAR = m_machine->GetMemory()->GetRoutineAddress_KM_WAIT_CHAR();
        if ((cpu->GetRegisters().PC.w == routineAddress_KM_WAIT_CHAR) &&    // If PC is at the start of the routine...
            !cpu->IsExecutingInstruction())                                 // If it has *not* started executing the instruction at the routine jumpblock...
        {
            const CGateArray* gateArray = m_machine->GetGateArray();
            if (gateArray->IsLowerRomVisible())                         // If the CPU is reading instructions from the firmware ROM...
            {
                // We have confirmed that it is executing the firmware routine KM WAIT CHAR (&BB18).
                // Inject next character.
                InjectCharacter(m_pendingCharacters.front());
                // Remove injected character from the queue.
                m_pendingCharacters.pop_front();
            }
        }
    }
}

void CPC::PasteInjector::InjectCharacter(cpcByte character)
{
    CCpu* cpu = m_machine->GetCpu();
    CCpu::Registers registers = cpu->GetRegisters();
    const CGateArray* gateArray = m_machine->GetGateArray();

    // Place the given character in register A.
    // Set flag Carry to true.
    registers.A() = character;
    registers.SetFlag(CCpu::Registers::Flag_C, true);
    // Simulate a RET instruction (pop PC from the stack) to bypass the actual firmware routine.
    registers.PC.b.l = gateArray->ReadByteFromMemory(registers.SP.w);
    registers.SP.w++;
    registers.PC.b.h = gateArray->ReadByteFromMemory(registers.SP.w);
    registers.SP.w++;

    cpu->SetRegisters(registers);
}
