//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

#include "cpcCpuInterface.h"


namespace CPC {

  class CMachine;

  /**
  ** A concrete implementation of CCpuInterface for the Amstrad CPC.
  */
  class CCpuToCpcInterface : public CCpuInterface
  {
  public:

    CCpuToCpcInterface(CMachine* machine);
    virtual ~CCpuToCpcInterface()  { }

    /** The Z80 is being reset. */
    virtual void OnReset(CCpu* cpu) override;
    /** The Z80 is starting a new T cycle, which happens at the rising edge of the clock. */
    virtual void OnTCycle(CCpu* cpu) override;

    /** The Z80 requests to read a byte from memory. */
    virtual cpcByte ReadByteFromMemory(CCpu* cpu, cpcWord address) override;
    /** The Z80 requests to write a byte to memory. */
    virtual void WriteByteToMemory(CCpu* cpu, cpcWord address, cpcByte value) override;
    /** The Z80 requests to read a byte from a port. */
    virtual cpcByte ReadByteFromPort(CCpu* cpu, cpcWord port) override;
    /** The Z80 requests to write a byte to a port. */
    virtual void WriteByteToPort(CCpu* cpu, cpcWord port, cpcByte value) override;


  private:

    CMachine* m_machine;

  };


} //namespace CPC
