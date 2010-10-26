//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcGateArray.h"
#include "cpcMachine.h"
#include "cpcMemoryBlock.h"


#define GET_MEMORY_BLOCK_FROM_ADDRESS(addr)   ((addr & 0xC000) >> 14)
#define GET_OFFSET_FROM_ADDRESS(addr)         (addr & 0x3FFF)



namespace CPC {


  /**
  ** Hardware palette for a color monitor.
  */
  static const unsigned s_aColorMonitorPalette[CGateArray::MAX_NUM_PALETTE_COLORS] =
  {
  //                     Red          Green      Blue
  //                     ---          -----      ----
 /*0 - White*/          (102 << 16) | (102 << 8) | 102,
 /*1 - White*/          (102 << 16) | (102 << 8) | 102,
 /*2 - Sea Green*/      (0 << 16) | (255 << 8) | 102,
 /*3 - Pastel Yellow*/  (255 << 16) | (255 << 8) | 102,
 /*4 - Blue*/           (0 << 16) | (0 << 8) | 102,
 /*5 - Purple*/         (255 << 16) | (0 << 8) | 102,
 /*6 - Cyan*/           (0 << 16) | (102 << 8) | 102,
 /*7 - Pink*/           (255 << 16) | (102 << 8) | 102,
 /*8 - Purple*/         (255 << 16) | (0 << 8) | 102,
 /*9 - Pastel Yellow*/  (255 << 16) | (255 << 8) | 102,
/*10 - Bright Yellow*/  (255 << 16) | (255 << 8) | 0,
/*11 - Bright White*/   (255 << 16) | (255 << 8) | 255,
/*12 - Bright Red*/     (255 << 16) | (0 << 8) | 0,
/*13 - Bright Magenta*/ (255 << 16) | (0 << 8) | 255,
/*14 - Orange*/         (255 << 16) | (102 << 8) | 0,
/*15 - Pastel Magenta*/ (255 << 16) | (102 << 8) | 255,
/*16 - Blue*/           (0 << 16) | (0 << 8) | 102,
/*17 - Sea Green*/      (0 << 16) | (255 << 8) | 102,
/*18 - Bright Green*/   (0 << 16) | (255 << 8) | 0,
/*19 - Bright Cyan*/    (0 << 16) | (255 << 8) | 255,
/*20 - Black*/          (0 << 16) | (0 << 8) | 0,
/*21 - Bright Blue*/    (0 << 16) | (0 << 8) | 255,
/*22 - Green*/          (0 << 16) | (102 << 8) | 0,
/*23 - Sky Blue*/       (0 << 16) | (102 << 8) | 255,
/*24 - Magenta*/        (102 << 16) | (0 << 8) | 102,
/*25 - Pastel Green*/   (102 << 16) | (255 << 8) | 102,
/*26 - Lime*/           (102 << 16) | (255 << 8) | 0,
/*27 - Pastel Cyan*/    (102 << 16) | (255 << 8) | 255,
/*28 - Red*/            (102 << 16) | (0 << 8) | 0,
/*29 - Mauve*/          (102 << 16) | (0 << 8) | 255,
/*30 - Yellow*/         (102 << 16) | (102 << 8) | 0,
/*31 - Pastel Blue*/    (102 << 16) | (102 << 8) | 255,
  };

  /**
  ** Hardware palette for a green monitor.
  */
  static const unsigned s_aGreenMonitorPalette[CGateArray::MAX_NUM_PALETTE_COLORS] =
  {
  };


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

    m_nBorderColor      = 0;
    m_paCurrentPalette  = s_aColorMonitorPalette;      // TODO - Allow the user to change this.
    m_eScreenMode       = SCREEN_MODE_1;
    m_nSecondaryRamPage = 1;
    m_eRamConfig        = RAM_CONFIG_0_1_2_3;
    m_bLowerRomVisible  = true;
    m_bUpperRomVisible  = false;
    m_eSelectedUpperRom = CMemory::ROMINDEX_BASIC;
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
    if ( !(nPort & 0x8000) && (nPort & 0x4000) )
    {
      //
      // The two most significant bits (7 and 6) of nValue determine the function to be performed
      //

      switch ((nValue&0xC0) >> 6)
      {
      case 0:   // Select pen
        {
          SelectPen( nValue & 0x1F );  // Take bits 4-0 only. If bit 4 is set, the border is selected.
        }
        break;

      case 1:   // Change selected pen color
        {
          // Bits 4-0 contain the new color index for the selected pen or border
          if (m_nSelectedPen < MAX_NUM_PENS)
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
          SetRomVisibility( (nValue&0x04)==0, (nValue&0x08)==0 );

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
        SetRamConfiguration( 1/*nSecondaryRamPage*/, (ERamConfig) (nValue&0x03) );
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
/////////////////////////////        case 7:   SelectUpperRom( CMemory::ROMINDEX_AMSDOS ); break;
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
    // If nPen is >= 16, the border is selected instead of a pen.
    m_nSelectedPen = nPen;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SetSelectedPenColor(cpcByte nColorIndex)
  {
    KMASSERT( m_nSelectedPen < MAX_NUM_PENS );
    KMASSERT( nColorIndex < MAX_NUM_PALETTE_COLORS );
    m_anPenColors[m_nSelectedPen] = nColorIndex;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::SetBorderColor(cpcByte nColorIndex)
  {
    KMASSERT( nColorIndex < MAX_NUM_PALETTE_COLORS );
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
    m_nSecondaryRamPage = nSecondaryPage;
    m_eRamConfig        = eConfig;

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
    switch (eIndex)
    {
      case CMemory::ROMINDEX_BASIC:
      case CMemory::ROMINDEX_AMSDOS:
        m_eSelectedUpperRom = eIndex;
        break;

      default:
        m_eSelectedUpperRom = CMemory::ROMINDEX_BASIC;     // If the requested ROM doesn't exist, BASIC ROM is selected.
    }

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
      int nRamPage;
      nRamPage = ( config.Range[i].bFromSecondaryPage ? m_nSecondaryRamPage : 0 );
      m_apVisibleWriteBlocks[i] = GetMachine()->GetMemory()->GetRamBlock( config.Range[i].nBlockIndex + (nRamPage * 4) );
    }

    // Read blocks
    m_apVisibleReadBlocks[0] = (m_bLowerRomVisible ? GetMachine()->GetMemory()->GetRomBlock(CMemory::ROMINDEX_OS) : m_apVisibleWriteBlocks[0]);
    m_apVisibleReadBlocks[1] = m_apVisibleWriteBlocks[1];
    m_apVisibleReadBlocks[2] = m_apVisibleWriteBlocks[2];
    m_apVisibleReadBlocks[3] = (m_bUpperRomVisible ? GetMachine()->GetMemory()->GetRomBlock(m_eSelectedUpperRom) : m_apVisibleWriteBlocks[3]);

    //// Print currently visible read and write blocks
    //{
    //  string sDebugStr;

    //  sDebugStr += "WRITE blocks: ";
    //  for (i = 0; i < 4; i++)
    //  {
    //    sDebugStr += m_apVisibleWriteBlocks[i]->GetLabel() + " | ";
    //  }

    //  sDebugStr += "      READ blocks: ";
    //  for (i = 0; i < 4; i++)
    //  {
    //    sDebugStr += m_apVisibleReadBlocks[i]->GetLabel() + " | ";
    //  }

    //  sDebugStr += '\n';
    //  OutputDebugString( sDebugStr.c_str() );
    //}
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
