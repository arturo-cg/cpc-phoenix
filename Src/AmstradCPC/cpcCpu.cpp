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
//#include <iostream>
//#include <sstream>
//#include <Windows.h>


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

  CCpu::OpcodeInfo CCpu::m_opcodesED[256] = {
#define Z80_OPCODE(_num, _isPrefix, _mnemonic, _microCode) { &CCpu::Execute_ED##_num, _isPrefix, _mnemonic },
#include "cpcCpu_OpcodesED.h"
#undef Z80_OPCODE
  };

  CCpu::OpcodeInfo CCpu::m_opcodesCB[256] = {
#define Z80_OPCODE(_num, _isPrefix, _mnemonic, _microCode) { &CCpu::Execute_CB##_num, _isPrefix, _mnemonic },
#include "cpcCpu_OpcodesCB.h"
#undef Z80_OPCODE
  };

  CCpu::OpcodeInfo CCpu::m_opcodesDD[256] = {
#define Z80_OPCODE(_num, _isPrefix, _mnemonic, _microCode) { &CCpu::Execute_DD##_num, _isPrefix, _mnemonic },
#include "cpcCpu_OpcodesDD.h"
#undef Z80_OPCODE
  };

  CCpu::OpcodeInfo CCpu::m_opcodesDDCB[256] = {
#define Z80_OPCODE(_num, _isPrefix, _mnemonic, _microCode) { &CCpu::Execute_DDCB##_num, _isPrefix, _mnemonic },
#include "cpcCpu_OpcodesDDCB.h"
#undef Z80_OPCODE
  };

  CCpu::OpcodeInfo CCpu::m_opcodesFD[256] = {
#define Z80_OPCODE(_num, _isPrefix, _mnemonic, _microCode) { &CCpu::Execute_FD##_num, _isPrefix, _mnemonic },
#include "cpcCpu_OpcodesFD.h"
#undef Z80_OPCODE
  };

  CCpu::OpcodeInfo CCpu::m_opcodesFDCB[256] = {
#define Z80_OPCODE(_num, _isPrefix, _mnemonic, _microCode) { &CCpu::Execute_FDCB##_num, _isPrefix, _mnemonic },
#include "cpcCpu_OpcodesFDCB.h"
#undef Z80_OPCODE
  };

  CCpu::StaticInitializer CCpu::s_staticInitializer;
  bool CCpu::s_parity[256];

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CCpu::StaticInitializer::StaticInitializer()
  {
      // Parity look-up table.
      for (int i = 0; i < 256; i++)
      {
          bool parity = true;
          if ((i & 0x01) != 0)  parity = !parity;
          if ((i & 0x02) != 0)  parity = !parity;
          if ((i & 0x04) != 0)  parity = !parity;
          if ((i & 0x08) != 0)  parity = !parity;
          if ((i & 0x10) != 0)  parity = !parity;
          if ((i & 0x20) != 0)  parity = !parity;
          if ((i & 0x40) != 0)  parity = !parity;
          if ((i & 0x80) != 0)  parity = !parity;
          s_parity[i] = parity;
      }
  }

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
    m_cpuInterface = NULL;
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
    m_inHalt = false;
    m_delayInterruptEnable = false;
    m_interruptRequestActive = false;
    m_interruptVector = 0;
    m_nmiRequested = false;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCpu::Run(unsigned nNumCycles)
  {
    KMASSERTM(m_cpuInterface != NULL, ("Unassigned CPU interface. Please assign one by calling the method CCpu::SetCpuInterface. The program will crash if you continue."));

    // Accumulate cycles.
    m_numCyclesAhead -= (int)nNumCycles;
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
      // Execute interrupt?
      if ((m_prefix == Prefix::None) &&     // If we are not in the middle of a multi-byte instruction...
          m_nmiRequested)                   // If an NMI has been requested...
      {
          // NMI.
          AcceptNmi();
      }
      else if ((m_prefix == Prefix::None) &&     // If we are not in the middle of a multi-byte instruction...
               m_interruptRequestActive &&       // If an interrupt is currently being requested...
               m_registers.IFF1 &&               // If maskable interrupts are enabled...
               !m_delayInterruptEnable)          // If the last instruction was not EI...
      {
          // Interrupt.
          AcceptInterrupt();
      }
      else
      {
          // Fetch and execute opcode.
          cpcByte opcode = (m_inHalt ? 0x00/*NOP*/ : FetchByte());
          StepOpcode(opcode);
      }
  }

  void CCpu::StepOpcode(cpcByte opcode)
  {
      // Two things can happen:
      //  * If it's a prefix, it is simply remembered.
      //  * If it's an opcode, the whole instruction (i.e. opcode plus operands) is fetched and executed.

      m_delayInterruptEnable = false;
      // Execute instruction or remember prefix.
      OpcodeInfo* table = m_opcodes[m_prefix];
      OpcodeInfo* opcodeInfo = &table[opcode];
      //{
      //    std::ostringstream ss;
      //    ss << std::hex << m_registers.PC.w - 1 << "    " << opcodeInfo->mnemonic << "\n";
      //    //std::cout << ss.str();
      //    OutputDebugString(ss.str().c_str());
      //}
      std::invoke(opcodeInfo->microcodeFn, this);
      if (opcodeInfo->isInstruction)   // If we just executed an instruction...
      {
          // Reset prefix.
          m_prefix = Prefix::None;
          // Consume cycles.
          // TODO: implement correct timing.
          m_numCyclesAhead += 16;
      }
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

  void CCpu::AcceptNmi()
  {
      // Resume normal execution if currently in a HALT instruction.
      m_inHalt = false;
      // Don't allow further interrupts.
      // Note that only IFF1 is changed; IFF2 is left as a backup of IFF1's original value so that a RETN instruction can restore it later on.
      m_registers.IFF1 = false;

      // Jump to NMI handler.
      // Equivalent to executing an imaginary instruction RST 66H.
      Push(m_registers.PC);
      m_registers.PC.w = 0x66;
  }

  void CCpu::AcceptInterrupt()
  {
      //{
      //    OutputDebugString("-- Interrupt --\n");
      //}
      // Resume normal execution if currently in a HALT instruction.
      m_inHalt = false;
      // Don't allow further interrupts.
      m_registers.IFF1 = false;
      m_registers.IFF2 = false;
      // Acknowledge interrupt.
      m_cpuInterface->OnInterruptAcknowledge(this);

      switch (m_registers.IM)
      {
      case 0:
          StepOpcode(m_interruptVector);
          break;

      case 1:
          StepOpcode(0xFF/*RST 38H*/);                                                                                                          
          break;

      case 2:
          //
          // TODO
          //
          break;

      default:
          KMASSERTM(false, ("Invalid interrupt mode specified."));
          break;
      }
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
  void CCpu::ProcessPrefixByte(cpcByte prefixByte)
  {
      // Is it a 2-byte prefix?
      if ((prefixByte == 0xCB) &&                                     // If current byte is CB...
          ((m_prefix == Prefix::DD) || (m_prefix == Prefix::FD)))     // If previous byte was either DD or FD...
      {
          m_prefix = (m_prefix == Prefix::DD ? Prefix::DDCB : Prefix::FDCB);
      }
      else
      {
          // It's a 1-byte prefix.
          // Ignore the previous prefix, if any.
          switch (prefixByte)
          {
              case 0xED:  m_prefix = Prefix::ED; break;
              case 0xCB:  m_prefix = Prefix::CB; break;
              case 0xDD:  m_prefix = Prefix::DD; break;
              case 0xFD:  m_prefix = Prefix::FD; break;
          }
      }
  }

  void CCpu::HandleInvalidInstruction()
  {
      // Reset prefix.
      m_prefix = Prefix::None;
      // Disallow interrupts immediately after an invalid instruction.
      m_delayInterruptEnable = true;
  }

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
  /**
  **
  */
  cpcByte CCpu::ReadByteFromPort(cpcWord address)
  {
    cpcByte ret;
    //// Enter WAIT states while the /WAIT signal is active.
    //SyncToWaitSignal();
    // Read byte.
    ret = m_cpuInterface->ReadByteFromPort(this, address);
    return ret;
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  void CCpu::WriteByteToPort(cpcWord address, cpcByte value)
  {
    //// Enter WAIT states while the /WAIT signal is active.
    //SyncToWaitSignal();
    // Write byte.
    m_cpuInterface->WriteByteToPort(this, address, value);
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  void CCpu::Push(const Reg16& value)
  {
      m_registers.SP.w--;
      WriteByteToMemory(m_registers.SP.w, value.b.h);
      m_registers.SP.w--;
      WriteByteToMemory(m_registers.SP.w, value.b.l);
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  void CCpu::Pop(Reg16* value)
  {
      value->b.l = ReadByteFromMemory(m_registers.SP.w);
      m_registers.SP.w++;
      value->b.h = ReadByteFromMemory(m_registers.SP.w);
      m_registers.SP.w++;
  }

  //----------------------------------------------------------------------------
  /**
  **
  */
  cpcWord CCpu::ConvertSignedByteToWord(cpcByte value) const
  {
      // Sign-extend the provided two's complement number.
      cpcWord ret = ((value & 0x80) != 0 ? 0xFF00 : 0x0000);  // Higher byte.
      ret |= cpcWord(value);                                  // Lower byte.
      return ret;
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

  void CCpu::LD8_reg_addrnn(cpcByte* dest)
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = FetchByte();
      *dest = ReadByteFromMemory(address.w);
  }

  void CCpu::LD8_reg_addrreg(cpcByte* dest, const Reg16& addressReg)
  {
      *dest = ReadByteFromMemory(addressReg.w);
  }

  void CCpu::LD8_addrreg_offset_n(const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      cpcByte value = FetchByte();
      WriteByteToMemory(addressReg.w + ConvertSignedByteToWord(displacement), value);
  }

  void CCpu::LD8_addrreg_offset_valuereg(const Reg16& addressReg, cpcByte value)
  {
      cpcByte displacement = FetchByte();
      WriteByteToMemory(addressReg.w + ConvertSignedByteToWord(displacement), value);
  }

  void CCpu::LD8_reg_addrreg_offset(cpcByte* dest, const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      *dest = ReadByteFromMemory(addressReg.w + ConvertSignedByteToWord(displacement));
  }

  void CCpu::LDI()
  {
      cpcByte value = ReadByteFromMemory(m_registers.HL.w);
      WriteByteToMemory(m_registers.DE.w, value);
      m_registers.HL.w++;
      m_registers.DE.w++;
      m_registers.BC.w--;

      cpcByte valuePlusA = value + m_registers.A();
      m_registers.SetFlag(Registers::Flag_5, (valuePlusA & 0x02) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (valuePlusA & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, m_registers.BC.w != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
  }

  void CCpu::LDIR()
  {
      // Do a LDI.
      LDI();
      // Repeat if BC != 0.
      if (m_registers.BC.w != 0)
      {
          m_registers.PC.w -= 2;    // Note that LDIR is a 2-byte instruction.
      }
  }

  void CCpu::LDD()
  {
      cpcByte value = ReadByteFromMemory(m_registers.HL.w);
      WriteByteToMemory(m_registers.DE.w, value);
      m_registers.HL.w--;
      m_registers.DE.w--;
      m_registers.BC.w--;

      cpcByte valuePlusA = value + m_registers.A();
      m_registers.SetFlag(Registers::Flag_5, (valuePlusA & 0x02) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (valuePlusA & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, m_registers.BC.w != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
  }

  void CCpu::LDDR()
  {
      // Do a LDD.
      LDD();
      // Repeat if BC != 0.
      if (m_registers.BC.w != 0)
      {
          m_registers.PC.w -= 2;    // Note that LDDR is a 2-byte instruction.
      }
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

  void CCpu::INC8_addrreg_offset(const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      cpcWord address = addressReg.w + ConvertSignedByteToWord(displacement);
      cpcByte value = ReadByteFromMemory(address);
      INC8_reg(&value);
      WriteByteToMemory(address, value);
  }

  void CCpu::DEC8_addrreg_offset(const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      cpcWord address = addressReg.w + ConvertSignedByteToWord(displacement);
      cpcByte value = ReadByteFromMemory(address);
      DEC8_reg(&value);
      WriteByteToMemory(address, value);
  }

  void CCpu::INC16_reg(Reg16* reg)
  {
      reg->w++;
  }

  void CCpu::DEC16_reg(Reg16* reg)
  {
      reg->w--;
  }

  void CCpu::AND_reg(cpcByte b)
  {
      m_registers.A() &= b;
      m_registers.SetFlag(Registers::Flag_S, (m_registers.A() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, true);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[m_registers.A()]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, false);
  }

  void CCpu::AND_n()
  {
      cpcByte b = FetchByte();
      AND_reg(b);
  }

  void CCpu::AND_addrreg(const Reg16& addressReg)
  {
      cpcByte b = ReadByteFromMemory(addressReg.w);
      AND_reg(b);
  }

  void CCpu::AND_addrreg_offset(const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      cpcByte b = ReadByteFromMemory(addressReg.w + ConvertSignedByteToWord(displacement));
      AND_reg(b);
  }

  void CCpu::OR_reg(cpcByte b)
  {
      m_registers.A() |= b;
      m_registers.SetFlag(Registers::Flag_S, (m_registers.A() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[m_registers.A()]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, false);
  }

  void CCpu::OR_n()
  {
      cpcByte b = FetchByte();
      OR_reg(b);
  }

  void CCpu::OR_addrreg(const Reg16& addressReg)
  {
      cpcByte b = ReadByteFromMemory(addressReg.w);
      OR_reg(b);
  }

  void CCpu::OR_addrreg_offset(const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      cpcByte b = ReadByteFromMemory(addressReg.w + ConvertSignedByteToWord(displacement));
      OR_reg(b);
  }

  void CCpu::XOR_reg(cpcByte b)
  {
      m_registers.A() ^= b;
      m_registers.SetFlag(Registers::Flag_S, (m_registers.A() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[m_registers.A()]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, false);
  }

  void CCpu::XOR_n()
  {
      cpcByte b = FetchByte();
      XOR_reg(b);
  }

  void CCpu::XOR_addrreg(const Reg16& addressReg)
  {
      cpcByte b = ReadByteFromMemory(addressReg.w);
      XOR_reg(b);
  }

  void CCpu::XOR_addrreg_offset(const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      cpcByte b = ReadByteFromMemory(addressReg.w + ConvertSignedByteToWord(displacement));
      XOR_reg(b);
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

  void CCpu::LD16_reg_reg(Reg16* dest, const Reg16& src)
  {
      *dest = src;
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

  void CCpu::ADD8_reg_reg(cpcByte* a, cpcByte b, cpcByte carry)
  {
      cpcByte oldA = *a;
      *a = *a + b + carry;
      m_registers.SetFlag(Registers::Flag_S, ((*a) & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*a) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*a) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, ((oldA & 0x0F) + (b & 0x0F) + carry) > 0x0F);
      m_registers.SetFlag(Registers::Flag_3, ((*a) & 0x08) != 0);
      // Overflow happens when adding two numbers with the same sign and the result has a different sign.
      // See: http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
      m_registers.SetFlag(Registers::Flag_PV, (((~(oldA ^ b)) & (oldA ^ *a)) & 0x80) != 0);  // Have a and b same sign, and result different sign?
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, (cpcWord(oldA) + cpcWord(b) + cpcWord(carry)) > 0xFF);
  }

  void CCpu::ADD8_reg_n(cpcByte* a, cpcByte carry)
  {
      cpcByte b = FetchByte();
      ADD8_reg_reg(a, b, carry);
  }

  void CCpu::ADD8_reg_addrreg(cpcByte* a, const Reg16& addressReg, cpcByte carry)
  {
      cpcByte b = ReadByteFromMemory(addressReg.w);
      ADD8_reg_reg(a, b, carry);
  }

  void CCpu::ADD8_reg_addrreg_offset(cpcByte* a, const Reg16& addressReg, cpcByte carry)
  {
      cpcByte displacement = FetchByte();
      cpcByte b = ReadByteFromMemory(addressReg.w + ConvertSignedByteToWord(displacement));
      ADD8_reg_reg(a, b, carry);
  }

  void CCpu::ADD16_reg_reg(Reg16* a, Reg16 b)
  {
      m_registers.SetFlag(Registers::Flag_H, (((a->w & 0x0FFF) + (b.w & 0x0FFF)) & 0x1000) != 0);
      uint32_t longResult = uint32_t(a->w) + uint32_t(b.w);
      a->w += b.w;
      m_registers.SetFlag(Registers::Flag_5, (a->w & 0x2000) != 0);
      m_registers.SetFlag(Registers::Flag_3, (a->w & 0x0800) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, (longResult & 0x10000) != 0);
  }

  void CCpu::SUB8_reg(cpcByte b, cpcByte borrow)
  {
      cpcByte oldA = m_registers.A();
      m_registers.A() = m_registers.A() - b - borrow;
      m_registers.SetFlag(Registers::Flag_S, (m_registers.A() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, ((oldA & 0x0F) < ((b & 0x0F) + borrow)));
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, ((((oldA ^ b)) & (oldA ^ m_registers.A())) & 0x80) != 0);  // Have a and b different sign, and result different sign?
      m_registers.SetFlag(Registers::Flag_N, true);
      m_registers.SetFlag(Registers::Flag_C, oldA < (b + borrow));
  }

  void CCpu::SUB8_n(cpcByte borrow)
  {
      cpcByte b = FetchByte();
      SUB8_reg(b, borrow);
  }
  
  void CCpu::SUB8_addrreg(const Reg16& addressReg, cpcByte borrow)
  {
      cpcByte b = ReadByteFromMemory(addressReg.w);
      SUB8_reg(b, borrow);
  }

  void CCpu::SUB8_addrreg_offset(const Reg16& addressReg, cpcByte borrow)
  {
      cpcByte displacement = FetchByte();
      cpcByte b = ReadByteFromMemory(addressReg.w + ConvertSignedByteToWord(displacement));
      SUB8_reg(b, borrow);
  }

  void CCpu::SUB16_reg_reg(Reg16* a, Reg16 b, cpcWord borrow)
  {
      cpcWord oldA = a->w;
      a->w = a->w - b.w - borrow;
      m_registers.SetFlag(Registers::Flag_S, (a->w & 0x8000) != 0);
      m_registers.SetFlag(Registers::Flag_Z, a->w == 0);
      m_registers.SetFlag(Registers::Flag_5, (a->w & 0x2000) != 0);
      m_registers.SetFlag(Registers::Flag_H, ((oldA & 0x0F) < ((b.w & 0x0F) + borrow)));
      m_registers.SetFlag(Registers::Flag_3, (a->w & 0x0800) != 0);
      m_registers.SetFlag(Registers::Flag_PV, ((((oldA ^ b.w)) & (oldA ^ a->w)) & 0x80) != 0);  // Have a and b different sign, and result different sign?
      m_registers.SetFlag(Registers::Flag_N, true);
      m_registers.SetFlag(Registers::Flag_C, oldA < (b.w + borrow));
  }

  void CCpu::NEG()
  {
      cpcByte oldA = m_registers.A();
      m_registers.A() = 0 - m_registers.A();
      m_registers.SetFlag(Registers::Flag_S, (m_registers.A() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, (0 < ((m_registers.A() & 0x0F))));
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, ((oldA & m_registers.A()) & 0x80) != 0);  // Have a and b different sign, and result different sign?
      m_registers.SetFlag(Registers::Flag_N, true);
      m_registers.SetFlag(Registers::Flag_C, 0 < oldA);
  }

  void CCpu::RL_reg(cpcByte* byte)
  {
      bool msb = ((*byte) & 0x80) != 0;
      *byte = ((*byte) << 1) | (m_registers.GetFlag(Registers::Flag_C) ? 0x01 : 0x00);
      m_registers.SetFlag(Registers::Flag_S, ((*byte) & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[*byte]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, msb);
  }

  void CCpu::RL_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      RL_reg(&value);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::RLC_reg(cpcByte* byte)
  {
      bool msb = ((*byte) & 0x80) != 0;
      *byte = ((*byte) << 1) | (msb ? 0x01 : 0x00);
      m_registers.SetFlag(Registers::Flag_S, ((*byte) & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[*byte]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, msb);
  }

  void CCpu::RLC_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      RLC_reg(&value);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::RLA()
  {
      bool msb = (m_registers.A() & 0x80) != 0;
      m_registers.A() = (m_registers.A() << 1) | (m_registers.GetFlag(Registers::Flag_C) ? 0x01 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, msb);
  }

  void CCpu::RLCA()
  {
      bool msb = (m_registers.A() & 0x80) != 0;
      m_registers.A() = (m_registers.A() << 1) | (msb ? 0x01 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, msb);
  }

  void CCpu::RLD()
  {
      cpcByte x = m_registers.A() & 0x0F;
      cpcByte yz = ReadByteFromMemory(m_registers.HL.w);
      m_registers.A() = (m_registers.A() & 0xF0) | ((yz & 0xF0) >> 4);
      yz = ((yz & 0x0F) << 4) | x;
      WriteByteToMemory(m_registers.HL.w, yz);
      m_registers.SetFlag(Registers::Flag_S, (m_registers.A() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[m_registers.A()]);
      m_registers.SetFlag(Registers::Flag_N, false);
  }

  void CCpu::RR_reg(cpcByte* byte)
  {
      bool lsb = ((*byte) & 0x01) != 0;
      *byte = ((*byte) >> 1) | (m_registers.GetFlag(Registers::Flag_C) ? 0x80 : 0x00);
      m_registers.SetFlag(Registers::Flag_S, ((*byte) & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[*byte]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, lsb);
  }

  void CCpu::RR_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      RR_reg(&value);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::RRC_reg(cpcByte* byte)
  {
      bool lsb = ((*byte) & 0x01) != 0;
      *byte = ((*byte) >> 1) | (lsb ? 0x80 : 0x00);
      m_registers.SetFlag(Registers::Flag_S, lsb);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[*byte]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, lsb);
  }

  void CCpu::RRC_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      RRC_reg(&value);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::RRA()
  {
      bool lsb = (m_registers.A() & 0x01) != 0;
      m_registers.A() = (m_registers.A() >> 1) | (m_registers.GetFlag(Registers::Flag_C) ? 0x80 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, lsb);
  }

  void CCpu::RRCA()
  {
      bool lsb = (m_registers.A() & 0x01) != 0;
      m_registers.A() = (m_registers.A() >> 1) | (lsb ? 0x80 : 0x00);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, lsb);
  }

  void CCpu::RRD()
  {
      cpcByte x = m_registers.A() & 0x0F;
      cpcByte yz = ReadByteFromMemory(m_registers.HL.w);
      m_registers.A() = (m_registers.A() & 0xF0) | (yz & 0x0F);
      yz = (x << 4) | ((yz & 0xF0) >> 4);
      WriteByteToMemory(m_registers.HL.w, yz);
      m_registers.SetFlag(Registers::Flag_S, (m_registers.A() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.A() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[m_registers.A()]);
      m_registers.SetFlag(Registers::Flag_N, false);
  }

  void CCpu::SL_reg(cpcByte* byte, bool bit0)
  {
      bool msb = ((*byte) & 0x80) != 0;
      *byte = ((*byte) << 1) | (bit0 ? 0x01 : 0x00);
      m_registers.SetFlag(Registers::Flag_S, ((*byte) & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[*byte]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, msb);
  }

  void CCpu::SL_addrreg(const Reg16& addressReg, bool bit0)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      SL_reg(&value, bit0);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::SR_reg(cpcByte* byte, bool bit7)
  {
      bool lsb = ((*byte) & 0x01) != 0;
      *byte = ((*byte) >> 1) | (bit7 ? 0x80 : 0x00);
      m_registers.SetFlag(Registers::Flag_S, bit7 != 0);
      m_registers.SetFlag(Registers::Flag_Z, (*byte) == 0);
      m_registers.SetFlag(Registers::Flag_5, ((*byte) & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, ((*byte) & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[*byte]);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, lsb);
  }

  void CCpu::SRA_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      SR_reg(&value, (value & 0x80) != 0);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::SRL_addrreg(const Reg16& addressReg)
  {
      cpcByte value = ReadByteFromMemory(addressReg.w);
      SR_reg(&value, false);
      WriteByteToMemory(addressReg.w, value);
  }

  void CCpu::EX_reg_reg(Reg16* a, Reg16* b)
  {
      std::swap(a->w, b->w);
  }

  void CCpu::EX_addrreg_reg(Reg16 addressReg, Reg16* b)
  {
      Reg16 temp;
      temp.b.l = ReadByteFromMemory(m_registers.SP.w);
      temp.b.h = ReadByteFromMemory(m_registers.SP.w + 1);
      WriteByteToMemory(m_registers.SP.w, b->b.l);
      WriteByteToMemory(m_registers.SP.w + 1, b->b.h);
      *b = temp;
  }

  void CCpu::EXX()
  {
      std::swap(m_registers.BC, m_registers.altBC);
      std::swap(m_registers.DE, m_registers.altDE);
      std::swap(m_registers.HL, m_registers.altHL);
  }

  void CCpu::SCF()
  {
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, true);
  }

  void CCpu::CCF()
  {
      bool oldCarry = m_registers.GetFlag(Registers::Flag_C);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.A() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, oldCarry);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.A() & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_N, false);
      m_registers.SetFlag(Registers::Flag_C, !oldCarry);
  }

  void CCpu::CP_reg(cpcByte b)
  {
      cpcByte result = m_registers.A() - b;
      m_registers.SetFlag(Registers::Flag_S, (result & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, result == 0);
      m_registers.SetFlag(Registers::Flag_5, (b & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, ((m_registers.A() & 0x0F) < (b & 0x0F)));
      m_registers.SetFlag(Registers::Flag_3, (b & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, ((((m_registers.A() ^ b)) & (m_registers.A() ^ result)) & 0x80) != 0);  // Have a and b different sign, and result different sign?
      m_registers.SetFlag(Registers::Flag_N, true);
      m_registers.SetFlag(Registers::Flag_C, m_registers.A() < b);
  }

  void CCpu::CP_n()
  {
      cpcByte b = FetchByte();
      CP_reg(b);
  }

  void CCpu::CP_addrreg(const Reg16& addressReg)
  {
      cpcByte b = ReadByteFromMemory(addressReg.w);
      CP_reg(b);
  }

  void CCpu::CP_addrreg_offset(const Reg16& addressReg)
  {
      cpcByte displacement = FetchByte();
      cpcByte b = ReadByteFromMemory(addressReg.w + ConvertSignedByteToWord(displacement));
      CP_reg(b);
  }

  void CCpu::CPI()
  {
      cpcByte value = ReadByteFromMemory(m_registers.HL.w);
      cpcByte result = m_registers.A() - value;
      m_registers.HL.w++;
      m_registers.BC.w--;

      m_registers.SetFlag(Registers::Flag_S, (result & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, result == 0);
      m_registers.SetFlag(Registers::Flag_H, ((m_registers.A() & 0x0F) < (value & 0x0F)));
      m_registers.SetFlag(Registers::Flag_PV, m_registers.BC.w != 0);
      m_registers.SetFlag(Registers::Flag_N, true);

      cpcByte resultMinusH = result;
      if (m_registers.GetFlag(Registers::Flag_H))
      {
          resultMinusH--;
      }
      m_registers.SetFlag(Registers::Flag_5, (resultMinusH & 0x01) != 0);
      m_registers.SetFlag(Registers::Flag_3, (resultMinusH & 0x04) != 0);
  }

  void CCpu::CPIR()
  {
      // Do a CPI.
      CPI();
      // Repeat if BC != 0.
      if (m_registers.BC.w != 0)
      {
          m_registers.PC.w -= 2;    // Note that CPIR is a 2-byte instruction.
      }
  }

  void CCpu::CPD()
  {
      cpcByte value = ReadByteFromMemory(m_registers.HL.w);
      cpcByte result = m_registers.A() - value;
      m_registers.HL.w--;
      m_registers.BC.w--;

      m_registers.SetFlag(Registers::Flag_S, (result & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, result == 0);
      m_registers.SetFlag(Registers::Flag_H, ((m_registers.A() & 0x0F) < (value & 0x0F)));
      m_registers.SetFlag(Registers::Flag_PV, m_registers.BC.w != 0);
      m_registers.SetFlag(Registers::Flag_N, true);

      cpcByte resultMinusH = result;
      if (m_registers.GetFlag(Registers::Flag_H))
      {
          resultMinusH--;
      }
      m_registers.SetFlag(Registers::Flag_5, (resultMinusH & 0x01) != 0);
      m_registers.SetFlag(Registers::Flag_3, (resultMinusH & 0x04) != 0);
  }

  void CCpu::CPDR()
  {
      // Do a CPD.
      CPD();
      // Repeat if BC != 0.
      if (m_registers.BC.w != 0)
      {
          m_registers.PC.w -= 2;    // Note that CPDR is a 2-byte instruction.
      }
  }

  void CCpu::BIT_reg(int bit, cpcByte value)
  {
      cpcByte result = value & (0x01 << bit);
      m_registers.SetFlag(Registers::Flag_S, (bit == 7) && ((value & 0x80) != 0));
      m_registers.SetFlag(Registers::Flag_Z, result == 0);
      m_registers.SetFlag(Registers::Flag_5, (value & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, true);
      m_registers.SetFlag(Registers::Flag_3, (value & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, result == 0);
      m_registers.SetFlag(Registers::Flag_N, false);
  }

  void CCpu::BIT_addr(int bit, cpcWord address)
  {
      cpcByte value = ReadByteFromMemory(address);
      BIT_reg(bit, value);
  }

  void CCpu::RES_reg(int bit, cpcByte* value)
  {
      *value &= ~(0x01 << bit);
  }

  void CCpu::RES_addr(int bit, cpcWord address)
  {
      cpcByte value = ReadByteFromMemory(address);
      RES_reg(bit, &value);
      WriteByteToMemory(address, value);
  }

  void CCpu::SET_reg(int bit, cpcByte* value)
  {
      *value |= (0x01 << bit);
  }

  void CCpu::SET_addr(int bit, cpcWord address)
  {
      cpcByte value = ReadByteFromMemory(address);
      SET_reg(bit, &value);
      WriteByteToMemory(address, value);
  }

  void CCpu::PUSH(const Reg16& value)
  {
      Push(value);
  }

  void CCpu::POP(Reg16* value)
  {
      Pop(value);
  }

  void CCpu::CALL_nn()
  {
      Reg16 newPC;
      newPC.b.l = FetchByte();
      newPC.b.h = FetchByte();
      // Push current PC value onto stack.
      Push(m_registers.PC);
      // Set new PC value.
      m_registers.PC = newPC;
  }

  void CCpu::CALL_condition_nn(bool condition)
  {
      Reg16 newPC;
      newPC.b.l = FetchByte();
      newPC.b.h = FetchByte();
      if (condition)
      {
          // Push current PC value onto stack.
          Push(m_registers.PC);
          // Set new PC value.
          m_registers.PC = newPC;
      }
  }

  void CCpu::RST_p(cpcByte p)
  {
      // Push current PC value onto stack.
      Push(m_registers.PC);
      // Set new PC value.
      m_registers.PC.b.h = 0;
      m_registers.PC.b.l = p;
  }

  void CCpu::RET()
  {
      Pop(&m_registers.PC);
  }

  void CCpu::RET_condition(bool condition)
  {
      if (condition)
      {
          Pop(&m_registers.PC);
      }
  }

  void CCpu::RETN()
  {
      Pop(&m_registers.PC);
      m_registers.IFF1 = m_registers.IFF2;
  }

  void CCpu::JP_reg(const Reg16& address)
  {
      m_registers.PC = address;
  }

  void CCpu::JP_nn()
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = FetchByte();
      m_registers.PC = address;
  }

  void CCpu::JP_condition_nn(bool condition)
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = FetchByte();
      if (condition)
      {
          m_registers.PC = address;
      }
  }

  void CCpu::JR_n()
  {
      cpcByte displacement = FetchByte();
      m_registers.PC.w += ConvertSignedByteToWord(displacement);
  }

  void CCpu::JR_condition_n(bool condition)
  {
      cpcByte displacement = FetchByte();
      if (condition)
      {
          m_registers.PC.w = m_registers.PC.w + ConvertSignedByteToWord(displacement);
      }
  }

  void CCpu::DJNZ_n()
  {
      cpcByte displacement = FetchByte();
      m_registers.B()--;
      if (m_registers.B() != 0)
      {
          m_registers.PC.w = m_registers.PC.w + ConvertSignedByteToWord(displacement);
      }
  }

  void CCpu::IN_value_address(cpcByte* value, const Reg16& addressReg)
  {
      cpcByte a = ReadByteFromPort(addressReg.w);
      if (value != NULL)
      {
          *value = a;
      }
      m_registers.SetFlag(Registers::Flag_S, (a & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, a == 0);
      m_registers.SetFlag(Registers::Flag_5, (a & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_H, false);
      m_registers.SetFlag(Registers::Flag_3, (a & 0x08) != 0);
      m_registers.SetFlag(Registers::Flag_PV, s_parity[a]);
      m_registers.SetFlag(Registers::Flag_N, false);
  }

  void CCpu::IN_n()
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = m_registers.A();
      m_registers.A() = ReadByteFromPort(address.w);
  }

  void CCpu::INI()
  {
      cpcByte value = ReadByteFromPort(m_registers.BC.w);
      WriteByteToMemory(m_registers.HL.w, value);
      m_registers.B()--;
      m_registers.HL.w++;

      m_registers.SetFlag(Registers::Flag_S, (m_registers.B() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.B() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.B() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.B() & 0x08) != 0);
  }

  void CCpu::INIR()
  {
      // Do a INI.
      INI();
      // Repeat if B != 0.
      if (m_registers.B() != 0)
      {
          m_registers.PC.w -= 2;    // Note that INIR is a 2-byte instruction.
      }
  }

  void CCpu::IND()
  {
      cpcByte value = ReadByteFromPort(m_registers.BC.w);
      WriteByteToMemory(m_registers.HL.w, value);
      m_registers.B()--;
      m_registers.HL.w--;

      m_registers.SetFlag(Registers::Flag_S, (m_registers.B() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.B() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.B() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.B() & 0x08) != 0);
  }

  void CCpu::INDR()
  {
      // Do a IND.
      IND();
      // Repeat if B != 0.
      if (m_registers.B() != 0)
      {
          m_registers.PC.w -= 2;    // Note that INDR is a 2-byte instruction.
      }
  }

  void CCpu::OUT_address_value(const Reg16& addressReg, cpcByte value)
  {
      WriteByteToPort(addressReg.w, value);
  }

  void CCpu::OUT_n()
  {
      Reg16 address;
      address.b.l = FetchByte();
      address.b.h = m_registers.A();
      WriteByteToPort(address.w, m_registers.A());
  }

  void CCpu::OUTI()
  {
      cpcByte value = ReadByteFromMemory(m_registers.HL.w);
      m_registers.B()--;
      WriteByteToPort(m_registers.BC.w, value);
      m_registers.HL.w++;

      m_registers.SetFlag(Registers::Flag_S, (m_registers.B() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.B() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.B() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.B() & 0x08) != 0);
  }

  void CCpu::OTIR()
  {
      // Do a OUTI.
      OUTI();
      // Repeat if B != 0.
      if (m_registers.B() != 0)
      {
          m_registers.PC.w -= 2;    // Note that OTIR is a 2-byte instruction.
      }
  }

  void CCpu::OUTD()
  {
      cpcByte value = ReadByteFromMemory(m_registers.HL.w);
      m_registers.B()--;
      WriteByteToPort(m_registers.BC.w, value);
      m_registers.HL.w--;

      m_registers.SetFlag(Registers::Flag_S, (m_registers.B() & 0x80) != 0);
      m_registers.SetFlag(Registers::Flag_Z, m_registers.B() == 0);
      m_registers.SetFlag(Registers::Flag_5, (m_registers.B() & 0x20) != 0);
      m_registers.SetFlag(Registers::Flag_3, (m_registers.B() & 0x08) != 0);
  }

  void CCpu::OTDR()
  {
      // Do a OUTD.
      OUTD();
      // Repeat if B != 0.
      if (m_registers.B() != 0)
      {
          m_registers.PC.w -= 2;    // Note that OTDR is a 2-byte instruction.
      }
  }

  void CCpu::EI()
  {
      m_registers.IFF1 = true;
      m_registers.IFF2 = true;
      m_delayInterruptEnable = true;
  }

  void CCpu::DI()
  {
      m_registers.IFF1 = false;
      m_registers.IFF2 = false;
  }

  void CCpu::IM(int mode)
  {
      m_registers.IM = mode;
  }

  void CCpu::HALT()
  {
      m_inHalt = true;
  }

} //namespace CPC
