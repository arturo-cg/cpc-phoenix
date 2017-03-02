//-------------------------------------------------------------------------------------------
// File:        Cpu.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcThirdPartyCpu.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"


namespace CPC {


  //--------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------
  // Callbacks used by the z80ex library.
  //--------------------------------------------------------------------------------------

  /*called on each T-State [optional]*/
  void CB_TState(Z80EX_CONTEXT *context, void *user_data)
  {
    CThirdPartyCpu* cpu = (CThirdPartyCpu*)user_data;
    cpu->m_nNumTStatesSinceLastWait++;
    // Force the Z80 to enter into a wait state every 4 t-states.
    const unsigned WAIT_EVERY_NUM_TSTATES_MINUS_ONE = 4 - 1;
    if ((cpu->m_nNumTStatesSinceLastWait % WAIT_EVERY_NUM_TSTATES_MINUS_ONE) == 0)
    {
      cpu->m_nNumTStatesSinceLastWait = 0;
      // Generate 1 Wait-state.
      // Note: The call to z80ex_w_states causes this callback to be triggered again.
      z80ex_w_states(context, 1);
    }
  }

  /*read byte from memory <addr> -- called when RD & MREQ goes active.
  m1_state will be 1 if M1 signal is active*/
  Z80EX_BYTE CB_ReadByteFromMemory(Z80EX_CONTEXT *context, Z80EX_WORD addr, int m1_state, void *user_data)
  {
    CThirdPartyCpu* cpu = (CThirdPartyCpu*) user_data;
    return cpu->GetMachine()->GetGateArray()->ReadByteFromMemory(addr);
  }

  /*write <value> to memory <addr> -- called when WR & MREQ goes active*/
  void CB_WriteByteToMemory(Z80EX_CONTEXT *context, Z80EX_WORD addr, Z80EX_BYTE value, void *user_data)
  {
    CThirdPartyCpu* cpu = (CThirdPartyCpu*)user_data;
    cpu->GetMachine()->GetGateArray()->WriteByteToMemory(addr, value);
  }

  /*read byte from <port> -- called when RD & IORQ goes active*/
  Z80EX_BYTE CB_ReadByteFromPort(Z80EX_CONTEXT *context, Z80EX_WORD port, void *user_data)
  {
    CThirdPartyCpu* cpu = (CThirdPartyCpu*)user_data;
    return cpu->GetMachine()->ReadByteFromPort(port);
  }

  /*write <value> to <port> -- called when WR & IORQ goes active*/
  void CB_WriteByteToPort(Z80EX_CONTEXT *context, Z80EX_WORD port, Z80EX_BYTE value, void *user_data)
  {
    CThirdPartyCpu* cpu = (CThirdPartyCpu*)user_data;
    cpu->GetMachine()->WriteByteToPort(port, value);
  }

  /*read byte of interrupt vector -- called when M1 and IORQ goes active*/
  Z80EX_BYTE CB_ReadByteFromInterruptVector(Z80EX_CONTEXT *context, void *user_data)
  {
    //CThirdPartyCpu* cpu = (CThirdPartyCpu*)user_data;
    //***************************** TODO - TODO - TODO ************************************
    //***************************** TODO - TODO - TODO ************************************
    return 0;
    //***************************** TODO - TODO - TODO ************************************
    //***************************** TODO - TODO - TODO ************************************
  }


  //--------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CThirdPartyCpu::CThirdPartyCpu(CMachine *pMachine) : inherited( pMachine )
  {
    ResetVars();

    // Create the Z80 context
    m_pZ80State = z80ex_create( CB_ReadByteFromMemory, this,
                                CB_WriteByteToMemory, this,
                                CB_ReadByteFromPort, this,
                                CB_WriteByteToPort, this,
                                CB_ReadByteFromInterruptVector, this );

    ///////////////////////////////////////////////////////z80ex_set_tstate_callback( m_pZ80State, CB_TState, this );
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CThirdPartyCpu::ResetVars()
  {
    m_pZ80State                    = NULL;
    m_nNumTStatesSinceLastWait     = 0;
    m_bCompleteInstructionLastStep = true;
    m_nNumSpareCycles              = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CThirdPartyCpu::FreeVars()
  {
    if (m_pZ80State != NULL)
    {
      z80ex_destroy( m_pZ80State );
      m_pZ80State = NULL;
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CThirdPartyCpu::Reset()
  {
    KMASSERT( m_pZ80State != NULL );
    z80ex_reset( m_pZ80State );

    m_nNumTStatesSinceLastWait     = 0;
    m_bCompleteInstructionLastStep = true;
    m_nNumSpareCycles              = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  bool CThirdPartyCpu::RequestInterrupt()
  {
    return (z80ex_int(m_pZ80State) > 0);
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CThirdPartyCpu::Run(unsigned nMinNumCycles)
  {
    unsigned nCurrNumCycles;

    KMASSERT( m_pZ80State != NULL );

    // Was there any spare cycle from the last call to CThirdPartyCpu::Run?
    nCurrNumCycles = m_nNumSpareCycles;

    while (nCurrNumCycles < nMinNumCycles)
    {
      nCurrNumCycles += z80ex_step( m_pZ80State );
    }

    // Spare cycles (will be taken into account next time CThirdPartyCpu::Run is called)
    m_nNumSpareCycles = nCurrNumCycles - nMinNumCycles;

    // Has last instruction been executed completely?
    m_bCompleteInstructionLastStep = ( z80ex_last_op_type( m_pZ80State ) == 0 );
  }

} //namespace CPC
