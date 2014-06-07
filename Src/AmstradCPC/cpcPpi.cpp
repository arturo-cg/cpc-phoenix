//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcPpi.h"
#include "cpcMachine.h"
#include "cpcPsg.h"
#include "cpcCrtc.h"
#include "cpcKeyboard.h"



namespace CPC {


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CPpi::CPpi(CMachine *pMachine) : inherited( pMachine )
  {
    // Reset members
    ResetVars();

    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CPpi::ResetVars()
  {
    m_aePortDirections[PORT_A]       = DIRECTION_OUTPUT;
    m_aePortDirections[PORT_B]       = DIRECTION_INPUT;
    m_aePortDirections[PORT_C_UPPER] = DIRECTION_OUTPUT;
    m_aePortDirections[PORT_C_LOWER] = DIRECTION_OUTPUT;

    m_anPortOutputValue[PORT_A]       = 0;
    m_anPortOutputValue[PORT_B]       = 0;
    m_anPortOutputValue[PORT_C_UPPER] = 0;
    m_anPortOutputValue[PORT_C_LOWER] = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CPpi::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CPpi::Reset()
  {
    ResetVars();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ bool CPpi::RespondToReadPortRequest(cpcWord nPort, cpcByte* pnValue)
  {
    bool bRet = false;

    //
    // 8255 PPI port --> Bit 11 == 0
    //

    // 8255 PPI port?
    if ( !(nPort & 0x0800) )
    {
      //
      // Bits 9,8 of nPort define the PPI port to access (A, B, C or Control).
      // Note: 8255 PPI ports should not be confused with CPU ports that are accessed with IN & OUT instructions.
      //

      int nFunction;
      nFunction = (nPort & 0x0300) >> 8;
      switch (nFunction)
      {
        case 0:  *pnValue = ReadPortA(); break;     // Read PPI port A
        case 1:  *pnValue = ReadPortB(); break;     // Read PPI port B
        case 2:  *pnValue = ReadPortC(); break;     // Read PPI port C
        case 3:  /*EMPTY*/               break;     // PPI control word is write-only
      }

      bRet = true;       // Indicates the device has responded to the port read request.
    }

    return bRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CPpi::RespondToWritePortRequest(cpcWord nPort, cpcByte nValue)
  {
    //
    // 8255 PPI port --> Bit 11 == 0
    //

    // 8255 PPI port?
    if ( !(nPort & 0x0800) )
    {
      //
      // Bits 9,8 of nPort define the PPI port to access (A, B, C or Control).
      // Note: 8255 PPI ports should not be confused with CPU ports that are accessed with IN & OUT instructions.
      //

      int nFunction;
      nFunction = (nPort & 0x0300) >> 8;
      switch (nFunction)
      {
        case 0:  WritePortA( nValue ); break;        // Write PPI port A
        case 1:  WritePortB( nValue ); break;        // Write PPI port B
        case 2:  WritePortC( nValue ); break;        // Write PPI port C
        case 3:  WriteControlWord( nValue ); break;  // Write PPI control word
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CPpi::ReadPortA()
  {
    cpcByte nRet = 0;

    KMASSERTM( m_aePortDirections[PORT_A] == DIRECTION_INPUT, ("Trying to read from PPI port A when it is currently configured as OUTPUT.") );
    if (m_aePortDirections[PORT_A] == DIRECTION_INPUT)
    {
      // All 8 bits are connected to the PSG data bus
      nRet = GetMachine()->GetPsg()->GetSelectedRegisterValue();
    }

    return nRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CPpi::WritePortA(cpcByte nValue)
  {
    KMASSERTM( m_aePortDirections[PORT_A] == DIRECTION_OUTPUT, ("Trying to write to PPI port A when it is currently configured as INPUT.") );
    if (m_aePortDirections[PORT_A] == DIRECTION_OUTPUT)
    {
      // Save the value into the internal register
      m_anPortOutputValue[PORT_A] = nValue;

      // All 8 bits are connected to the PSG data bus

      // TODO
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CPpi::ReadPortB()
  {
    cpcByte nRet = 0;

    KMASSERTM( m_aePortDirections[PORT_B] == DIRECTION_INPUT, ("Trying to read from PPI port B when it is currently configured as OUTPUT.") );
    if (m_aePortDirections[PORT_B] == DIRECTION_INPUT)
    {
      cpcByte nVSyncState;
      nVSyncState = ( GetMachine()->GetCrtc()->GetVSyncState() ? 1 : 0 );

      nRet = (0 << 7) |     // Bit 7 --> Cassette read data. No cassette emulation for now.
             (1 << 6) |     // Bit 6 --> Parallel/Printer port ready signal ("1" = not ready, "0" = Ready). No parallel port emulation.
             (0 << 5) |     // Bit 5 --> Expansion device connected signal. No expansion device emulation.
             (1 << 4) |     // Bit 4 --> Screen refresh frequency ("1" = 50Hz, "0" = 60Hz).
             (7 << 1) |     // Bits 3-1 --> Manufacturer name ("7" = Amstrad).
             (nVSyncState); // Bit 0 --> VSYNC state of VSYNC signal from the CRTC ("1" = VSYNC active, "0" = VSYNC inactive).
    }

    return nRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CPpi::WritePortB(cpcByte nValue)
  {
    KMASSERTM( m_aePortDirections[PORT_B] == DIRECTION_OUTPUT, ("Trying to write to PPI port B when it is currently configured as INPUT.") );
    if (m_aePortDirections[PORT_B] == DIRECTION_OUTPUT)
    {
      // Save the value into the internal register
      m_anPortOutputValue[PORT_B] = nValue;
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CPpi::ReadPortC()
  {
    cpcByte nRet = 0;

    KMASSERTM( (m_aePortDirections[PORT_C_UPPER] == DIRECTION_INPUT) && (m_aePortDirections[PORT_C_LOWER] == DIRECTION_INPUT),
               ("Trying to read from PPI port C when it is currently configured as OUTPUT.") );

    if ( (m_aePortDirections[PORT_C_UPPER] == DIRECTION_OUTPUT) && (m_aePortDirections[PORT_C_LOWER] == DIRECTION_OUTPUT) )
    {
      // The port is configured as output, so when a read operation gives the value of the port internal output register
      nRet = m_anPortOutputValue[PORT_C];
    }

    return nRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CPpi::WritePortC(cpcByte nValue)
  {
    KMASSERTM( (m_aePortDirections[PORT_C_UPPER] == DIRECTION_OUTPUT) && (m_aePortDirections[PORT_C_LOWER] == DIRECTION_OUTPUT),
               ("Trying to write to PPI port C when it is currently configured as INPUT.") );
    if (m_aePortDirections[PORT_C_UPPER] == DIRECTION_OUTPUT)
    {
      // Save the value into the internal register, preserving lower bits
      m_anPortOutputValue[PORT_C] = (nValue & 0xF0) | (m_anPortOutputValue[PORT_C] & 0x0F);

      // Bit 7 --> PSG BDIR
      // Bit 6 --> PSG BC1
      CPsg::EFunction ePsgFunction;
      ePsgFunction = (CPsg::EFunction) ((nValue & 0xC0) >> 6);
      GetMachine()->GetPsg()->SelectFunction( ePsgFunction );

      // Bit 5 --> Cassette Write data
      // Bit 4 --> Cassette Motor control
      // TODO
    }

    if (m_aePortDirections[PORT_C_LOWER] == DIRECTION_OUTPUT)
    {
      // Save the value into the internal register, preserving upper bits
      m_anPortOutputValue[PORT_C] = (m_anPortOutputValue[PORT_C] & 0xF0) | (nValue & 0x0F);

      // Bits 3-0 --> Keyboard line to be scanned
      GetMachine()->GetKeyboard()->SetSelectedLine( nValue & 0x0F );
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CPpi::WriteControlWord(cpcByte nValue)
  {
    int        nMode;
    EDirection eDirection;

    // When bit 7 == "1" --> Defines mode and directions (input or output) of each port.
    // When bit 7 == "0" --> Sets or clears a single bit of port C.
    if (nValue & 0x80)      // If bit 7 is "1"...
    {
      // Bits 6,5 --> Group A mode (0=mode 0, 1=mode 1, 2 and 3=mode 2. Only mode 0 is emulated)
      nMode = (nValue & 0x60) >> 5;
      KMASSERTM( nMode == 0, ("The mode selected for PPI group A is not emulated.") );

      // Bit 4 --> Direction of port A (0=output, 1=input)
      eDirection = ( (nValue & 0x10) ? DIRECTION_INPUT : DIRECTION_OUTPUT );
      m_aePortDirections[PORT_A] = eDirection;

      // Bit 3 --> Direction of upper 4 bits of port C (0=output, 1=input)
      eDirection = ( (nValue & 0x08) ? DIRECTION_INPUT : DIRECTION_OUTPUT );
      m_aePortDirections[PORT_C_UPPER] = eDirection;

      // Bit 2 --> Group B mode (0=mode 0, 1=mode 1. Only mode 0 is emulated)
      nMode = (nValue & 0x04) >> 2;
      KMASSERTM( nMode == 0, ("The mode selected for PPI group B is not emulated.") );

      // Bit 1 --> Direction of port B (0=output, 1=input)
      eDirection = ( (nValue & 0x02) ? DIRECTION_INPUT : DIRECTION_OUTPUT );
      m_aePortDirections[PORT_B] = eDirection;

      // Bit 0 --> Direction of lower 4 bits of port C (0=output, 1=input)
      eDirection = ( (nValue & 0x01) ? DIRECTION_INPUT : DIRECTION_OUTPUT );
      m_aePortDirections[PORT_C_LOWER] = eDirection;

      // Every time the control word is written, port internal registers are cleared
      m_anPortOutputValue[PORT_A] = 0x00;
      m_anPortOutputValue[PORT_B] = 0x00;
      m_anPortOutputValue[PORT_C] = 0x00;
    }
    else
    {
      // Bits 3-1 indicate the bit to select of port C
      int nBit;
      nBit = (nValue & 0x1E) >> 1;

      // Bit 0 indicates the new value for the selected bit
      if (nValue & 0x01)
      {
        m_anPortOutputValue[PORT_C] = m_anPortOutputValue[PORT_C] | (1 << nBit);    // Set the bit
      }
      else
      {
        m_anPortOutputValue[PORT_C] = m_anPortOutputValue[PORT_C] & !(1 << nBit);   // Clear the bit
      }
    }
  }

} //namespace CPC
