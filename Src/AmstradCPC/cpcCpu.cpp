//-------------------------------------------------------------------------------------------
// File:        Cpu.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcCpu.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"


namespace CPC {

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CCpu::CCpu(CMachine *pMachine) : inherited( pMachine )
  {
    ResetVars();

  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CCpu::ResetVars()
  {
      // Power-on defaults.
      m_activeGprSet = 0;
      memset(&m_registers, 0xFF, sizeof(m_registers));
      m_registers.PC.word = 0;
      m_registers.IFF1 = 0;
      m_registers.IFF2 = 0;
      m_registers.IM = 0;

      m_numCyclesAhead = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CCpu::FreeVars()
  {
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCpu::Reset()
  {
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  bool CCpu::RequestInterrupt()
  {
    return false;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCpu::Run(unsigned nNumCycles)
  {
    for (unsigned i = 0; i < nNumCycles; i++)
    {
      // Check whether the CPU emulation is still ahead of the rest of the emulator or not.
      if (m_numCyclesAhead == 0)
      {
        // Time to fetch and execute the next instruction.
        m_numCyclesAhead += FetchAndExecuteInstruction();
      }

      // Consume one clock cycle.
      m_numCyclesAhead--;
    }
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  unsigned CCpu::FetchAndExecuteInstruction()
  {
    unsigned ret = xxx;
    FetchOpcode();

    // TODO - Check interrupts.

    return ret;
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  void CCpu::FetchOpcode()
  {

  }

} //namespace CPC
