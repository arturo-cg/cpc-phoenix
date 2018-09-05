
#include "stdafx.h"
#include "cpcCpuToCpcInterface.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"


namespace CPC {

  CCpuToCpcInterface::CCpuToCpcInterface(CMachine* machine)
  {
    m_machine = machine;
  }

  void CCpuToCpcInterface::OnReset(CCpu* cpu)
  {
  }

  void CCpuToCpcInterface::OnTCycle(CCpu* cpu)
  {
  }

  cpcByte CCpuToCpcInterface::ReadByteFromMemory(CCpu* cpu, cpcWord address)
  {
    return m_machine->GetGateArray()->ReadByteFromMemory(address);
  }

  void CCpuToCpcInterface::WriteByteToMemory(CCpu* cpu, cpcWord address, cpcByte value)
  {
    m_machine->GetGateArray()->WriteByteToMemory(address, value);
  }

  cpcByte CCpuToCpcInterface::ReadByteFromPort(CCpu* cpu, cpcWord port)
  {
    return m_machine->ReadByteFromPort(port);
  }

  void CCpuToCpcInterface::WriteByteToPort(CCpu* cpu, cpcWord port, cpcByte value)
  {
    m_machine->WriteByteToPort(port, value);
  }

} //namespace CPC
