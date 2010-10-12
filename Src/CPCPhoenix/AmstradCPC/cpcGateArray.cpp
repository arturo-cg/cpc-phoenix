//-------------------------------------------------------------------------------------------
// File:        GateArray.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcGateArray.h"
#include "cpcMachine.h"
#include "cpcMemory.h"



namespace CPC {



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CGateArray::CGateArray(CMachine *pMachine) : inherited( pMachine )
  {
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CGateArray::ResetVars()
  {
    int i;

    m_nSelectedPen = 0;

    for(i=0; i < MAX_NUM_PENS; i++)
    {
      m_anPenColors[i] = i;
    }

    m_nBorderColor = 0;
    m_eScreenMode  = SCREEN_MODE_1;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CGateArray::FreeVars()
  {

  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CGateArray::Reset()
  {



  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::Run(unsigned nMinNumCycles)
  {









  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::RespondToWritePortRequest(cpcWord nPort, cpcByte nValue)
  {
    //
    // Gate-Array port --> Bit 15 == 0, Bit 14 == 1
    //

    // Gate-Array port?
    if( !(nPort & 0x8000) && (nPort & 0x4000) )
    {
      //
      // The two most significant bits (7 and 6) of nValue determine the function to be performed
      //

      switch( (nValue&0xC0) >> 6 )
      {
      case 0:   // Select pen
        {
          SelectPen( nValue & 0x1F );  // Take bits 4-0 only. If bit 4 is set, the border is selected.
        }
        break;

      case 1:   // Change selected pen color
        {
          // Bits 4-0 contain the new color index for the selected pen or border
          if(m_nSelectedPen < 16)
          {
            SetSelectedPenColor( nValue & 0x1F );
          }
          else
          {
            SetBorderColor( nValue & 0x1F );
          }
        }
        break;

      case 2:   // Change screen mode, ROM visibility and interrupt control
        {
          // Screen mode (bits 1,0)
          SetScreenMode( (TScreenMode) (nValue & 0x03) );

          // ROM selection (bit 2 - Lower ROM, bit 3 - Upper ROM)
          GetMachine()->GetMemory()->SetRomVisibility( (nValue&0x02)==0, (nValue&0x04)==0 );

          // Interrupt control
          //**************************************** TODO - TODO - TODO **********************************************
          //**************************************** TODO - TODO - TODO **********************************************
          //**************************************** TODO - TODO - TODO **********************************************
          //**************************************** TODO - TODO - TODO **********************************************
        }
        break;
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SelectPen(cpcByte nPen)
  {
    ASSERT( nPen < MAX_NUM_PENS );
    m_nSelectedPen = nPen;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SetSelectedPenColor(cpcByte nColorIndex)
  {
    ASSERT( nColorIndex < MAX_NUM_PALETTE_COLORS );
    m_anPenColors[m_nSelectedPen] = nColorIndex;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SetBorderColor(cpcByte nColorIndex)
  {
    ASSERT( nColorIndex < MAX_NUM_PALETTE_COLORS );
    m_nBorderColor = nColorIndex;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SetScreenMode(TScreenMode eScreenMode)
  {
    m_eScreenMode = eScreenMode;
  }

} //namespace CPC
