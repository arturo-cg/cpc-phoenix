//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCGATEARRAY_H_
#define _CPCGATEARRAY_H_


#include "cpcSubSystem.h"


namespace CPC {


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

                            CGateArray                (CMachine *pMachine);
    virtual                ~CGateArray                ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** We are notified that another subsytem is trying to write a byte to us.
    *** Usually it's the CPU through an OUT instruction. */
    virtual void            RespondToWritePortRequest (cpcWord nPort, cpcByte nValue);

    /** Runs the Gate-Array for the given number of cycles. */
    void                    Run                       (unsigned nMinNumCycles);


private:

    typedef                 CSubSystem                inherited;


    enum
    {
        MAX_NUM_PENS           = 16,
        MAX_NUM_PALETTE_COLORS = 32,
    };

    enum TScreenMode
    {
        // DO NOT change the integer value of each item, it is a direct mapping to the CPC hardware values
        SCREEN_MODE_0 = 0,   // 160x200 resolution, 16 colors
        SCREEN_MODE_1 = 1,   // 320x200 resolution, 4 colors
        SCREEN_MODE_2 = 2,   // 640x200 resolution, 2 colors
        SCREEN_MODE_3 = 3,   // 160x200 resolution, 4 colors (unofficial)
    };


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    void                    SelectPen                 (cpcByte nPen);
    void                    SetSelectedPenColor       (cpcByte nColorIndex);
    void                    SetBorderColor            (cpcByte nColorIndex);
    void                    SetScreenMode             (TScreenMode eScreenMode);
    void                    SetRomVisibility          (bool bLowerRomVisible, bool bUpperRomVisible);


    /** Currently selected pen. This is the pen that will be changed on the next "change pen color" operation.
    *** If this value is >= 16, the border is selected instead of a pen. */
    int                     m_nSelectedPen;

    /** Pen color table. Each entry is an index into the hardware color palette. */
    int                     m_anPenColors[MAX_NUM_PENS];
    /** Border color. It is an index into the hardware color palette. */
    int                     m_nBorderColor;

    /** Screen mode. When this value is changed, it won't take effect until the next HSYNC. */
    TScreenMode             m_eScreenMode;

// --- This is kept in CMemory ---
//     /** If true, lower ROM (operating system ROM) is mapped into memory at &0000-&3FFF. */
//     bool                    m_bLowerRomVisible;
//     /** If true, upper ROM (BASIC or expansion ROM) is mapped into memory at &C000-&FFFF. */
//     bool                    m_bUpperRomVisible;

};


} //namespace CPC

#endif // _CPCGATEARRAY_H_
