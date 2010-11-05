//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCPPI_H_
#define _CPCPPI_H_


#include "cpcSubSystem.h"


namespace CPC {


  /**
  ** The 8255 PPI is a general purpose programmable I/O device. In the Amstrad CPC, the following devices
  ** are connected and accessed through the 8255 PPI: AY-3-8912 PSG, keyboard, cassette, VSYNC signal from
  ** the CRTC and the BUSY signal from the parallel port.
  **
  ** The CPC makes a very simple use of the 8255 PPI. We take advantage of this fact to greatly simplify its
  ** emulation. These are the simplifications made (as the CPC firmware itself does):
  **   - There are three 8-bit ports: A, B and C (actually, port C can be treated as two 4-bit ports).
  **   - All ports are always in mode 0 (the 8255 PPI allows modes 1 and 2 also).
  */
  class CPpi : public CSubSystem
  {
  public:

                            CPpi                   (CMachine *pMachine);
    virtual                ~CPpi                   ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** We are notified that the machine is trying to read a byte from this subsystem. */
    virtual bool            RespondToReadPortRequest  (cpcWord nPort, cpcByte* pnValue);
    /** We are notified that the machine is trying to write a byte to this subsystem. */
    virtual void            RespondToWritePortRequest (cpcWord nPort, cpcByte nValue);


  private:

    // 8255 PPI ports. Note: Not to be confused with CPU ports that are accessed with IN & OUT instructions.
    enum EPort
    {
      PORT_A = 0,
      PORT_B,
      PORT_C,

      PORT_C_UPPER = PORT_C,    // Note that it has the same value as PORT_C.
      PORT_C_LOWER,

      PORT_LAST,
      PORT_INVALID = 0x7FFFFFFF
    };

    enum EDirection
    {
      DIRECTION_OUTPUT = 0,     // Data goes from the data bus (i.e. CPU) to the PPI port.
      DIRECTION_INPUT,          // Data goes from the PPI port to the data bus (i.e. CPU).
    };

    typedef                 CSubSystem                inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    void                    WriteControlWord          (cpcByte nValue);
    void                    WritePortA                (cpcByte nValue);
    void                    WritePortB                (cpcByte nValue);
    void                    WritePortC                (cpcByte nValue);
    cpcByte                 ReadPortA                 ();
    cpcByte                 ReadPortB                 ();
    cpcByte                 ReadPortC                 ();


    EDirection              m_aePortDirections[PORT_LAST];

  };


} //namespace CPC

#endif // _CPCPPI_H_
