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

  CCpu::OpcodeInfo CCpu::m_opcodesMain[256] = {

    // This yields something similar to this:
    //
    // { &CCpu::Execute_00, false, "NOP" },
    // { &CCpu::Execute_01, false, "LD BC, %nn" },
    // { &CCpu::Execute_02, false, "LD (BC), A" },
    // { &CCpu::Execute_03, false, "INC BC" },
    // ...
#define Z80_OPCODE(_num, _isPrefix, _mnemonic, _microCode) { &CCpu::Execute_##_num, _isPrefix, _mnemonic },
#include "cpcCpu_MainOpcodes.h"
#undef Z80_OPCODE

  };

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CCpu::CCpu(CMachine *pMachine) : inherited( pMachine )
  {
    ResetVars();

    m_opcodes[Prefix::None] = m_opcodesMain;
    m_opcodes[Prefix::ED] = m_opcodesED;
    m_opcodes[Prefix::CB] = m_opcodesCB;
    m_opcodes[Prefix::DD] = m_opcodesDD;
    m_opcodes[Prefix::DDCB] = m_opcodesDDCB;
    m_opcodes[Prefix::FD] = m_opcodesFD;
    m_opcodes[Prefix::FDCB] = m_opcodesFDCB;
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
    memset(&m_registers, 0xFF, sizeof(m_registers));
    m_registers.IFF1 = 0;
    m_registers.IFF2 = 0;
    m_registers.PC.w = 0;
    m_registers.IR.w = 0;
    m_registers.IM = 0;

    m_numCyclesAhead = 0;
    m_prefix = Prefix::None;
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

    // Fetch opcode/prefix.
    cpcByte opcode = FetchByte();
    // Execute instruction or remember prefix.
    OpcodeInfo* table = m_opcodes[m_prefix];
    OpcodeInfo* opcodeInfo = &table[opcode];
    std::invoke(opcodeInfo->microcodeFn, this);
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  cpcByte CCpu::FetchByte()
  {
    // Read byte and increment PC.
    cpcByte ret = ReadByteFromMemory(m_registers.PC.w);
    m_registers.PC.w++;
    return ret;
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

  //----------------------------------------------------------------------------
  /**
  **
  */
  void CCpu::WriteByteToMemory(cpcWord address, cpcByte value)
  {
    //// Enter WAIT states while the /WAIT signal is active.
    //SyncToWaitSignal();
    // Write byte.
    m_cpuInterface->WriteByteToMemory(this, address, value);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  // Instructions.
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  void CCpu::LD8_reg_reg(cpcByte* dest, cpcByte value)
  {
      *dest = value;
  }

  void CCpu::LD8_reg_n(cpcByte* byte)
  {
      *byte = FetchByte();
  }

  void CCpu::LD8_reg_mem(cpcByte* byte, const Reg16& addressReg)
  {
      *byte = ReadByteFromMemory(addressReg.w);
  }

  void CCpu::LD8_addrreg_valuereg(const Reg16& addressReg, cpcByte valueReg)
  {
      WriteByteToMemory(addressReg.w, valueReg);
  }

  void CCpu::LD8_addrreg_n(const Reg16& addressReg)
  {
      cpcByte value = FetchByte();
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::LD8_addrnn_reg(cpcByte value)
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = FetchByte();
      WriteByteToMemory(address.w, value);
  }

  void CCpu::LD8_reg_addrreg(cpcByte* dest, const Reg16& addressReg)
  {
      *dest = ReadByteFromMemory(addressReg.w);
  }

  void CCpu::INC8_reg(cpcByte* byte)
  {
      (*byte)++;
      m_registers.SetFlag(Registers::Flag_S, ((*byte) & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, ((*byte) & 0x0F) == 0);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, (*byte) == 0x80);
      m_registers.SetFlag(Registers::Flag_N, false);
      // Registers::Flag_C unaffected.
  }

  void CCpu::DEC8_reg(cpcByte* byte)
  {
      (*byte)--;
      m_registers.SetFlag(Registers::Flag_S, ((*byte) & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, ((*byte) & 0x0F) == 0x0F);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, (*byte) == 0x7F);
      m_registers.SetFlag(Registers::Flag_N, true);
      // Registers::Flag_C unaffected.
  }

  void CCpu::INC8_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      INC8_reg(&value);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::DEC8_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      DEC8_reg(&value);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::INC16_reg(Reg16* reg)
  {
      reg->w++;
  }

  void CCpu::DEC16_reg(Reg16* reg)
  {
      reg->w--;
  }

  void CCpu::CPL()
  {
      m_registers.A() = ~m_registers.A();
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, true);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, true);
  }

  void CCpu::DAA()
  {
      int tmp = m_registers.A();

      if (!m_registers.GetFlag(Registers::Flag_N))
      {
          if (m_registers.GetFlag(Registers::Flag_H) || (tmp & 0x0F) > 9)
          {
              tmp += 6;
          }
          if (m_registers.GetFlag(Registers::Flag_C) || tmp > 0x9F)
          {
              tmp += 0x60;
          }
      }
      else
      {
          if (m_registers.GetFlag(Registers::Flag_H))
          {
              tmp -= 6;
              if (!m_registers.GetFlag(Registers::Flag_C))
              {
                  tmp &= 0xFF;
              }
          }
          if (m_registers.GetFlag(Registers::Flag_C))
          {
              tmp -= 0x60;
          }
      }
      m_registers.SetFlag(Registers::Flag_H, false);
      if (tmp & 0x100)
      {
          m_registers.SetFlag(Registers::Flag_C, true);
      }
      m_registers.A() = tmp & 0xFF;
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
  }

  void CCpu::LD16_reg_nn(Reg16* reg)
  {
    reg->b.l = FetchByte();
    reg->b.h = FetchByte();
  }

  void CCpu::LD16_addrnn_reg(const Reg16& value)
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = FetchByte();
      WriteByteToMemory(address.w, value.b.l);
      WriteByteToMemory(address.w + 1, value.b.h);
  }

  void CCpu::LD16_reg_addrnn(Reg16* reg)
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = FetchByte();
      reg->b.l = ReadByteFromMemory(address.w);
      reg->b.h = ReadByteFromMemory(address.w + 1);
  }

  void CCpu::ADD16_reg_reg(Reg16* a, Reg16 b)
  {
      m_registers.SetFlag(Registers::Flag_H, (((a->w & 0x0FFF) + (b.w & 0x0FFF)) & 0x1000) != 0);
      uint32_t longResult = uint32_t(a->w) + uint32_t(b.w);
      a->w += b.w;
      m_registers.SetFlag(Registers::Flag_S, (a->w & 0x8000) != 0);                                     // TODO: Unclear if flag affected.
      m_registers.SetFlag(Registers::Flag_Z, a->w == 0);                                                // TODO: Unclear if flag affected.
      m_registers.SetFlag(Registers::Flag_5, (a->w & 0x2000) != 0);
      m_registers.SetFlag(Registers::Flag_3, (a->w & 0x0800) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, (longResult & 0x10000) != 0);
  }

  void CCpu::RL(cpcByte* byte)
  {
      bool msb = ((*byte) & 0x80) != 0;
      *byte = ((*byte) << 1) | (m_registers.GetFlag(Registers::Flag_C) ? 0x01 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, msb);
  }

  void CCpu::RLC(cpcByte* byte)
  {
      bool msb = ((*byte) & 0x80) != 0;
      *byte = ((*byte) << 1) | (msb ? 0x01 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, msb);
  }

  void CCpu::RR(cpcByte* byte)
  {
      bool lsb = ((*byte) & 0x01) != 0;
      *byte = ((*byte) >> 1) | (m_registers.GetFlag(Registers::Flag_C) ? 0x80 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, lsb);
  }

  void CCpu::RRC(cpcByte* byte)
  {
      bool lsb = ((*byte) & 0x01) != 0;
      *byte = ((*byte) >> 1) | (lsb ? 0x80 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, lsb);
  }

  void CCpu::EX_reg_reg(Reg16* a, Reg16* b)
  {
      std::swap(a->w, b->w);
  }

  void CCpu::SCF()
  {
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, true);
  }

  void CCpu::JR_n()
  {
      cpcByte displacement = FetchByte();
      m_registers.PC.w += displacement;
  }

  void CCpu::JR_condition_n(bool condition)
  {
      cpcByte displacement = FetchByte();
      if (condition)
      {
          m_registers.PC.w = m_registers.PC.w + displacement;
      }
  }

  void CCpu::DJNZ_n()
  {
      cpcByte displacement = FetchByte();
      m_registers.B()--;
      if (m_registers.B() != 0)
      {
          m_registers.PC.w = m_registers.PC.w + displacement;
      }
  }

} //namespace CPC
