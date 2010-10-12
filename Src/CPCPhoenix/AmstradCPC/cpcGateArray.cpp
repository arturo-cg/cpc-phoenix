//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcGateArray.h"
#include "cpcMachine.h"
#include "cpcMemoryBlock.h"


#define GET_MEMORY_BLOCK_FROM_ADDRESS(addr)   ((addr & 0xC000) >> 14)
#define GET_OFFSET_FROM_ADDRESS(addr)         (addr & 0x3FFF)



namespace CPC {


  struct SRamConfigItem
  {
    unsigned nBlockIndex;
    bool     bFromSecondaryPage;
  };

  struct SRamConfigEntry
  {
    SRamConfigItem Range[4];
  };

  static SRamConfigEntry s_aRamConfigTable[] =
  {
    // RAM_CONFIG_0_1_2_3
    { 0, false, 1, false, 2, false, 3, false },
    // RAM_CONFIG_0_1_2_3s
    { 0, false, 1, false, 2, false, 3, true  },
    // RAM_CONFIG_0s_1s_2s_3s
    { 0, true , 1, true , 2, true , 3, true  },
    // RAM_CONFIG_0_3_2_3s
    { 0, false, 3, false, 2, false, 3, true  },
    // RAM_CONFIG_0_0s_2_3
    { 0, false, 0, true , 2, false, 3, false },
    // RAM_CONFIG_0_1s_2_3
    { 0, false, 1, true , 2, false, 3, false },
    // RAM_CONFIG_0_2s_2_3
    { 0, false, 2, true , 2, false, 3, false },
    // RAM_CONFIG_0_3s_2_3
    { 0, false, 3, true , 2, false, 3, false },
  };



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CGateArray::CGateArray(CMachine *pMachine) : inherited( pMachine )
  {
    // Simulate a system reset
    Reset();
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

    m_nBorderColor     = 0;
    m_eScreenMode      = SCREEN_MODE_1;
    m_nSecondaryPage   = 0;
    m_eRamConfig       = RAM_CONFIG_0_1_2_3;
    m_bLowerRomVisible = true;
    m_bUpperRomVisible = false;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CGateArray::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CGateArray::Reset()
  {
    // Reset members
    ResetVars();

    // Determine visible read/write blocks
    UpdateVisibleMemoryBlocks();
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
    // RAM configuration port --> Bit 15 == 0
    // ROM Select port --> Bit 13 == 0
    //
    // Note: Several ports can be written to at the same time, and also the same device can respond to different addresses.
    //       This is because the CPC doesn't fully decode the port address.
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
          SetScreenMode( (EScreenMode) (nValue & 0x03) );

          // ROM selection (bit 2 - Lower ROM, bit 3 - Upper ROM)
          SetRomVisibility( (nValue&0x02)==0, (nValue&0x04)==0 );

          // Interrupt control
          //**************************************** TODO - TODO - TODO **********************************************
          //**************************************** TODO - TODO - TODO **********************************************
          //**************************************** TODO - TODO - TODO **********************************************
          //**************************************** TODO - TODO - TODO **********************************************
        }
        break;
      }
    }

    // RAM configuration port?
    if( !(nPort & 0x8000) )      // If bit 15 is cleared...
    {
      if( ((nValue & 0xC0) >> 6) == 4)
      {
        // Bits 2-0 define one of the eight possible RAM configurations
        // Note: If we wanted to emulate expansion RAMs other than the CPC6128 built-in one, we would have
        //       to look into bits 4,3 which contain the secondary 64k page to use.
        SetRamConfiguration( 0/*nSecondaryPage*/, (ERamConfig) (nValue&0x03) );
      }
    }

    // ROM Select port?
    if( !(nPort & 0x2000) )    // If bit 13 is cleared...
    {
      // This selects the upper ROM in use (range &C000-&FFFF), but it still needs to be made visible through the Gate Array.
      // Lower ROM (range &0000-&3fff) cannot be changed, Operating System ROM is the only choice.
      //
      // Every expansion ROM has a 8-bit identifier. BASIC has identifier 0 and AMSDOS has identifier 7.
      // If an attempt to select a ROM that is not connected is made, BASIC is selected.
      switch (nValue)
      {
        case 0:   SelectUpperRom( CMemory::ROMINDEX_BASIC ); break;
        case 7:   SelectUpperRom( CMemory::ROMINDEX_AMSDOS ); break;
        // ...Insert other expansion ROMs here...
        default:  SelectUpperRom( CMemory::ROMINDEX_BASIC ); break;
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
  void CGateArray::SetScreenMode(EScreenMode eScreenMode)
  {
    m_eScreenMode = eScreenMode;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SetRamConfiguration(unsigned nSecondaryPage, ERamConfig eConfig)
  {
    m_nSecondaryPage = nSecondaryPage;
    m_eRamConfig     = eConfig;

    UpdateVisibleMemoryBlocks();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SetRomVisibility(bool bLowerRomVisible, bool bUpperRomVisible)
  {
    m_bLowerRomVisible = bLowerRomVisible;
    m_bUpperRomVisible = bUpperRomVisible;

    UpdateVisibleMemoryBlocks();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SelectUpperRom(CMemory::ERomBlockIndex eIndex)
  {
    m_eSelectedUpperRom = eIndex;
    UpdateVisibleMemoryBlocks();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::UpdateVisibleMemoryBlocks()
  {
    SRamConfigEntry &config = s_aRamConfigTable[ m_eRamConfig ];
    unsigned         i;

    // Write blocks
    for (i = 0; i < 4; i++)
    {
      if ( !config.Range[i].bFromSecondaryPage )
      {
        // From primary page
        m_apVisibleWriteBlocks[i] = GetMachine()->GetMemory()->GetRamBlock( config.Range[i].nBlockIndex );
      }
      else
      {
        // From secondary page
        m_apVisibleWriteBlocks[i] = GetMachine()->GetMemory()->GetRamBlock( config.Range[i].nBlockIndex + (m_nSecondaryPage * 4) );
      }
    }

    // Read blocks
    m_apVisibleReadBlocks[0] = (m_bLowerRomVisible ? GetMachine()->GetMemory()->GetRomBlock(CMemory::ROMINDEX_OS) : m_apVisibleWriteBlocks[0]);
    m_apVisibleReadBlocks[1] = m_apVisibleWriteBlocks[1];
    m_apVisibleReadBlocks[2] = m_apVisibleWriteBlocks[2];
    m_apVisibleReadBlocks[3] = (m_bUpperRomVisible ? GetMachine()->GetMemory()->GetRomBlock(m_eSelectedUpperRom) : m_apVisibleWriteBlocks[3]);
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CGateArray::ReadByteFromMemory(cpcWord nAddress) const
  {
    // Bits 15,14 of nAddress determine which one of the four visible blocks to use
    // Bits 13-0 of nAddress determine the offset into the selected block
    return m_apVisibleReadBlocks[GET_MEMORY_BLOCK_FROM_ADDRESS(nAddress)]->ReadByte( GET_OFFSET_FROM_ADDRESS(nAddress) );
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::WriteByteToMemory(cpcWord nAddress, cpcByte nValue)
  {
    // Bits 15,14 of nAddress determine which one of the four visible blocks to use
    // Bits 13-0 of nAddress determine the offset into the selected block
    m_apVisibleWriteBlocks[GET_MEMORY_BLOCK_FROM_ADDRESS(nAddress)]->WriteByte( GET_OFFSET_FROM_ADDRESS(nAddress), nValue );
  }

} //namespace CPC
