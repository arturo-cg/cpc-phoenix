//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcFdc.h"
#include "cpcMachine.h"



namespace CPC {


  struct SCommandInfo
  {
    unsigned nParameterCount;
    unsigned nResultCount;
  };

  static const SCommandInfo s_aCommandInfos[] =
  {
    { 8, 7 },  /*COMMAND_READ_TRACK*/
    { 2, 0 },  /*COMMAND_SPECIFY_SPD_DMA*/
    { 1, 1 },  /*COMMAND_SENSE_DRIVE_STATE*/
    { 8, 7 },  /*COMMAND_WRITE_SECTORS*/
    { 8, 7 },  /*COMMAND_READ_SECTORS*/
    { 1, 0 },  /*COMMAND_RECALIBRATE_SEEK*/
    { 0, 2 },  /*COMMAND_SENSE_INT_STATE*/
    { 8, 7 },  /*COMMAND_WRITE_DELETED_SECTORS*/
    { 1, 7 },  /*COMMAND_READ_ID*/
    { 8, 7 },  /*COMMAND_READ_DELETED_SECTORS*/
    { 5, 7 },  /*COMMAND_FORMAT_TRACK*/
    { 2, 0 },  /*COMMAND_SEEK_TRACK_N*/
    { 8, 7 },  /*COMMAND_SCAN_EQUAL*/
    { 8, 7 },  /*COMMAND_SCAN_LOW_OR_EQUAL*/
    { 8, 7 },  /*COMMAND_SCAN_HIGH_OR_EQUAL*/
  };


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CFdc::CFdc(CMachine *pMachine) : inherited( pMachine )
  {
    // Reset members
    ResetVars();

    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CFdc::ResetVars()
  {
    m_eCurrentPhase   = PHASE_COMMAND;
    m_eCurrentCommand = COMMAND_INVALID;
    m_nCurrentIndex   = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CFdc::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CFdc::Reset()
  {
    ResetVars();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ bool CFdc::RespondToReadPortRequest(cpcWord nPort, cpcByte* pnValue)
  {
    bool bRet = false;

    //
    // 765 FDC port --> Bits 10,7 == 0
    //

    // 765 FDC port?
    if ( !(nPort & 0x0480) )
    {
      //
      // Bits 8,0 of nPort define the specific register.
      //

      int nRegister;
      nRegister = ((nPort & 0x0100) >> 7) | (nPort & 0x0001);
      switch (nRegister)
      {
        case 2:   *pnValue = ReadMainStatusRegister(); break;
        case 3:   *pnValue = ReadDataRegister(); break;
        default:  *pnValue = 0xFF;
      }

      bRet = true;       // Indicates the device has responded to the port read request.
    }

    return bRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CFdc::RespondToWritePortRequest(cpcWord nPort, cpcByte nValue)
  {
    //
    // 765 FDC port --> Bits 10,7 == 0
    //

    // 765 FDC port?
    if ( !(nPort & 0x0480) )
    {
      //
      // Bits 8,0 of nPort define the specific register.
      //

      int nRegister;
      nRegister = ((nPort & 0x0100) >> 7) | (nPort & 0x0001);
      switch (nRegister)
      {
        case 0:   TurnMotorOn( (nValue&0x0001) ? true : false ); break;
        case 1:   TurnMotorOn( (nValue&0x0001) ? true : false ); break;
        case 2:   WriteDataRegister( nValue ); break;
        case 3:   WriteDataRegister( nValue ); break;
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CFdc::TurnMotorOn(bool bOn)
  {
    // We just ignore it in the emulator
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CFdc::ReadMainStatusRegister() const
  {
    cpcByte nRet = 0;


    return nRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CFdc::ReadDataRegister() const
  {
    cpcByte nRet = 0;

    // The Data Register should be read during Result Phase only.
    // In this phase, result bytes written by the last executed command to m_anResult are sent to the CPU.
    if (m_eCurrentCommand == PHASE_RESULT)
    {
      //...
    }

    return nRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CFdc::WriteDataRegister(cpcByte nValue)
  {
    if (m_eCurrentPhase == PHASE_COMMAND)
    {
      //
      // In this phase, the command and its parameters are gathered from the CPU and written to m_anParameters.
      //

      // The first byte written is the command.
      // Remaining bytes written are the parameters
      if (m_eCurrentCommand == COMMAND_INVALID)
      {
        m_eCurrentCommand = (ECommand) (nValue & 0x1F);
        m_nCurrentIndex   = 0;
      }
      else
      {
        m_anParameters[m_nCurrentIndex] = nValue;
        m_nCurrentIndex++;

        // If all parameters for the selected command have been written, execute the command
        if (m_nCurrentIndex >= s_aCommandInfos[m_eCurrentCommand].nParameterCount)    // If this was the last parameter...
        {
          // Execute the command
          m_eCurrentPhase = PHASE_EXECUTION;
          m_nCurrentIndex = 0;

          switch (m_eCurrentCommand)
          {
          }
        }
      }
    }
  }

} //namespace CPC
