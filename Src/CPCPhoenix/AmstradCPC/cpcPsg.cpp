//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcPsg.h"
#include "cpcMachine.h"
#include "cpcPpi.h"



namespace CPC {


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CPsg::CPsg(CMachine *pMachine) : inherited( pMachine )
  {
    // Reset members
    ResetVars();

    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CPsg::ResetVars()
  {
    m_nSelectedRegister = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CPsg::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CPsg::Reset()
  {
    ResetVars();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CPsg::SelectFunction(EFunction eFunction)
  {
    switch (eFunction)
    {
      case FUNCTION_INACTIVE:
        // Inactive - Do nothing.
        break;

      case FUNCTION_READ_REGISTER:
        // We do nothing right now, CPpi will eventually call CPsg::GetSelectedRegisterValue to actually read the selected register.
        {
          char szBuffer[100];
          _snprintf( szBuffer, sizeof(szBuffer), "PSG: Read register %d\n", m_nSelectedRegister );
          OutputDebugString( szBuffer );
        }
        break;

      case FUNCTION_WRITE_REGISTER:
        // We take the value from PPI port A and write it to the currently selected PSG register.
        m_anRegisters[m_nSelectedRegister] = GetMachine()->GetPpi()->GetPortOutputValue( CPpi::PORT_A );
        {
          char szBuffer[100];
          _snprintf( szBuffer, sizeof(szBuffer), "PSG: Write %d into register %d\n", GetMachine()->GetPpi()->GetPortOutputValue(CPpi::PORT_A), m_nSelectedRegister );
          OutputDebugString( szBuffer );
        }
        break;

      case FUNCTION_SELECT_REGISTER:
        // We take the register index from PPI port A and remember it for subsequent register reads/writes.
        m_nSelectedRegister = (unsigned) (GetMachine()->GetPpi()->GetPortOutputValue( CPpi::PORT_A ) & 0x0F);   // Bits 3-0 of PPI port A value contain the register index.
        {
          char szBuffer[100];
          _snprintf( szBuffer, sizeof(szBuffer), "PSG: Select register %d\n", m_nSelectedRegister );
          OutputDebugString( szBuffer );
        }
    }
  }

} //namespace CPC
