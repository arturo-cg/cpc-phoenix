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
    m_nSelectedRegister = 0;
    m_uCycleCount       = 0;
    //m_uFrameCount       = 0;
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

      switch (nPort & 0x0300)
      {
        case 0x0:     // Register select
          break;
        case 0x1:     // Register write
          break;
      }
    }
  }

} //namespace CPC
