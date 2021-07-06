//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

#include "cpcTypes.h"


namespace CPC {

  class CCpu;

  /**
  ** The CPU interface to the rest of the machine. This allows the CPU emulation to be machine-agnostic and therefore be reusable on other emulators.
  ** You must implement a class that inherits from this one and assign it to your CCpu object by calling the method CCpu::SetCpuInterface.
  */
  class CCpuInterface
  {
  public:

    virtual ~CCpuInterface()  { }

    /** The Z80 is being reset. */
    virtual void OnReset(CCpu* cpu) = 0;
    /** The Z80 is starting a new T state (i.e. clock cycle), which happens at the rising edge of the clock. */
    virtual void OnTState(CCpu* cpu) = 0;
    /** The Z80 just accepted the interrupt request. */
    virtual void OnInterruptAcknowledge(CCpu* cpu) = 0;

    /** The Z80 requests to read a byte from memory. */
    virtual cpcByte ReadByteFromMemory(const CCpu* cpu, cpcWord address) const = 0;
    /** The Z80 requests to write a byte to memory. */
    virtual void WriteByteToMemory(const CCpu* cpu, cpcWord address, cpcByte value) = 0;
    /** The Z80 requests to read a byte from a port. */
    virtual cpcByte ReadByteFromPort(const CCpu* cpu, cpcWord port) const = 0;
    /** The Z80 requests to write a byte to a port. */
    virtual void WriteByteToPort(const CCpu* cpu, cpcWord port, cpcByte value) = 0;


  private:

  };


} //namespace CPC
