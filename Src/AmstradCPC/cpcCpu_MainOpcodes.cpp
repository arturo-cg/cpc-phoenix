#include "stdafx.h"
#include "cpcCpu.h"


namespace CPC {

  CCpu::OpcodeInfo CCpu::m_opcodesMain[256] = {
    { &CCpu::Execute_00, false, "NOP" },
    { &CCpu::Execute_01, false, "LD BC, %nn" },
    { &CCpu::Execute_02, false, "LD (BC), A" },
    { &CCpu::Execute_03, false, "INC BC" },
    { &CCpu::Execute_04, false, "INC B" },
    { &CCpu::Execute_05, false, "DEC B" },
    { &CCpu::Execute_06, false, "LD B, %n" },
    { &CCpu::Execute_07, false, "RLCA" },
    { &CCpu::Execute_08, false, "EX AF, AF'" },
    { &CCpu::Execute_09, false, "ADD HL, BC" },
    { &CCpu::Execute_0A, false, "LD A,(BC)" },
    { &CCpu::Execute_0B, false, "DEC BC" },
    { &CCpu::Execute_0C, false, "INC C" },
    { &CCpu::Execute_0D, false, "DEC C" },
    { &CCpu::Execute_0E, false, "LD C, %n" },
    { &CCpu::Execute_0F, false, "RRCA" },
  };

  void CCpu::Execute_00()
  {
      // Do nothing.
  }

  void CCpu::Execute_01()
  {
      LD16_reg_nn(&m_registers.BC);
  }

  void CCpu::Execute_02()
  {
      LD8_addrreg_valuereg(m_registers.BC, m_registers.A());
  }

  void CCpu::Execute_03()
  {
      INC16_reg(&m_registers.BC);
  }

  void CCpu::Execute_04()
  {
      INC8_reg(&m_registers.B());
  }

  void CCpu::Execute_05()
  {
      DEC8_reg(&m_registers.B());
  }

  void CCpu::Execute_06()
  {
      LD8_reg_n(&m_registers.B());
  }

  void CCpu::Execute_07()
  {
      RLC(&m_registers.A());
  }

  void CCpu::Execute_08()
  {
      EX_reg_reg(&m_registers.AF, &m_registers.altAF);
  }

  void CCpu::Execute_09()
  {
      ADD16_reg_reg(&m_registers.HL, m_registers.BC);
  }

  void CCpu::Execute_0A()
  {
      LD8_reg_mem(&m_registers.A(), m_registers.BC);
  }

  void CCpu::Execute_0B()
  {
      DEC16_reg(&m_registers.BC);
  }

  void CCpu::Execute_0C()
  {
      INC8_reg(&m_registers.C());
  }

  void CCpu::Execute_0D()
  {
      DEC8_reg(&m_registers.C());
  }

  void CCpu::Execute_0E()
  {
      LD8_reg_n(&m_registers.C());
  }

  void CCpu::Execute_0F()
  {
      RRC(&m_registers.A());
  }

} //namespace CPC
