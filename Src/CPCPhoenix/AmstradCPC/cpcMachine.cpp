//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcMachine.h"
#include "cpcCpu.h"
#include "cpcMemory.h"
#include "cpcGateArray.h"
#include "cpcCrtc.h"
#include "cpcPpi.h"
#include "cpcPsg.h"
#include "cpcDisplay.h"
#include "cpcKeyboard.h"



namespace CPC {



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMachine::CMachine(EModel eType)
  {
    ResetVars();

    m_eModel = eType;

    // Create the sub-systems
    m_pCpu       = new CCpu( this );
    m_pMemory    = new CMemory( this );
    m_pGateArray = new CGateArray( this );
    m_pCrtc      = new CCrtc( this );
    m_pPpi       = new CPpi( this );
    m_pPsg       = new CPsg( this );
    m_pDisplay   = new CDisplay( this );
    m_pKeyboard  = new CKeyboard( this );
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CMachine::ResetVars()
  {
    m_eModel     = MODEL_INVALID;
    m_pCpu       = NULL;
    m_pMemory    = NULL;
    m_pGateArray = NULL;
    m_pCrtc      = NULL;
    m_pPpi       = NULL;
    m_pPsg       = NULL;
    m_pDisplay   = NULL;
    m_pKeyboard  = NULL;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CMachine::FreeVars()
  {
    delete m_pKeyboard; m_pKeyboard = NULL;
    delete m_pDisplay; m_pDisplay = NULL;
    delete m_pPsg; m_pPsg = NULL;
    delete m_pPpi; m_pPpi = NULL;
    delete m_pCrtc; m_pCrtc = NULL;
    delete m_pGateArray; m_pGateArray = NULL;
    delete m_pMemory; m_pMemory = NULL;
    delete m_pCpu; m_pCpu = NULL;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  unsigned CMachine::GetFrameCount() const
  {
    return m_pCrtc->GetFrameCount();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CMachine::ReadByteFromPort(cpcWord nPort)
  {
    cpcByte nRet;
    if ( !GetPpi()->RespondToReadPortRequest(nPort, &nRet) )
    {
      //if ( !GetDEVICE2()->RespondToReadPortRequest(nPort, &nRet) )
      {
        //if ( !GetDEVICE3()->RespondToReadPortRequest(nPort, &nRet) )
        {
          // No device has served the request - Return default value
          // TODO - What value does the real CPC return in this case?
          nRet = 0xFF;
        }
      }
    }

    return nRet;
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
    GetCrtc()->RespondToWritePortRequest( nPort, nValue );
    GetPpi()->RespondToWritePortRequest( nPort, nValue );
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
    KMASSERT( nMicroSecs >= 10 );  // To ensure a minimum precision in the integer division in the next line
    nNumCycles = (nMicroSecs * 33) / 10;
    GetCpu()->Run( nNumCycles );

    // CRTC (1Mhz clock)
    nNumCycles = nMicroSecs;
    GetCrtc()->Run( nNumCycles );

    // Gate-Array (1Mhz clock)
    nNumCycles = nMicroSecs;
    GetGateArray()->Run( nNumCycles );
  }

} //namespace CPC
