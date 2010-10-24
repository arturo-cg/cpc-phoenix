//-------------------------------------------------------------------------------------------
// File:        Crtc.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcCrtc.h"


namespace CPC {


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CCrtc::CCrtc(CMachine *pMachine) : inherited( pMachine )
  {
    m_uFrameCount = 0;

    // Simulate a system reset
    Reset();
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CCrtc::ResetVars()
  {
    m_eSelectedRegister = HORIZONTAL_TOTAL;
    m_uCycleCount       = 0;
    //m_uFrameCount       = 0;
    m_bGeneratedAddressTableUpToDate = false;
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
    else
    {
      // TODO - What to do when the index is invalid?
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
    m_uCycleCount += nNumCycles;
    if (m_uCycleCount >= CYCLES_PER_FRAME)
    {
      // New frame
      m_uFrameCount++;
      m_uCycleCount -= CYCLES_PER_FRAME;
    }
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
