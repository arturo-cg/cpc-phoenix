//-------------------------------------------------------------------------------------------
// File:        Machine.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcMachine.h"
#include "cpcCpu.h"
#include "cpcMemory.h"
#include "cpcGateArray.h"
#include "cpcCrtc.h"
#include "cpcVideoOutput.h"


#define FRAME_PERIOD 19968


namespace CPC {



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMachine::CMachine(TModel eType)
  {
    ResetVars();

    m_eModel = eType;

    // Create the sub-systems
    m_pCpu       = new CCpu( this );
    m_pMemory    = new CMemory( this );
    m_pGateArray = new CGateArray( this );
    m_pCrtc      = new CCrtc( this );
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CMachine::ResetVars()
  {
    m_eModel             = CPC_INVALID;
    m_pCpu               = NULL;
    m_pMemory            = NULL;
    m_pGateArray         = NULL;
    m_pCrtc              = NULL;
    m_pVideoOutput     = NULL;
    m_nTimeFromLastFrame = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CMachine::FreeVars()
  {
    delete m_pCpu; m_pCpu = NULL;
    delete m_pMemory; m_pMemory = NULL;
    delete m_pGateArray; m_pGateArray = NULL;
    delete m_pCrtc; m_pCrtc = NULL;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CMachine::ReadByteFromPort(cpcWord nPort) const
  {
    //***************************** TODO - TODO - TODO ************************************
    //***************************** TODO - TODO - TODO ************************************
    return 0;
    //***************************** TODO - TODO - TODO ************************************
    //***************************** TODO - TODO - TODO ************************************
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMachine::WriteByteToPort(cpcWord nPort, cpcByte nValue)
  {
    // The Amstrad CPC doesn't decode the port address fully. This implies that: (1) a device can respond to
    // more than one port address and (2) several devices can respond to a single port address.
    GetGateArray()->RespondToWritePortRequest( nPort, nValue );
    GetMemory()->RespondToWritePortRequest( nPort, nValue );
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMachine::Reset()
  {
    GetCpu()->Reset();
    GetMemory()->Reset();
    GetGateArray()->Reset();
    GetCrtc()->Reset();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMachine::Run(unsigned nMicroSecs)
  {
    unsigned nNumCycles;

    // CPU (3.3Mhz clock)
    //
    // NOTE: The Z80 clock input in the real CPC runs at 4Mhz, although its effective frequency is 3.3Mhz.
    //   The reason is that both the Gate-Array and the CPU need to access the memory (the Gate-Array reads
    //   it to generate the video signal). The Gate-Array has priority over the CPU to ensure video signal
    //   is correctly generated, so it periodically forces the CPU to enter in a wait state while it is
    //   reading the memory. Because of this, we simply think of the CPU clock input as running at 3.3Mhz.
    ASSERT( nMicroSecs >= 10 );  // To ensure a minimum precision in the integer division in the next line
    nNumCycles = (nMicroSecs * 33) / 10;
    GetCpu()->Run( nNumCycles );

    // CRTC (1Mhz clock)
    nNumCycles = nMicroSecs;
    GetCrtc()->Run( nNumCycles );

    // Gate-Array (1Mhz clock)
    nNumCycles = nMicroSecs;
    GetGateArray()->Run( nNumCycles );

    // Should we draw a new frame? The CPC VDU displays a new frame every 19968 microseconds (50Hz frame rate)
    m_nTimeFromLastFrame += nMicroSecs;

    if( m_nTimeFromLastFrame >= FRAME_PERIOD )
    {
      // Notify the frame listener
      if(m_pVideoOutput != NULL)
      {
        m_pVideoOutput->NotifyNewFrame();
      }

      // Accumulate the remaining time for the next frame notification
      m_nTimeFromLastFrame -= FRAME_PERIOD;
    }

  }

} //namespace CPC
