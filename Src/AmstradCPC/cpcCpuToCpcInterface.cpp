
#include "stdafx.h"
#include "cpcCpuToCpcInterface.h"
#include "cpcMachine.h"
#include "cpcCpu.h"
#include "cpcGateArray.h"


namespace CPC {

  CCpuToCpcInterface::CCpuToCpcInterface(CMachine* machine)
  {
    m_machine = machine;
    m_tStateCounter = 0;
  }

  void CCpuToCpcInterface::OnReset(CCpu* cpu)
  {
    m_tStateCounter = 0;
  }

  void CCpuToCpcInterface::OnTState(CCpu* cpu)
  {
    // The Gate Array controls the CPU's WAIT input signal so that it is inactive 1 out of every 4 T states.
    // The WAIT signal remains active the other 3 out of every 4 T states.
    m_tStateCounter = m_tStateCounter % 4;
    cpu->SetWaitSignalActive(m_tStateCounter != 0);   // T state 0: inactive; T states 1, 2 and 3: active.
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
