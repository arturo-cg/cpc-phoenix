//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCFDC_H_
#define _CPCFDC_H_


#include "cpcSubSystem.h"


namespace CPC {


  /**
  ** The 765 FDC (Floppy Disk Controller).
  */
  class CFdc : public CSubSystem
  {
  public:

                            CFdc                      (CMachine *pMachine);
    virtual                ~CFdc                      ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** We are notified that the machine is trying to read a byte from this subsystem. */
    virtual bool            RespondToReadPortRequest  (cpcWord nPort, cpcByte* pnValue);
    /** We are notified that the machine is trying to write a byte to this subsystem. */
    virtual void            RespondToWritePortRequest (cpcWord nPort, cpcByte nValue);


  private:

    typedef                 CSubSystem                inherited;

    enum EPhase
    {
      PHASE_COMMAND = 0,      // Receiving command byte and parameters from the CPU.
      PHASE_EXECUTION,        // Transferring data from/to the disk.
      PHASE_RESULT,           // Returning status information to the CPU.

      PHASE_LAST,
      PHASE_INVALID = 0xFFFFFFFF
    };

    enum ECommand
    {
      COMMAND_READ_TRACK = 0x02,
      COMMAND_SPECIFY_SPD_DMA = 0x03,
      COMMAND_SENSE_DRIVE_STATE = 0x04,
      COMMAND_WRITE_SECTORS = 0x05,
      COMMAND_READ_SECTORS = 0x06,
      COMMAND_RECALIBRATE_SEEK = 0x07,
      COMMAND_SENSE_INT_STATE = 0x08,
      COMMAND_WRITE_DELETED_SECTORS = 0x09,
      COMMAND_READ_ID = 0x0A,
      COMMAND_READ_DELETED_SECTORS = 0x0C,
      COMMAND_FORMAT_TRACK = 0x0D,
      COMMAND_SEEK_TRACK_N = 0x0F,
      COMMAND_SCAN_EQUAL = 0x11,
      COMMAND_SCAN_LOW_OR_EQUAL = 0x19,
      COMMAND_SCAN_HIGH_OR_EQUAL = 0x1D,

      COMMAND_INVALID = 0xFFFFFFFF,
    };

    static const unsigned   MAX_PARAMETER_COUNT = 8;
    static const unsigned   MAX_RESULT_COUNT = 7;


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    void                    TurnMotorOn               (bool bOn);
    cpcByte                 ReadMainStatusRegister    () const;
    cpcByte                 ReadDataRegister          () const;
    void                    WriteDataRegister         (cpcByte nValue);


    EPhase                  m_eCurrentPhase;
    ECommand                m_eCurrentCommand;
    cpcByte                 m_anParameters[1 + MAX_PARAMETER_COUNT];   // The first byte is the command byte.
    cpcByte                 m_anResult[MAX_PARAMETER_COUNT];
    unsigned                m_nCurrentIndex;

  };


} //namespace CPC

#endif // _CPCFDC_H_
