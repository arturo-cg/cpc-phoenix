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
    // Reset members
    ResetVars();
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CCrtc::ResetVars()
  {
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CCrtc::FreeVars()
  {
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CCrtc::Reset()
  {



  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CCrtc::Run(unsigned nMinNumCycles)
  {









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
    if( !(nPort & 0x4000) )
    {
      // Bits 9,8 select a function:
      //   0,0 --> Register select
      //   0,1 --> Register write
      //   1,0 --> (depends on the model of the 6845 chip)
      //   1,1 --> (depends on the model of the 6845 chip)

      //...
    }
  }

} //namespace CPC
