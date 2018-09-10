//-------------------------------------------------------------------------------------------
// File:        Cpu.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcCpu.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"
#include "cpcCpuInterface.h"


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
    Reset();
    m_waitActive = false;
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
    // Power-on defaults.
    m_activeGprSet = 0;
    memset(&m_registers, 0xFF, sizeof(m_registers));
    m_registers.IFF1 = 0;
    m_registers.IFF2 = 0;
    m_registers.PC.w = 0;
    m_registers.IR.w = 0;
    m_registers.IM = 0;

    m_numCyclesAhead = 0;
    m_opcodePrefix = OpcodePrefix::None;
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
    KMASSERTM(m_cpuInterface != NULL, ("Unassigned CPU interface. Please assign one by calling the method CCpu::SetCpuInterface. The program will crash if you continue."));

    // Accumulate cycles.
    m_numCyclesAhead -= nNumCycles;
    while (m_numCyclesAhead < 0)     // As long as we are behind the emulation clock...
    {
      // Take a step forward: either read prefix or fetch and execute instruction.
      Step();
    }
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  void CCpu::Step()
  {
    // Two things can happen:
    //  * If it's a prefix, the next byte of the prefix is read and remembered.
    //  * If it's an opcode, the whole instruction (i.e. opcode plus operands) is fetched and executed.

    // Read byte and increment PC.
    cpcByte byte = ReadByteFromMemory(m_registers.PC.w);
    m_registers.PC.w++;
    // Determine whether it's a prefix or an opcode.
    switch (byte)
    {
      case 0xED:
      {
        break;
      }

      case 0xCB:
      {
        if (m_opcodePrefix == OpcodePrefix::DD)         // If the previous byte was 0xDD...
        {
          m_opcodePrefix = OpcodePrefix::DDCB;
        }
        else if (m_opcodePrefix == OpcodePrefix::FD)    // If the previous byte was 0xFD...
        {
          m_opcodePrefix = OpcodePrefix::FDCB;
        }
        break;
      }

      case 0xDD:
      {
        break;
      }

      case 0xFD:
      {
        break;
      }

      default:
      {
        // We just read a non-prefix byte: assume it's an instruction.
        break;
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  void CCpu::FetchOpcode()
  {

  }

  ////----------------------------------------------------------------------------
  ///**
  //**
  //*/
  //void CCpu::AdvanceTStates(int numTStates)
  //{
  //  for (int i = 0; i < numTStates; i++)
  //  {
  //    m_numCyclesAhead++;
  //    m_cpuInterface->OnTState(this);
  //  }
  //}

  ////----------------------------------------------------------------------------
  ///**
  //**
  //*/
  //void CCpu::SyncToWaitSignal()
  //{
  //  while (m_waitActive)
  //  {
  //    // Wait for one clock cycle.
  //    m_numCyclesAhead++;
  //    m_cpuInterface->OnTState(this);
  //  }
  //}

  //----------------------------------------------------------------------------
  /**
  **
  */
  cpcByte CCpu::ReadByteFromMemory(cpcWord address)
  {
    cpcByte ret;
    //// Enter WAIT states while the /WAIT signal is active.
    //SyncToWaitSignal();
    // Read byte.
    ret = m_cpuInterface->ReadByteFromMemory(this, address);
    return ret;
  }

} //namespace CPC
