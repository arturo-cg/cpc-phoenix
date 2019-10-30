//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcGateArray.h"
#include "cpcMachine.h"
#include "cpcMemoryBlock.h"
#include "cpcCpu.h"
#include "cpcVideoOutput.h"
//#include <Windows.h>


#define GET_MEMORY_BLOCK_FROM_ADDRESS(addr)   ((addr & 0xC000) >> 14)
#define GET_OFFSET_FROM_ADDRESS(addr)         (addr & 0x3FFF)

// Final
// Memory     CRTC
// Address    Address
// -------    -------
// A15        MA13
// A14        MA12
// A13        RA2
// A12        RA1
// A11        RA0
// A10        MA9
// A9         MA8
// A8         MA7
// A7         MA6
// A6         MA5
// A5         MA4
// A4         MA3
// A3         MA2
// A2         MA1
// A1         MA0
// A0         CCLK
#define CONVERT_ADDRESS_CRTC_TO_MEMORY(_MA, _RA) \
  ( (((_MA) & 0x3000) <<  2) | \
    (((_RA) & 0x0007) << 11) | \
    (((_MA) & 0x03FF) <<  1) )


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
        /* 4 (1)  - Blue*/           (0 << 16) | (0 << 8) | 0,
        /* 5 (7)  - Purple*/         (0 << 16) | (0 << 8) | 0,
        /* 6 (10) - Cyan*/           (0 << 16) | (94 << 8) | 0,
        /* 7 (16) - Pink*/           (0 << 16) | (151 << 8) | 0,
        /* 8 (7)  - Purple*/         (0 << 16) | (66 << 8) | 0,
        /* 9 (25) - Pastel Yellow*/  (0 << 16) | (236 << 8) | 0,
        /*10 (24) - Bright Yellow*/  (0 << 16) | (226 << 8) | 0,
        /*11 (26) - Bright White*/   (0 << 16) | (255 << 8) | 0,
        /*12 (6)  - Bright Red*/     (0 << 16) | (57 << 8) | 0,
        /*13 (8)  - Bright Magenta*/ (0 << 16) | (76 << 8) | 0,
        /*14 (15) - Orange*/         (0 << 16) | (141 << 8) | 0,
        /*15 (17) - Pastel Magenta*/ (0 << 16) | (160 << 8) | 0,
        /*16 (1)  - Blue*/           (0 << 16) | (9 << 8) | 0,
        /*17 (19) - Sea Green*/      (0 << 16) | (179 << 8) | 0,
        /*18 (18) - Bright Green*/   (0 << 16) | (169 << 8) | 0,
        /*19 (20) - Bright Cyan*/    (0 << 16) | (188 << 8) | 0,
        /*20 (0)  - Black*/          (0 << 16) | (0 << 8) | 0,
        /*21 (2)  - Bright Blue*/    (0 << 16) | (19 << 8) | 0,
        /*22 (9)  - Green*/          (0 << 16) | (85 << 8) | 0,
        /*23 (11) - Sky Blue*/       (0 << 16) | (104 << 8) | 0,
        /*24 (4)  - Magenta*/        (0 << 16) | (38 << 8) | 0,
        /*25 (22) - Pastel Green*/   (0 << 16) | (207 << 8) | 0,
        /*26 (21) - Lime*/           (0 << 16) | (198 << 8) | 0,
        /*27 (23) - Pastel Cyan*/    (0 << 16) | (217 << 8) | 0,
        /*28 (3)  - Red*/            (0 << 16) | (28 << 8) | 0,
        /*29 (5)  - Mauve*/          (0 << 16) | (47 << 8) | 0,
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
    CGateArray::CGateArray(CMachine *pMachine) : inherited(pMachine)
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

        for (i = 0; i < MAX_NUM_PENS; i++)
        {
            m_anPenColors[i] = i;
        }

        m_nBorderColor = 0;
        m_paCurrentRgbConversionTable = s_aRgbConversionTable_Color;
        m_eRequestedScreenMode = SCREEN_MODE_1;
        m_eScreenMode = SCREEN_MODE_1;
        m_nSecondaryRamPage = 1;
        m_eRamConfig = RAM_CONFIG_0_1_2_3;
        m_bLowerRomVisible = true;
        m_bUpperRomVisible = false;
        m_nSelectedUpperRom = 0;
        m_nCrtcHSyncCounter = 0;
        m_nCrtcHSyncCountSinceVSync = 0;
        m_bGateArrayHSyncState = false;
        m_nTicksSinceStartOfCrtcHSync = 0;
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
    void CGateArray::OnCrtcHSyncBegin()
    {
        // Set requested screen mode.
        // Note that if a new screen mode has not been requested, this call won't have any effect.
        SetScreenMode(m_eRequestedScreenMode);

        // Note about HSYNC:
        // The Gate-Array modifies the signal from the CRTC before sending it to the monitor. See Run() function.
        m_nTicksSinceStartOfCrtcHSync = -1;    // Initialize to -1 so that, in the next call to Run (in this same clock tick),
                                               // it will be incremented to 0, which is the value we actually want.
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::OnCrtcHSyncEnd()
    {
        // Pass it on to the video output.
        GetMachine()->GetVideoOutput()->OnHSyncEnd();
        // Interrupt generation logic.
        // +- Increment the 6-bit counter
        m_nCrtcHSyncCounter = (m_nCrtcHSyncCounter + 1) & 0x3F;
        m_nCrtcHSyncCountSinceVSync++;
        // +- Is it time to generate an interrupt?
        if (m_nCrtcHSyncCountSinceVSync == 2)   // If it is the 2nd HSYNC after the last VSYNC...
        {
            if (m_nCrtcHSyncCounter >= 32)
            {
                // Request interrupt.
                GetMachine()->GetCpu()->SetInterruptRequestActive(true);
            }
            // Reset counter.
            m_nCrtcHSyncCounter = 0;
        }
        else if (m_nCrtcHSyncCountSinceVSync > 2)
        {
            if (m_nCrtcHSyncCounter >= 52)
            {
                m_nCrtcHSyncCounter = 0;
                GetMachine()->GetCpu()->SetInterruptRequestActive(true);
            }
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::OnCrtcVSyncBegin()
    {
        // Pass it on to the video output.
        GetMachine()->GetVideoOutput()->OnVSyncBegin();
        m_nCrtcHSyncCountSinceVSync = 0;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::OnCrtcVSyncEnd()
    {
        // Pass it on to the video output.
        GetMachine()->GetVideoOutput()->OnVSyncEnd();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::OnInterruptAcknowledge()
    {
        // Clear the interrupt request.
        GetMachine()->GetCpu()->SetInterruptRequestActive(false);
        // Clear top bit (bit 5) of the internal HSYNC counter - This prevents the next interrupt from occuring sooner than 32 HSYNCs.
        m_nCrtcHSyncCounter = (m_nCrtcHSyncCounter & 0x1F);
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::Run(unsigned nMinNumCycles)
    {
        // Process HSYNC and VSYNC from the CRTC and generate the signals sent to the monitor.
        ProcessHSync();
        // Generate the RGB output for the next 16 cycles of a 16MHz clock.
        // These 16 physical pixels will then be fed to the monitor and displayed.
        GeneratePhysicalPixels();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::ProcessHSync()
    {
        // Determine new activation state of HSYNC.
        m_nTicksSinceStartOfCrtcHSync++;
        bool newHSyncState = GetMachine()->GetCrtc()->GetHSyncState() &&
                             (m_nTicksSinceStartOfCrtcHSync >= GATE_ARRAY_HSYNC_DELAY) &&
                             (m_nTicksSinceStartOfCrtcHSync < (GATE_ARRAY_HSYNC_DELAY + GATE_ARRAY_HSYNC_LENGTH));
        // Signal the start/end of HSYNC to the monitor, if required.
        if (newHSyncState != m_bGateArrayHSyncState)
        {
            m_bGateArrayHSyncState = newHSyncState;
            // Notify the monitor.
            if (m_bGateArrayHSyncState)
            {
                GetMachine()->GetVideoOutput()->OnHSyncBegin();
            }
            else
            {
                GetMachine()->GetVideoOutput()->OnHSyncEnd();
            }
        }
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
        if (!(nPort & 0x8000) && (nPort & 0x4000))
        {
            //
            // The two most significant bits (7 and 6) of nValue determine the function to be performed
            //

            switch ((nValue & 0xC0) >> 6)
            {
                case 0:   // Select pen
                {
                    SelectPen(nValue & 0x1F);  // Take bits 4-0 only. If bit 4 is set, the border is selected.
                }
                break;

                case 1:   // Change selected pen color
                {
                    // Bits 4-0 contain the new color index for the selected pen or border
                    if (m_nSelectedPen < MAX_NUM_PENS)
                    {
                        SetSelectedPenColor(nValue & 0x1F);
                    }
                    else
                    {
                        SetBorderColor(nValue & 0x1F);
                    }
                }
                break;

                case 2:   // Change screen mode, ROM visibility and interrupt control
                {
                    // Screen mode (bits 1,0).
                    // It will take effect during the next HSYNC.
                    RequestScreenModeChange((EScreenMode)(nValue & 0x03));

                    // ROM selection (bit 2 - Lower ROM, bit 3 - Upper ROM)
                    SetRomVisibility((nValue & 0x04) == 0, (nValue & 0x08) == 0);

                    // Interrupt control (bit 4).
                    // If set to 1, the m_nCrtcHSyncCounter counter is reset to 0 and the interrupt request is cleared.
                    if (nValue & 0x10)
                    {
                        m_nCrtcHSyncCounter = 0;
                        GetMachine()->GetCpu()->SetInterruptRequestActive(false);
                    }
                }
                break;
            }
        }

        // RAM configuration port?
        if (!(nPort & 0x8000))      // If bit 15 is cleared...
        {
            if (((nValue & 0xC0) >> 6) == 3)
            {
                // Bits 2-0 define one of the eight possible RAM configurations
                // Note: If we wanted to emulate expansion RAMs other than the CPC6128 built-in one, we would have
                //       to look into bits 5-3 which contain the secondary 64k page to use.
                ERamConfig ramConfig = (ERamConfig)(nValue & 0x07);
                SetRamConfiguration(1/*nSecondaryRamPage*/, ramConfig);
            }
        }

        // ROM Select port?
        if (!(nPort & 0x2000))    // If bit 13 is cleared...
        {
            // This selects the upper ROM in use (range &C000-&FFFF), but it still needs to be made visible through the Gate Array.
            // Lower ROM (range &0000-&3fff) cannot be changed, Operating System ROM is the only choice.
            //
            // Every expansion ROM has a 8-bit identifier. BASIC has identifier 0 and AMSDOS has identifier 7.
            // If an attempt to select a ROM that is not connected is made, BASIC is selected.
            SelectUpperRom(nValue);
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
        KMASSERT(m_nSelectedPen < MAX_NUM_PENS);
        KMASSERT(nColorIndex < MAX_NUM_PALETTE_COLORS);
        m_anPenColors[m_nSelectedPen] = nColorIndex;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::SetBorderColor(cpcByte nColorIndex)
    {
        KMASSERT(nColorIndex < MAX_NUM_PALETTE_COLORS);
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
                default:                        KMASSERTM(false, ("Unknown RGB conversion table."));
            }
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::RequestScreenModeChange(EScreenMode eScreenMode)
    {
        m_eRequestedScreenMode = eScreenMode;
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
        m_eRamConfig = eConfig;

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
        SRamConfigEntry &config = s_aRamConfigTable[m_eRamConfig];
        unsigned         i;

        // Write blocks
        for (i = 0; i < 4; i++)
        {
            int nRamPage;
            nRamPage = (config.Range[i].bFromSecondaryPage ? m_nSecondaryRamPage : 0);
            m_apVisibleWriteBlocks[i] = GetMachine()->GetMemory()->GetRamBlock(config.Range[i].nBlockIndex + (nRamPage * 4));
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
        return m_apVisibleReadBlocks[GET_MEMORY_BLOCK_FROM_ADDRESS(nAddress)]->ReadByte(GET_OFFSET_FROM_ADDRESS(nAddress));
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::WriteByteToMemory(cpcWord nAddress, cpcByte nValue)
    {
        // Bits 15,14 of nAddress determine which one of the four visible blocks to use
        // Bits 13-0 of nAddress determine the offset into the selected block
        m_apVisibleWriteBlocks[GET_MEMORY_BLOCK_FROM_ADDRESS(nAddress)]->WriteByte(GET_OFFSET_FROM_ADDRESS(nAddress), nValue);
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CGateArray::GeneratePhysicalPixels()
    {
        // Every cycle of a 1MHz clock, the Gate Array reads 2 bytes of data from RAM at the address generated by the CRTC.
        // Depending on the mode, these 2 bytes contain 4 logical pixels (mode 0), 8 logical pixels (mode 1) or 16 logical pixels (mode 2).
        // Those 4, 8 or 16 logical pixels always correspond to 16 physical pixels by duplicating logical pixels as needed.
        // E.g. in mode 0, each logical pixels is drawn 4 times.
        //
        // I guess this is why the Gate Array uses a 16Mhz clock. Every 1MHz cycle, it needs to change its RGB output signal 16 times.

        const CCrtc* pCrtc = GetMachine()->GetCrtc();
        if (pCrtc->GetDisplayEnabledState())
        {
            // Currently drawing the visible area.
            // Read 2 bytes from RAM at the address generated by the CRTC.
            const CCrtc::SGeneratedAddress& crtcAddress = m_pMachine->GetCrtc()->GetCurrentAddress();
            cpcWord ramAddress = CONVERT_ADDRESS_CRTC_TO_MEMORY(crtcAddress.MA, crtcAddress.RA);
            CMemoryBlock* ramBlock = m_pMachine->GetMemory()->GetRamBlock((ramAddress & 0xC000) >> 14);    // Bits 15,14 of the address determine which RAM block to read from.
            cpcWord videoWord = (ramBlock->ReadByte(ramAddress) << 8) |                                    // Bits 13-0 of the address are the offset into the selected RAM block.
                ramBlock->ReadByte(ramAddress + 1);
            // Generate the 16 physical pixels.
            switch (m_eScreenMode)
            {
                case CGateArray::SCREEN_MODE_0:  GeneratePhysicalPixels_Mode0(videoWord); break;
                case CGateArray::SCREEN_MODE_1:  GeneratePhysicalPixels_Mode1(videoWord); break;
                case CGateArray::SCREEN_MODE_2:  GeneratePhysicalPixels_Mode2(videoWord); break;
                case CGateArray::SCREEN_MODE_3:  KMASSERTM(false, ("Unofficial video mode 3 not implemented.")); break;
            }
        }
        else
        {
            // Currently drawing the border.
            // While HSYNC from the CRTC is active, it outputs black color; otherwise, it outputs the normal border color.
            unsigned colorRgb = GetMachine()->GetCrtc()->GetHSyncState() ? 0 :      // Black output while CRTC's HSYNC is active.
                                m_paCurrentRgbConversionTable[m_nBorderColor];      // Border color.
            for (unsigned i = 0; i < NUM_PHYSICAL_PIXELS_PER_CYCLE; i++)
            {
                m_aPhysicalPixels[i] = colorRgb;
            }
        }
    }

    void CGateArray::GeneratePhysicalPixels_Mode0(cpcWord videoWord)
    {
        // Mode 0 - Each word holds 4 logical pixels. Each logical pixel is drawn 4 times as physical pixels.
        unsigned pixelRgb;
        pixelRgb = GetPenRgb(((videoWord & 0x0200) >> 6) | ((videoWord & 0x2000) >> 11) | ((videoWord & 0x0800) >> 10) | ((videoWord & 0x8000) >> 15));
        m_aPhysicalPixels[0] = m_aPhysicalPixels[1] = m_aPhysicalPixels[2] = m_aPhysicalPixels[3] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0100) >> 5) | ((videoWord & 0x1000) >> 10) | ((videoWord & 0x0400) >> 9) | ((videoWord & 0x4000) >> 14));
        m_aPhysicalPixels[4] = m_aPhysicalPixels[5] = m_aPhysicalPixels[6] = m_aPhysicalPixels[7] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0002) << 2) | ((videoWord & 0x0020) >> 3) | ((videoWord & 0x0008) >> 2) | ((videoWord & 0x0080) >> 7));
        m_aPhysicalPixels[8] = m_aPhysicalPixels[9] = m_aPhysicalPixels[10] = m_aPhysicalPixels[11] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0001) << 3) | ((videoWord & 0x0010) >> 2) | ((videoWord & 0x0004) >> 1) | ((videoWord & 0x0040) >> 6));
        m_aPhysicalPixels[12] = m_aPhysicalPixels[13] = m_aPhysicalPixels[14] = m_aPhysicalPixels[15] = pixelRgb;
    }

    void CGateArray::GeneratePhysicalPixels_Mode1(cpcWord videoWord)
    {
        // Mode 1 - Each word holds 8 logical pixels. Each logical pixel is drawn twice as physical pixels.
        unsigned pixelRgb;
        pixelRgb = GetPenRgb(((videoWord & 0x0800) >> 10) | ((videoWord & 0x8000) >> 15));
        m_aPhysicalPixels[0] = m_aPhysicalPixels[1] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0400) >> 9) | ((videoWord & 0x4000) >> 14));
        m_aPhysicalPixels[2] = m_aPhysicalPixels[3] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0200) >> 8) | ((videoWord & 0x2000) >> 13));
        m_aPhysicalPixels[4] = m_aPhysicalPixels[5] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0100) >> 7) | ((videoWord & 0x1000) >> 12));
        m_aPhysicalPixels[6] = m_aPhysicalPixels[7] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0008) >> 2) | ((videoWord & 0x0080) >> 7));
        m_aPhysicalPixels[8] = m_aPhysicalPixels[9] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0004) >> 1) | ((videoWord & 0x0040) >> 6));
        m_aPhysicalPixels[10] = m_aPhysicalPixels[11] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0002)) | ((videoWord & 0x0020) >> 5));
        m_aPhysicalPixels[12] = m_aPhysicalPixels[13] = pixelRgb;

        pixelRgb = GetPenRgb(((videoWord & 0x0001) << 1) | ((videoWord & 0x0010) >> 4));
        m_aPhysicalPixels[14] = m_aPhysicalPixels[15] = pixelRgb;
    }

    void CGateArray::GeneratePhysicalPixels_Mode2(cpcWord videoWord)
    {
        // Mode 2 - Each word holds 16 logical pixels.
        m_aPhysicalPixels[0] = GetPenRgb((videoWord & 0x8000) >> 15);
        m_aPhysicalPixels[1] = GetPenRgb((videoWord & 0x4000) >> 14);
        m_aPhysicalPixels[2] = GetPenRgb((videoWord & 0x2000) >> 13);
        m_aPhysicalPixels[3] = GetPenRgb((videoWord & 0x1000) >> 12);
        m_aPhysicalPixels[4] = GetPenRgb((videoWord & 0x0800) >> 11);
        m_aPhysicalPixels[5] = GetPenRgb((videoWord & 0x0400) >> 10);
        m_aPhysicalPixels[6] = GetPenRgb((videoWord & 0x0200) >> 9);
        m_aPhysicalPixels[7] = GetPenRgb((videoWord & 0x0100) >> 8);
        m_aPhysicalPixels[8] = GetPenRgb((videoWord & 0x0080) >> 7);
        m_aPhysicalPixels[9] = GetPenRgb((videoWord & 0x0040) >> 6);
        m_aPhysicalPixels[10] = GetPenRgb((videoWord & 0x0020) >> 5);
        m_aPhysicalPixels[11] = GetPenRgb((videoWord & 0x0010) >> 4);
        m_aPhysicalPixels[12] = GetPenRgb((videoWord & 0x0008) >> 3);
        m_aPhysicalPixels[13] = GetPenRgb((videoWord & 0x0004) >> 2);
        m_aPhysicalPixels[14] = GetPenRgb((videoWord & 0x0002) >> 1);
        m_aPhysicalPixels[15] = GetPenRgb((videoWord & 0x0001) >> 0);
    }

} //namespace CPC
