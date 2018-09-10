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

    /** The Z80 requests to read a byte from memory. */
    virtual cpcByte ReadByteFromMemory(CCpu* cpu, cpcWord address) = 0;
    /** The Z80 requests to write a byte to memory. */
    virtual void WriteByteToMemory(CCpu* cpu, cpcWord address, cpcByte value) = 0;
    /** The Z80 requests to read a byte from a port. */
    virtual cpcByte ReadByteFromPort(CCpu* cpu, cpcWord port) = 0;
    /** The Z80 requests to write a byte to a port. */
    virtual void WriteByteToPort(CCpu* cpu, cpcWord port, cpcByte value) = 0;


  private:

  };


} //namespace CPC
