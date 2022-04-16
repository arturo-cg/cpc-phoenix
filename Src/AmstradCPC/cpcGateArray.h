//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCGATEARRAY_H_
#define _CPCGATEARRAY_H_


#include "cpcSubSystem.h"
#include "cpcMemory.h"


namespace CPC {

    class CMemoryBlock;


    /**
    ** The Gate-Array is a custom chip which controls several things: pen colors, screen mode,
    ** ROM visibility, RAM configuration(*) and interrupt control.
    **
    ** (*) In the real Amstrad CPC, RAM configuration is not implemented in the Gate-Array itself but in a separate
    **     chip or integrated into another one. However that chip is mapped to the same port and uses a function code
    **     that the Gate-Array doesn't use so they both work together as if they were one.
    */
    class CGateArray : public CSubSystem
    {
    public:

        enum EScreenMode
        {
            // DO NOT change the integer value of each item, it is a direct mapping to the CPC hardware values
            SCREEN_MODE_0 = 0,   // 160x200 resolution, 16 colors
            SCREEN_MODE_1 = 1,   // 320x200 resolution, 4 colors
            SCREEN_MODE_2 = 2,   // 640x200 resolution, 2 colors
            SCREEN_MODE_3 = 3,   // 160x200 resolution, 4 colors (unofficial)
        };

        enum ERgbConversionTableType
        {
            RGBCONVERSIONTABLE_COLOR = 0,     // Simulates a color monitor.
            RGBCONVERSIONTABLE_GREEN,         // Simulates a green monitor.

            RGBCONVERSIONTABLE_LAST,
            RGBCONVERSIONTABLE_INVALID = 0x7FFFFFFF,
        };

        enum ERamConfig
        {
            RAM_CONFIG_0_1_2_3 = 0,
            RAM_CONFIG_0_1_2_3s = 1,
            RAM_CONFIG_0s_1s_2s_3s = 2,
            RAM_CONFIG_0_3_2_3s = 3,
            RAM_CONFIG_0_0s_2_3 = 4,
            RAM_CONFIG_0_1s_2_3 = 5,
            RAM_CONFIG_0_2s_2_3 = 6,
            RAM_CONFIG_0_3s_2_3 = 7,
        };

        struct Snapshot
        {
            cpcByte selectedPen;            // Index of the selected pen.
            EScreenMode screenMode;         // Screen mode.
            bool lowerRomVisible;           // Lower ROM visibility.
            bool upperRomVisible;           // Upper ROM visibility;
            bool interruptControlState;     // Interrupt Control bit.
            cpcByte penColors[16];          // Pen colors.
            cpcByte borderColor;            // Border color.
            ERamConfig ramConfig;           // RAM configuration.
            unsigned secondaryRamPage;      // Selected secondary RAM page.
            cpcByte selectedUpperRom;       // Selected upper ROM.
        };

        static const unsigned   MAX_NUM_PENS = 16;
        static const unsigned   MAX_NUM_PALETTE_COLORS = 32;
        static const unsigned   NUM_PHYSICAL_PIXELS_PER_CYCLE = 16;   // Number of physical pixels per cycle of a 1MHz clock.


        CGateArray(CMachine *pMachine);
        virtual                ~CGateArray() { FreeVars(); }

        /** Resets the subsystem. */
        virtual void            Reset();

        /** Takes a snapshot of the current state of the device. */
        void                    TakeSnapshot(Snapshot* snapshot) const;
        /** Applies the values from the specified snaphot. */
        void                    ApplySnapshot(const Snapshot& snapshot);

        /** Returns the current screen mode. */
        EScreenMode             GetScreenMode() const { return m_eScreenMode; }

        /** Sets the RGB conversion table to use. It allows to simulate a color or a green monitor. */
        void                    SetRgbConversionTable(ERgbConversionTableType eTableType);
        /** Gets the RGB conversion table currently in use. */
        ERgbConversionTableType GetRgbConversionTable() const { return m_eRgbConversionTableType; }

        /** Returns the current RGB of the specified pen. */
        unsigned                GetPenRgb(cpcByte nPen) const { return m_paCurrentRgbConversionTable[m_anPenColors[nPen]] | m_alphaMask; }
        /** Returns the current RGB of the border. */
        unsigned                GetBorderRgb() const { return m_paCurrentRgbConversionTable[m_nBorderColor] | m_alphaMask; }

        /** Sets the color alpha value. All the RGB values returned by the CGateArray will have this value in their alpha channel.
        *** The emulator core doesn't use or care about this value; this is useful to the emulator front-end only. */
        void                    SetColorAlpha(cpcByte alpha) { m_alphaMask = (unsigned(alpha) << 24); }
        /** Returns the color alpha value. */
        cpcByte                 GetColorAlpha(cpcByte alpha) const { return cpcByte(m_alphaMask >> 24); }

        /** Returns true if the HSYNC output from the Gate-Array is currently active, or false otherwise. */
        bool                    IsHSyncActive() const { return m_bMonitorHSyncState; }
        /** Returns true if the VSYNC output from the Gate-Array is currently active, or false otherwise. */
        bool                    IsVSyncActive() const { return m_bMonitorVSyncState; }

        /** Returns the lastest batch of physical pixels generated by the Gate Array. */
        const unsigned*         GetGeneratedPhysicalPixels() const { return m_aPhysicalPixels; }

        /** Reads a byte from memory at the specified address.
        *** The CPU doesn't access memory directly. Instead, it goes through the Gate Array which provides RAM paging. */
        cpcByte                 ReadByteFromMemory(cpcWord nAddress) const;
        /** Writes a byte to memory at the specified address.
        *** The CPU doesn't access memory directly. Instead, it goes through the Gate Array which provides RAM paging. */
        void                    WriteByteToMemory(cpcWord nAddress, cpcByte nValue);

        /** CRTC's HSYNC signal rising edge notification. */
        void                    OnCrtcHSyncBegin();
        /** CRTC's HSYNC signal falling edge notification. */
        void                    OnCrtcHSyncEnd();
        /** CRTC's VSYNC signal rising edge notification. */
        void                    OnCrtcVSyncBegin();
        /** CRTC's VSYNC signal falling edge notification. */
        void                    OnCrtcVSyncEnd();
        /** The CPU just accepted the Gate Array's interrupt request. */
        void                    OnInterruptAcknowledge();

        /** We are notified that another subsytem is trying to write a byte to us.
        *** Usually it's the CPU through an OUT instruction. */
        virtual void            RespondToWritePortRequest(cpcWord nPort, cpcByte nValue);

        /** Runs the Gate-Array for the given number of cycles. */
        void                    Run(unsigned nMinNumCycles);

        /** Static utility method to obtain a memory address on a CPC system from a CRTC address. */
        static cpcWord          ConvertCrtcAddressToCpc(cpcWord ma, cpcByte ra)
        {
            return (((ma & 0x3000) << 2) |
                    ((ra & 0x0007) << 11) |
                    ((ma & 0x03FF) << 1));
        }

    private:

        typedef                 CSubSystem                inherited;


        static const unsigned GATE_ARRAY_HSYNC_DELAY = 2;   // Delay relative to the start of CRTC's HSYNC, in 1Mhz clock ticks.
        static const unsigned GATE_ARRAY_HSYNC_LENGTH = 4;  // In 1Mhz clock ticks.

        static const unsigned GATE_ARRAY_VSYNC_DELAY = 2;   // Delay relative to the start of CRTC's VSYNC, in scan lines.
        static const unsigned GATE_ARRAY_VSYNC_LENGTH = 2;  // In scan lines.


        void                    ResetVars();
        void                    FreeVars();

        void                    SelectPen(cpcByte nPen);
        void                    SetSelectedPenColor(cpcByte nColorIndex);
        void                    SetBorderColor(cpcByte nColorIndex);
        void                    RequestScreenModeChange(EScreenMode eScreenMode);
        void                    SetScreenMode(EScreenMode eScreenMode);
        void                    SetRamConfiguration(ERamConfig eConfig, unsigned nSecondaryPage);
        void                    SetRomVisibility(bool bLowerRomVisible, bool bUpperRomVisible);
        void                    SelectUpperRom(cpcByte nIndex);

        void                    UpdateVisibleMemoryBlocks();

        void                    ProcessHSync();
        void                    ProcessVSync();

        void                    OnMonitorHSyncBegin();
        void                    OnMonitorHSyncEnd();
        void                    OnMonitorVSyncBegin();
        void                    OnMonitorVSyncEnd();

        void                    GeneratePhysicalPixels();
        void                    GeneratePhysicalPixels_Mode0(cpcWord videoWord);
        void                    GeneratePhysicalPixels_Mode1(cpcWord videoWord);
        void                    GeneratePhysicalPixels_Mode2(cpcWord videoWord);

        /** Currently selected pen. This is the pen that will be changed on the next "change pen color" operation.
        *** If this value is >= 16, the border is selected instead of a pen. */
        int                     m_nSelectedPen;

        /** Pen color table. Each entry is an index into the hardware color palette. */
        int                     m_anPenColors[MAX_NUM_PENS];
        /** Border color. It is an index into the hardware color palette. */
        int                     m_nBorderColor;
        /** RGB conversion table. It allows to simulate a color or a green monitor. */
        ERgbConversionTableType m_eRgbConversionTableType;
        const unsigned*         m_paCurrentRgbConversionTable;
        unsigned                m_alphaMask;

        /** Screen mode. When this value is changed, it won't take effect until the next HSYNC. */
        EScreenMode             m_eRequestedScreenMode;
        EScreenMode             m_eScreenMode;

        /** The blocks that are visible for read operations (can be either ROM or RAM blocks). */
        const CMemoryBlock     *m_apVisibleReadBlocks[4];
        /** The blocks that are visible for write operations (always RAM blocks). */
        CMemoryBlock           *m_apVisibleWriteBlocks[4];

        /** Which secondary 64K RAM page to use. */
        unsigned                m_nSecondaryRamPage;
        /** The RAM configuration. */
        ERamConfig              m_eRamConfig;

        /** Whether the lower ROM (Operating System ROM) is visible in the range &0000-&3FFF or not. */
        bool                    m_bLowerRomVisible;
        /** Whether the upper ROM (BASIC, AMSDOS or expansion ROM) is visible in the range &C000-&FFFF or not. */
        bool                    m_bUpperRomVisible;
        /** The upper ROM (range &C000-&FFFF) currently selected. */
        cpcByte                 m_nSelectedUpperRom;

        /** 6-bit counter related to the HSYNC signal from the CRTC, used to generate interrupts. */
        unsigned                m_nCrtcHSyncCounter;
        unsigned                m_nCrtcHSyncCountSinceVSync;
        unsigned                m_nMonitorHSyncCountSinceVSync;
        int                     m_nTicksSinceStartOfCrtcHSync;     // 1ms ticks since the start of the last CRTC's HSYNC.

        bool                    m_bMonitorHSyncState;
        bool                    m_bMonitorVSyncState;

        /** */
        unsigned                m_aPhysicalPixels[NUM_PHYSICAL_PIXELS_PER_CYCLE];

    };


} //namespace CPC

#endif // _CPCGATEARRAY_H_
