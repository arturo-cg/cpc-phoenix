//-------------------------------------------------------------------------------------------
// File:        Crtc.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcCrtc.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"


namespace CPC {


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CCrtc::CCrtc(CMachine *pMachine) : inherited( pMachine )
  {
    Reset();
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CCrtc::ResetVars()
  {
    m_eSelectedRegister  = HORIZONTAL_TOTAL;
    m_nCurrentHCharacter = 0;
    m_nCurrentVCharacter = 0;
    m_nCurrentScanLine   = 0;
    m_nScanLinesForVSyncOff = 0;
    m_bDisplayEnabled    = true;
    m_bHSyncState        = false;
    m_bVSyncState        = false;
    m_bGeneratedAddressTableUpToDate = false;

    for (int i = 0; i < NUM_REGISTERS; i++)
    {
      m_anRegisters[i] = 0;
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CCrtc::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CCrtc::Reset()
  {
    // Reset members
    ResetVars();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::SelectRegister(ERegister eRegister)
  {
    if (eRegister != INVALID_REGISTER)
    {
      m_eSelectedRegister = eRegister;
      m_bGeneratedAddressTableUpToDate = false;      // TODO - Invalid cached table only when writing to a register that it depends on.
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::WriteSelectedRegister(cpcByte nValue)
  {
    KMASSERT( m_eSelectedRegister < NUM_REGISTERS );
    m_anRegisters[m_eSelectedRegister] = nValue;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::ComputeGeneratedAddressTable()
  {
    unsigned nScanLine = 0;

    cpcWord currentMA;
    currentMA = (m_anRegisters[START_ADDRESS_HIGH] << 8) | m_anRegisters[START_ADDRESS_LOW];

    unsigned nCharacterLine;
    for (nCharacterLine = 0; nCharacterLine < m_anRegisters[VERTICAL_DISPLAYED]; nCharacterLine++)
    {
      cpcByte RA;
      for (RA = 0; RA < (m_anRegisters[MAXIMUM_RASTER_ADDRESS] + 1); RA++)
      {
        m_aGeneratedAddressTable[nScanLine].MA = currentMA;
        m_aGeneratedAddressTable[nScanLine].RA = RA;
        nScanLine++;
      }

      currentMA += m_anRegisters[HORIZONTAL_DISPLAYED];
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  const CCrtc::SGeneratedAddress* CCrtc::GetGeneratedAddressTable() const
  {
    if (!m_bGeneratedAddressTableUpToDate)
    {
      CCrtc* pNonConstThis;
      pNonConstThis = const_cast<CCrtc*>( this );   // To be able to call/change non-const members (i.e. CCrtc::ComputeGeneratedAddressTable).

      pNonConstThis->ComputeGeneratedAddressTable();
      pNonConstThis->m_bGeneratedAddressTableUpToDate = true;
    }

    return m_aGeneratedAddressTable;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::Run(unsigned nNumCycles)
  {
    for (unsigned i = 0; i < nNumCycles; i++)
    {
      // Advance 1 character per 1us cycle.
      UpdateHorizontal();
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::UpdateHorizontal()
  {
    cpcByte nHorizontalTotal   = m_anRegisters[HORIZONTAL_TOTAL] + 1;
    cpcByte nHorizontalSyncOff = m_anRegisters[HORIZONTAL_SYNC_POSITION] + (m_anRegisters[SYNC_WIDTHS] & 0x0F);

    // Advance 1 character.
    m_nCurrentHCharacter = (m_nCurrentHCharacter + 1) % nHorizontalTotal;

    // Update signals depending on where we are in the scan line.
    if (m_nCurrentHCharacter == 0)    // At start of new CRTC scan line?
    {
      // At this point, monitor raster is right past the left border.
      // Gate-Array starts reading bytes from RAM to generate video signal (if vertical position is in visible area too).
      // Move to next scan line.
      UpdateVertical();
    }
    else if (m_nCurrentHCharacter == m_anRegisters[HORIZONTAL_DISPLAYED])    // At start of right border area?
    {
      // Gate-Array starts using border color to generate video signal.
      m_bDisplayEnabled = false;
    }
    else if (m_nCurrentHCharacter == m_anRegisters[HORIZONTAL_SYNC_POSITION])    // At start of HSYNC high?
    {
      // Monitor starts moving its beam to the beginning of next raster line.
      m_bHSyncState = true;
    }
    else if (m_nCurrentHCharacter == nHorizontalSyncOff)    // At start of HSYNC back to low?
    {
      // Monitor starts rasterizing next raster line (note that the CRTC remains on the current scan line for a few more characters).
      // Also, DISPLAY_ENABLED signal is still OFF, which means the left border is starting to be rasterized.
      m_bHSyncState = false;
      // Notify the Gate Array that HSYNC went from high to low. The Gate Array uses HSYNC and VSYNC to generate interrupts.
      GetMachine()->GetGateArray()->OnHSync();
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::UpdateVertical()
  {
    KMASSERTM( m_anRegisters[VERTICAL_TOTAL_ADJUST] == 0, ("TODO - CRTC's register 5 (VERTICAL_TOTAL_ADJUST) is not 0. We are ignoring it for now.") );

    // Advance 1 scan line.
    cpcByte nMaximumRasterAddress = m_anRegisters[MAXIMUM_RASTER_ADDRESS] + 1;
    m_nCurrentScanLine = (m_nCurrentScanLine + 1) % nMaximumRasterAddress;

    // If VSYNC is active, check whether it is time for it to go inactive.
    if (m_bVSyncState)
    {
      m_nScanLinesForVSyncOff--;
      if (m_nScanLinesForVSyncOff == 0)
      {
        // Monitor starts rasterizing top raster line (note that CRTC doesn't reset character row count yet).
        // Also, DISPLAY_ENABLED signal is still OFF, which means the top border is starting to be rasterized.
        m_bVSyncState = false;
        // Notify the Gate Array that VSYNC went from high to low. The Gate Array uses HSYNC and VSYNC to generate interrupts.
        GetMachine()->GetGateArray()->OnVSync();
      }
    }

    // Is it time to advance to the next character row?
    if (m_nCurrentScanLine == 0)
    {
      cpcByte nVerticalTotal = m_anRegisters[VERTICAL_TOTAL] + 1;

      // Advance 1 character row.
      m_nCurrentVCharacter = (m_nCurrentVCharacter + 1) % nVerticalTotal;

      // Update signals depending on where we are in the frame.
      if (m_nCurrentVCharacter == 0)    // At start of new CRTC frame?
      {
        // At this point, monitor raster is right past the top border.
        // DISPLAY_ENABLED signal is enabled again (see below).
      }
      else if (m_nCurrentVCharacter == m_anRegisters[VERTICAL_SYNC_POSITION])    // At start of VSYNC high?
      {
        // Monitor starts moving its beam to the beginning of top raster line.
        m_bVSyncState = true;
        m_nScanLinesForVSyncOff = (m_anRegisters[SYNC_WIDTHS] & 0xF0) >> 4;
        if (m_nScanLinesForVSyncOff == 0)
        {
          m_nScanLinesForVSyncOff = 16;
        }
      }
    }

    // Display is re-enabled if character row is in the range [0, VERTICAL_DISPLAYED).
    // When enabled, Gate-Array reads bytes from RAM to generate video signal.
    // When disabled, Gate-Array uses border color to generate video signal.
    m_bDisplayEnabled = m_nCurrentVCharacter < m_anRegisters[VERTICAL_DISPLAYED];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::RespondToWritePortRequest(cpcWord nPort, cpcByte nValue)
  {
    //
    // CRTC port --> Bit 14 == 0
    //

    // CRTC port?
    if ( !(nPort & 0x4000) )    // If bit 14 is cleared...
    {
      // Bits 9,8 select the function:
      //   0,0 --> Register select
      //   0,1 --> Register write
      //   1,0 --> *Read-only* (depends on the model of the 6845 chip)
      //   1,1 --> *Read-only* (depends on the model of the 6845 chip)

      switch ((nPort & 0x0300) >> 8)
      {
        // Register select
        case 0:     SelectRegister( nValue<NUM_REGISTERS ? (ERegister)nValue : INVALID_REGISTER ); break;
        // Register write
        case 1:     WriteSelectedRegister( nValue ); break;
        // *Read-only* (depends on the model of the 6845 chip)
        case 2:     /* ... */; break;
        // *Read-only* (depends on the model of the 6845 chip)
        case 3:     /* ... */; break;
      }
    }
  }

} //namespace CPC
