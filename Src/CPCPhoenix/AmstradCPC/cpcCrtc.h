//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCCRTC_H_
#define _CPCCRTC_H_


#include "cpcSubSystem.h"


namespace CPC {


  /**
  ** Emulates the Motorola 6845 CRTC. This chip along with the Gate-Array are responsible
  ** for generating the correct video signal that is sent to the Amstrad monitor.
  ** @par
  ** The CRTC was a commercial chip that was used in several machines including the Amstrad.
  ** Its goal is 'simply' to generate memory addresses and HSYNC/VSYNC signals. The Gate-Array
  ** then reads the memory at these addresses and generates the video signal.
  ** @par
  ** The CRTC has several internal registers to configure the addresses it generates. However,
  ** only two are exposed to other components: one to select the internal register and other
  ** to read/write to the selected internal register.
  */
  class CCrtc : public CSubSystem
  {
  public:

    CCrtc                     (CMachine *pMachine);
    virtual                ~CCrtc                     ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** We are notified that another subsytem is trying to write a byte to us.
    *** Usually it's the CPU through an OUT instruction. */
    virtual void            RespondToWritePortRequest (cpcWord nPort, cpcByte nValue);

    /** Runs the CRTC for the given number of cycles. */
    void                    Run                       (unsigned nMinNumCycles);


  private:

    typedef                 CSubSystem                inherited;


    enum TRegister
    {
      HORIZONTAL_TOTAL = 0,
      HORIZONTAL_DISPLAYED = 1,
      HORIZONTAL_SYNC_POSITION = 2,
      SYNC_WIDTHS = 3,
      VERTICAL_TOTAL = 4,
      VERTICAL_TOTAL_ADJUST = 5,
      VERTICAL_DISPLAYED = 6,
      VERTICAL_SYNC_POSITION = 7,
      INTERLACE_MODE_AND_SKEW = 8,
      MAXIMUM_RASTER_ADDRESS = 9,
      CURSOR_START_RASTER = 10,
      CURSOR_END_RASTER = 11,
      START_ADDRESS_HIGH = 12,
      START_ADDRESS_LOW = 13,
      CURSOR_HIGH = 14,
      CURSOR_LOW = 15,
      LIGHT_PEN_HIGH = 16,
      LIGHT_PEN_LOW = 17,

      NUM_REGISTERS,
      INVALID_REGISTER = 0xFFFFFFFF
    };


    void                    ResetVars                 ();
    void                    FreeVars                  ();


    /** The internal control registers. */
    cpcByte                 m_anRegisters[NUM_REGISTERS];

  };


} //namespace CPC

#endif // _CPCCRTC_H_
