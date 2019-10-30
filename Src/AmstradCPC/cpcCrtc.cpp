//-------------------------------------------------------------------------------------------
// File:        Crtc.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcCrtc.h"
#include "cpcMachine.h"
#include "cpcGateArray.h"


namespace CPC {

    /*static*/ const cpcByte CCrtc::s_registerMasks[NUM_REGISTERS] = {
        0xFF,   // R0
        0xFF,   // R1
        0xFF,   // R2
        0xFF,   // R3
        0x7F,   // R4
        0x1F,   // R5
        0x7F,   // R6
        0x7F,   // R7
        0x03,   // R8
        0x1F,   // R9
        0x7F,   // R10
        0x1F,   // R11
        0x3F,   // R12
        0xFF,   // R13
        0x3F,   // R14
        0xFF,   // R15
        0x3F,   // R16
        0xFF,   // R17
    };


    //----------------------------------------------------------------------------
    /**
    **
    */
    CCrtc::CCrtc(CMachine *pMachine) : inherited(pMachine)
    {
        Reset();
    }

    //----------------------------------------------------------------------------
    /**
    ** ResetVars
    */
    void CCrtc::ResetVars()
    {
        m_eSelectedRegister = HORIZONTAL_TOTAL;
        m_nCurrentHCharacter = 0;
        m_nCurrentVCharacter = 0;
        m_nCurrentScanLine = 0;
        m_nExtraScanLinesCounter = 0;
        m_nScanLinesForVSyncOff = 0;
        m_bDisplayEnabledH = true;
        m_bDisplayEnabledV = true;
        m_bHSyncState = false;
        m_bVSyncState = false;

        for (int i = 0; i < NUM_REGISTERS; i++)
        {
            m_anRegisters[i] = 0;
        }
        m_anRegisters[HORIZONTAL_TOTAL] = 63;
        m_anRegisters[VERTICAL_TOTAL] = 38;
    }

    //----------------------------------------------------------------------------
    /**
    ** FreeVars
    */
    void CCrtc::FreeVars()
    {
        //...
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ void CCrtc::Reset()
    {
        // Reset members
        ResetVars();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CCrtc::SelectRegister(ERegister eRegister)
    {
        if (eRegister != INVALID_REGISTER)
        {
            m_eSelectedRegister = eRegister;
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CCrtc::WriteSelectedRegister(cpcByte nValue)
    {
        KMASSERT(m_eSelectedRegister < NUM_REGISTERS);
        m_anRegisters[m_eSelectedRegister] = nValue & s_registerMasks[m_eSelectedRegister];
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CCrtc::Run(unsigned nNumCycles)
    {
        for (unsigned i = 0; i < nNumCycles; i++)
        {
            // Advance 1 character per 1us cycle.
            UpdateHorizontal();
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CCrtc::UpdateHorizontal()
    {
        cpcByte nHorizontalTotal = m_anRegisters[HORIZONTAL_TOTAL] + 1;
        cpcByte nHorizontalSyncOff = m_anRegisters[HORIZONTAL_SYNC_POSITION] + (m_anRegisters[SYNC_WIDTHS] & 0x0F);

        // Advance 1 character.
        m_nCurrentHCharacter++;
        m_currentAddress.MA++;

        // Update signals depending on where we are in the scan line.
        if (m_nCurrentHCharacter == nHorizontalTotal)
        {
            // At this point, monitor raster is right past the left border.
            // Gate-Array starts reading bytes from RAM to generate video signal (if vertical position is in visible area too).
            // Re-enable display.
            m_bDisplayEnabledH = true;
            // Move to next scan line.
            m_nCurrentHCharacter = 0;
            m_currentAddress.MA -= nHorizontalTotal;
            UpdateVertical();
        }

        if (m_nCurrentHCharacter == m_anRegisters[HORIZONTAL_DISPLAYED])
        {
            // Disable display.
            // Display is enabled (horizontally) if character column is in the range [0, HORIZONTAL_DISPLAYED).
            // When enabled, Gate-Array reads bytes from RAM to generate video signal.
            // When disabled, Gate-Array uses border color to generate video signal.
            m_bDisplayEnabledH = false;
        }

        if (m_nCurrentHCharacter == m_anRegisters[HORIZONTAL_SYNC_POSITION])     // At HSYNC's rising edge?
        {
            // Monitor starts moving its beam to the beginning of next raster line.
            m_bHSyncState = true;
            // Notify the Gate Array that HSYNC's rising edge just occured.
            GetMachine()->GetGateArray()->OnCrtcHSyncBegin();
        }

        if (m_nCurrentHCharacter == nHorizontalSyncOff)    // At HSYNC's falling edge?
        {
            // Monitor starts rasterizing next raster line (note that the CRTC remains on the current scan line for a few more characters).
            // DISPLAY_ENABLED signal is still OFF, which means the left border is starting to be rasterized.
            m_bHSyncState = false;
            // Notify the Gate Array that HSYNC's falling edge just occured. The Gate Array uses HSYNC and VSYNC to generate interrupts.
            GetMachine()->GetGateArray()->OnCrtcHSyncEnd();
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CCrtc::UpdateVertical()
    {
        if (m_nExtraScanLinesCounter <= 0)
        {
            // Advance 1 scan line.
            m_nCurrentScanLine++;
            m_currentAddress.RA++;

            // If VSYNC is active, check whether it is time for it to go inactive.
            if (m_bVSyncState)
            {
                m_nScanLinesForVSyncOff--;
                if (m_nScanLinesForVSyncOff == 0)
                {
                    // Monitor starts rasterizing top raster line (note that CRTC doesn't reset character row count yet).
                    // Also, DISPLAY_ENABLED signal is still OFF, which means the top border is starting to be rasterized.
                    m_bVSyncState = false;
                    // Notify the Gate Array that VSYNC's falling edge just occured. The Gate Array uses HSYNC and VSYNC to generate interrupts.
                    GetMachine()->GetGateArray()->OnCrtcVSyncEnd();
                    // Add extra scan lines, if requested.
                    m_nExtraScanLinesCounter = m_anRegisters[VERTICAL_TOTAL_ADJUST] & 0x1F;
                }
            }

            // Is it time to advance to the next character row?
            cpcByte nMaximumScanLineAddress = m_anRegisters[MAXIMUM_SCAN_LINE_ADDRESS] + 1;
            if (m_nCurrentScanLine == nMaximumScanLineAddress)
            {
                cpcByte nVerticalTotal = m_anRegisters[VERTICAL_TOTAL] + 1;

                // Advance 1 character row.
                m_nCurrentVCharacter++;
                m_currentAddress.MA += m_anRegisters[HORIZONTAL_DISPLAYED];
                m_currentAddress.RA = 0;
                m_nCurrentScanLine = 0;

                // Update signals depending on where we are in the frame.
                if (m_nCurrentVCharacter == nVerticalTotal)    // At start of new CRTC frame?
                {
                    // At this point, monitor raster is right past the top border vertically and right past the left border horizontally.
                    // Re-enable display.
                    m_bDisplayEnabledV = true;
                    m_nCurrentVCharacter = 0;
                    m_currentAddress.MA = (m_anRegisters[START_ADDRESS_HIGH] << 8) | m_anRegisters[START_ADDRESS_LOW];
                }

                if (m_nCurrentVCharacter == m_anRegisters[VERTICAL_DISPLAYED])
                {
                    // Disable display.
                    m_bDisplayEnabledV = false;
                }

                if (m_nCurrentVCharacter == m_anRegisters[VERTICAL_SYNC_POSITION])    // At VSYNC rising edge?
                {
                    // Monitor starts moving its beam to the beginning of top raster line.
                    m_bVSyncState = true;
                    m_nScanLinesForVSyncOff = (m_anRegisters[SYNC_WIDTHS] & 0xF0) >> 4;
                    if (m_nScanLinesForVSyncOff == 0)
                    {
                        m_nScanLinesForVSyncOff = 16;
                    }
                    // Notify the Gate Array that VSYNC's rising edge just occured.
                    GetMachine()->GetGateArray()->OnCrtcVSyncBegin();
                }
            }
        }
        else
        {
            // We are adding extra scan lines at the start of the frame (i.e. right after VSYNC goes off) as per the value contained in R5 (VERTICAL TOTAL ADJUST).
            m_nExtraScanLinesCounter--;
        }
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
        if (!(nPort & 0x4000))    // If bit 14 is cleared...
        {
            // Bits 9,8 select the function:
            //   0,0 --> Register select
            //   0,1 --> Register write
            //   1,0 --> *Read-only* (depends on the model of the 6845 chip)
            //   1,1 --> *Read-only* (depends on the model of the 6845 chip)

            switch ((nPort & 0x0300) >> 8)
            {
                // Register select
                case 0:     SelectRegister(nValue < NUM_REGISTERS ? (ERegister)nValue : INVALID_REGISTER); break;
                    // Register write
                case 1:     WriteSelectedRegister(nValue); break;
                    // *Read-only* (depends on the model of the 6845 chip)
                case 2:     /* ... */; break;
                    // *Read-only* (depends on the model of the 6845 chip)
                case 3:     /* ... */; break;
            }
        }
    }

} //namespace CPC
