//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcGateArray.h"
#include "cpcMachine.h"
#include "cpcMemoryBlock.h"
#include "cpcCpu.h"
#include "cpcVideoOutput.h"


#define GET_MEMORY_BLOCK_FROM_ADDRESS(addr)   ((addr & 0xC000) >> 14)
#define GET_OFFSET_FROM_ADDRESS(addr)         (addr & 0x3FFF)



namespace CPC {


  /**
  ** RGB conversion table that mimics a color monitor.
  ** It specifies the resulting RGB for each entry in the hardware palette.
  */
  static const unsigned s_aRgbConversionTable_Color[CGateArray::MAX_NUM_PALETTE_COLORS] =
  {
  // Hardware (Firmware)           Red          Green      Blue
  // palette index                 ---          -----      ----
     /* 0 (13) - White*/          (102 << 16) | (102 << 8) | 102,
     /* 1 (13) - White*/          (102 << 16) | (102 << 8) | 102,
     /* 2 (19) - Sea Green*/      (0 << 16) | (255 << 8) | 102,
     /* 3 (25) - Pastel Yellow*/  (255 << 16) | (255 << 8) | 102,
     /* 4 (1)  - Blue*/           (0 << 16) | (0 << 8) | 102,
     /* 5 (7)  - Purple*/         (255 << 16) | (0 << 8) | 102,
     /* 6 (10) - Cyan*/           (0 << 16) | (102 << 8) | 102,
     /* 7 (16) - Pink*/           (255 << 16) | (102 << 8) | 102,
     /* 8 (7)  - Purple*/         (255 << 16) | (0 << 8) | 102,
     /* 9 (25) - Pastel Yellow*/  (255 << 16) | (255 << 8) | 102,
     /*10 (24) - Bright Yellow*/  (255 << 16) | (255 << 8) | 0,
     /*11 (26) - Bright White*/   (255 << 16) | (255 << 8) | 255,
     /*12 (6)  - Bright Red*/     (255 << 16) | (0 << 8) | 0,
     /*13 (8)  - Bright Magenta*/ (255 << 16) | (0 << 8) | 255,
     /*14 (15) - Orange*/         (255 << 16) | (102 << 8) | 0,
     /*15 (17) - Pastel Magenta*/ (255 << 16) | (102 << 8) | 255,
     /*16 (1)  - Blue*/           (0 << 16) | (0 << 8) | 102,
     /*17 (19) - Sea Green*/      (0 << 16) | (255 << 8) | 102,
     /*18 (18) - Bright Green*/   (0 << 16) | (255 << 8) | 0,
     /*19 (20) - Bright Cyan*/    (0 << 16) | (255 << 8) | 255,
     /*20 (0)  - Black*/          (0 << 16) | (0 << 8) | 0,
     /*21 (2)  - Bright Blue*/    (0 << 16) | (0 << 8) | 255,
     /*22 (9)  - Green*/          (0 << 16) | (102 << 8) | 0,
     /*23 (11) - Sky Blue*/       (0 << 16) | (102 << 8) | 255,
     /*24 (4)  - Magenta*/        (102 << 16) | (0 << 8) | 102,
     /*25 (22) - Pastel Green*/   (102 << 16) | (255 << 8) | 102,
     /*26 (21) - Lime*/           (102 << 16) | (255 << 8) | 0,
     /*27 (23) - Pastel Cyan*/    (102 << 16) | (255 << 8) | 255,
     /*28 (3)  - Red*/            (102 << 16) | (0 << 8) | 0,
     /*29 (5)  - Mauve*/          (102 << 16) | (0 << 8) | 255,
     /*30 (12) - Yellow*/         (102 << 16) | (102 << 8) | 0,
     /*31 (14) - Pastel Blue*/    (102 << 16) | (102 << 8) | 255,
  };

  /**
  ** RGB conversion table that mimics a green monitor.
  ** It specifies the resulting RGB for each entry in the hardware palette.
  */
  static const unsigned s_aRgbConversionTable_Green[CGateArray::MAX_NUM_PALETTE_COLORS] =
  {
  //                     Red          Green      Blue
  //                     ---          -----      ----
     /* 0 (13) - White*/          (0 << 16) | (123 << 8) | 0,
     /* 1 (13) - White*/          (0 << 16) | (123 << 8) | 0,
     /* 2 (19) - Sea Green*/      (0 << 16) | (255 << 8) | 0,
     /* 3 (25) - Pastel Yellow*/  (0 << 16) | (255 << 8) | 0,
     /* 4 (1)  - Blue*/           (0 << 16) | (0   << 8) | 0,
     /* 5 (7)  - Purple*/         (0 << 16) | (0   << 8) | 0,
     /* 6 (10) - Cyan*/           (0 << 16) | (94  << 8) | 0,
     /* 7 (16) - Pink*/           (0 << 16) | (151 << 8) | 0,
     /* 8 (7)  - Purple*/         (0 << 16) | (66  << 8) | 0,
     /* 9 (25) - Pastel Yellow*/  (0 << 16) | (236 << 8) | 0,
     /*10 (24) - Bright Yellow*/  (0 << 16) | (226 << 8) | 0,
     /*11 (26) - Bright White*/   (0 << 16) | (255 << 8) | 0,
     /*12 (6)  - Bright Red*/     (0 << 16) | (57  << 8) | 0,
     /*13 (8)  - Bright Magenta*/ (0 << 16) | (76  << 8) | 0,
     /*14 (15) - Orange*/         (0 << 16) | (141 << 8) | 0,
     /*15 (17) - Pastel Magenta*/ (0 << 16) | (160 << 8) | 0,
     /*16 (1)  - Blue*/           (0 << 16) | (9   << 8) | 0,
     /*17 (19) - Sea Green*/      (0 << 16) | (179 << 8) | 0,
     /*18 (18) - Bright Green*/   (0 << 16) | (169 << 8) | 0,
     /*19 (20) - Bright Cyan*/    (0 << 16) | (188 << 8) | 0,
     /*20 (0)  - Black*/          (0 << 16) | (0   << 8) | 0,
     /*21 (2)  - Bright Blue*/    (0 << 16) | (19  << 8) | 0,
     /*22 (9)  - Green*/          (0 << 16) | (85  << 8) | 0,
     /*23 (11) - Sky Blue*/       (0 << 16) | (104 << 8) | 0,
     /*24 (4)  - Magenta*/        (0 << 16) | (38  << 8) | 0,
     /*25 (22) - Pastel Green*/   (0 << 16) | (207 << 8) | 0,
     /*26 (21) - Lime*/           (0 << 16) | (198 << 8) | 0,
     /*27 (23) - Pastel Cyan*/    (0 << 16) | (217 << 8) | 0,
     /*28 (3)  - Red*/            (0 << 16) | (28  << 8) | 0,
     /*29 (5)  - Mauve*/          (0 << 16) | (47  << 8) | 0,
     /*30 (12) - Yellow*/         (0 << 16) | (113 << 8) | 0,
     /*31 (14) - Pastel Blue*/    (0 << 16) | (132 << 8) | 0,
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
    m_paCurrentRgbConversionTable = s_aRgbConversionTable_Color;
    m_eScreenMode       = SCREEN_MODE_1;
    m_nSecondaryRamPage = 1;
    m_eRamConfig        = RAM_CONFIG_0_1_2_3;
    m_bLowerRomVisible  = true;
    m_bUpperRomVisible  = false;
    m_nSelectedUpperRom = 0;
    m_nHSyncCounter     = 0;
    m_nHSyncCountSinceVSync = 0;
    m_bRequestingInterrupt = false;
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
  void CGateArray::OnHSync()
  {
    // Pass it on to the video output.
    // Do this before requesting interrupts so that the image is decoded using current CRTC and Gate Array's values.
    GetMachine()->GetVideoOutput()->OnHSync();

    // Increment the 6-bit counter
    m_nHSyncCounter = (m_nHSyncCounter + 1) & 0x3F;
    m_nHSyncCountSinceVSync++;

    // Is it time to generate an interrupt?
    if (m_nHSyncCountSinceVSync == 2)   // If it is the 2nd HSYNC after the last VSYNC...
    {
      if (m_nHSyncCounter >= 32)
      {
        // Request interrupt.
        m_bRequestingInterrupt = true;
      }
      // Reset counter.
      m_nHSyncCounter = 0;
    }
    else
    {
      if (m_nHSyncCounter >= 52)
      {
        m_nHSyncCounter = 0;
        m_bRequestingInterrupt = true;
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::OnVSync()
  {
    // Pass it on to the video output.
    // Do this before requesting interrupts so that the image is decoded using current CRTC and Gate Array's values.
    GetMachine()->GetVideoOutput()->OnVSync();

    m_nHSyncCountSinceVSync = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::RequestInterruptIfApplicable()
  {
    if (m_bRequestingInterrupt)     // If the Gate Array is currently requesting an interrupt...
    {
      if ( GetMachine()->GetCpu()->RequestInterrupt() )     // If the interrupt has been accepted...
      {
        // Clear the interrupt request
        // Clear top bit (bit 5) of the internal HSYNC counter
        m_bRequestingInterrupt = false;
        m_nHSyncCounter = (m_nHSyncCounter & 0x1F);
      }
      else
      {
        // The Gate Array keeps requesting the interrupt until the CPU accepts it.
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CGateArray::Run(unsigned nMinNumCycles)
  {
    // Request interrupt, if needed.
    RequestInterruptIfApplicable();
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

          // Interrupt control (bit 4).
          // If set to 1, the m_nHSyncCounter counter is reset to 0 and the interrupt request is cleared.
          if (nValue & 0x10)
          {
            m_nHSyncCounter        = 0;
            m_bRequestingInterrupt = false;
          }
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
      SelectUpperRom( nValue );
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
  void CGateArray::SetRgbConversionTable(ERgbConversionTableType eTableType)
  {
    if (eTableType < RGBCONVERSIONTABLE_LAST)
    {
      m_eRgbConversionTableType = eTableType;
      switch (eTableType)
      {
        case RGBCONVERSIONTABLE_COLOR:  m_paCurrentRgbConversionTable = s_aRgbConversionTable_Color; break;
        case RGBCONVERSIONTABLE_GREEN:  m_paCurrentRgbConversionTable = s_aRgbConversionTable_Green; break;
        default:                        KMASSERTM( false, ("Unknown RGB conversion table.") );
      }
    }
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
  void CGateArray::SelectUpperRom(cpcByte nIndex)
  {
    m_nSelectedUpperRom = nIndex;
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
    m_apVisibleReadBlocks[0] = (m_bLowerRomVisible ? GetMachine()->GetMemory()->GetLowerRomBlock() : m_apVisibleWriteBlocks[0]);
    m_apVisibleReadBlocks[1] = m_apVisibleWriteBlocks[1];
    m_apVisibleReadBlocks[2] = m_apVisibleWriteBlocks[2];
    m_apVisibleReadBlocks[3] = (m_bUpperRomVisible ? GetMachine()->GetMemory()->GetUpperRomBlock(m_nSelectedUpperRom) : m_apVisibleWriteBlocks[3]);

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
