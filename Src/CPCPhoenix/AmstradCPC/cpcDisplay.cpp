//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcDisplay.h"
#include "cpcMachine.h"
#include "cpcMemory.h"
#include "cpcMemoryBlock.h"
#include "cpcGateArray.h"


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


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CDisplay::CDisplay(CMachine *pMachine) : inherited( pMachine )
  {
    m_bScanLineEffectActivated = false;
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CDisplay::ResetVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CDisplay::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CDisplay::Reset()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CDisplay::DecodeImage_B8G8R8X8(unsigned char* pImageBuffer)
  {
    if (pImageBuffer != NULL)
    {
      CCrtc* pCrtc;
      pCrtc = GetMachine()->GetCrtc();

      CGateArray* pGateArray;
      pGateArray = GetMachine()->GetGateArray();

      const CCrtc::SGeneratedAddress* pCrtcAddressTable;
      pCrtcAddressTable = pCrtc->GetGeneratedAddressTable();

      unsigned* pDestPixel;
      pDestPixel = (unsigned*) pImageBuffer;

      unsigned nScanLineCount;
      nScanLineCount = pCrtc->GetRegisterValue(CCrtc::VERTICAL_DISPLAYED) * (pCrtc->GetRegisterValue(CCrtc::MAXIMUM_RASTER_ADDRESS) + 1);
      nScanLineCount = ( nScanLineCount<=(IMAGEBUFFER_HEIGHT/2) ? nScanLineCount : IMAGEBUFFER_HEIGHT/2 );  // Limit to 200 scan lines

      unsigned nCurrScanLine;
      for (nCurrScanLine = 0; nCurrScanLine < nScanLineCount; nCurrScanLine++)
      {
        const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress = pCrtcAddressTable[nCurrScanLine];

        // Draw each scan line pixel, taking screen mode into account
        switch (pGateArray->GetScreenMode())
        {
          case CGateArray::SCREEN_MODE_0:   // 160x200 resolution, 16 colors (4bpp)
          {
            pDestPixel = DecodeScanLine_B8G8R8X8_Mode0( pDestPixel, scanLineStartCrtcAddress );
          }
          break;

          case CGateArray::SCREEN_MODE_1:   // 320x200 resolution, 4 colors (2bpp)
          {
            pDestPixel = DecodeScanLine_B8G8R8X8_Mode1( pDestPixel, scanLineStartCrtcAddress );
          }
          break;

          case CGateArray::SCREEN_MODE_2:   // 640x200 resolution, 2 colors (1bpp)
          {
            pDestPixel = DecodeScanLine_B8G8R8X8_Mode2( pDestPixel, scanLineStartCrtcAddress );
          }
          break;

          case CGateArray::SCREEN_MODE_3:   // 160x200 resolution, 4 colors (2bpp) (unofficial)  --  TODO
          {
            KMASSERTM( false, ("Unofficial video mode 3 not implemented.") );
          }
          break;
        }

        if (m_bScanLineEffectActivated)
        {
          // Scan line effect
          unsigned x;
          static const unsigned SCAN_LINE_EFFECT_COLOR = 0xFF000000;
          for (x = 0; x < IMAGEBUFFER_WIDTH; x++)
          {
            *pDestPixel = SCAN_LINE_EFFECT_COLOR;
            pDestPixel++;
          }
        }
        else
        {
          // Duplicate previous scan line
          unsigned* pSrcPixel;
          unsigned  x;
          pSrcPixel = pDestPixel - IMAGEBUFFER_WIDTH;

          for (x = 0; x < IMAGEBUFFER_WIDTH; x++)
          {
            *pDestPixel = *pSrcPixel;
            pSrcPixel++;
            pDestPixel++;
          }
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  unsigned* CDisplay::DecodeScanLine_B8G8R8X8_Mode0(unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress)
  {
    CMemory* pMemory;
    pMemory = GetMachine()->GetMemory();

    CGateArray* pGateArray;
    pGateArray = GetMachine()->GetGateArray();

    CCrtc* pCrtc;
    pCrtc = GetMachine()->GetCrtc();

    unsigned nPixelRgb;
    cpcWord nSrcAddress;
    cpcWord nVideoWord;
    CMemoryBlock* pRamBlock;
    unsigned nCurrCharacter;
    for (nCurrCharacter = 0; nCurrCharacter < pCrtc->GetRegisterValue(CCrtc::HORIZONTAL_DISPLAYED); nCurrCharacter++)
    {
      // Two bytes are read per character
      nSrcAddress = CONVERT_ADDRESS_CRTC_TO_MEMORY( scanLineStartCrtcAddress.MA + nCurrCharacter, scanLineStartCrtcAddress.RA );
      pRamBlock   = pMemory->GetRamBlock( (nSrcAddress & 0xC000) >> 14 );    // Bits 15,14 of the address determine which RAM block to read from.

      nVideoWord = (pRamBlock->ReadByte(nSrcAddress) << 8) |                 // Only bits 13-0 of address will be taken into account.
                    pRamBlock->ReadByte(nSrcAddress + 1);                    // Only bits 13-0 of address will be taken into account.

      // Decode pixels and write them to the destination buffer.
      // Mode 0 - Each word holds 4 pixels. Additionally, we write each pixel four times in the output image buffer
      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0200) >> 6) | ((nVideoWord&0x2000) >> 11)  | ((nVideoWord&0x0800) >> 10)  | ((nVideoWord&0x8000) >> 15) );
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0100) >> 5) | ((nVideoWord&0x1000) >> 10) | ((nVideoWord&0x0400) >> 9) | ((nVideoWord&0x4000) >> 14)  );
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;   // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;


      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0002) << 2) | ((nVideoWord&0x0020) >> 3)  | ((nVideoWord&0x0008) >> 2)  | ((nVideoWord&0x0080) >> 7) );
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0001) << 3) | ((nVideoWord&0x0010) >> 2) | ((nVideoWord&0x0004) >> 1) | ((nVideoWord&0x0040) >> 6)  );
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;   // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb; *pDestPixel++ = nPixelRgb;
    }

    return pDestPixel;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  unsigned* CDisplay::DecodeScanLine_B8G8R8X8_Mode1(unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress)
  {
    CMemory* pMemory;
    pMemory = GetMachine()->GetMemory();

    CGateArray* pGateArray;
    pGateArray = GetMachine()->GetGateArray();

    CCrtc* pCrtc;
    pCrtc = GetMachine()->GetCrtc();

    unsigned nPixelRgb;
    cpcWord nSrcAddress;
    cpcWord nVideoWord;
    CMemoryBlock* pRamBlock;
    unsigned nCurrCharacter;
    for (nCurrCharacter = 0; nCurrCharacter < pCrtc->GetRegisterValue(CCrtc::HORIZONTAL_DISPLAYED); nCurrCharacter++)
    {
      // Two bytes are read per character
      nSrcAddress = CONVERT_ADDRESS_CRTC_TO_MEMORY( scanLineStartCrtcAddress.MA + nCurrCharacter, scanLineStartCrtcAddress.RA );
      pRamBlock   = pMemory->GetRamBlock( (nSrcAddress & 0xC000) >> 14 );    // Bits 15,14 of the address determine which RAM block to read from.

      nVideoWord = (pRamBlock->ReadByte(nSrcAddress) << 8) |                 // Only bits 13-0 of address will be taken into account.
                    pRamBlock->ReadByte(nSrcAddress + 1);                    // Only bits 13-0 of address will be taken into account.

      // Decode pixels and write them to the destination buffer.
      // Mode 1 - Each word holds 8 pixels. Additionally, we write each pixel twice in the output image buffer
      nPixelRgb     = pGateArray->GetPenRgb( ((nVideoWord&0x0800) >> 10) | ((nVideoWord&0x8000) >> 15) );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0400) >> 9) | ((nVideoWord&0x4000) >> 14)  );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0200) >> 8) | ((nVideoWord&0x2000) >> 13)  );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0100) >> 7) | ((nVideoWord&0x1000) >> 12)  );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;


      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0008) >> 2) | ((nVideoWord&0x0080) >> 7) );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0004) >> 1) | ((nVideoWord&0x0040) >> 6)  );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0002)     ) | ((nVideoWord&0x0020) >> 5)  );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;

      nPixelRgb = pGateArray->GetPenRgb( ((nVideoWord&0x0001) << 1) | ((nVideoWord&0x0010) >> 4)  );
      *pDestPixel++ = nPixelRgb;    // Write pixel and advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
      *pDestPixel++ = nPixelRgb;
    }

    return pDestPixel;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  unsigned* CDisplay::DecodeScanLine_B8G8R8X8_Mode2(unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress)
  {
    CMemory* pMemory;
    pMemory = GetMachine()->GetMemory();

    CGateArray* pGateArray;
    pGateArray = GetMachine()->GetGateArray();

    CCrtc* pCrtc;
    pCrtc = GetMachine()->GetCrtc();

    cpcWord nSrcAddress;
    cpcWord nVideoWord;
    CMemoryBlock* pRamBlock;
    unsigned nCurrCharacter;
    for (nCurrCharacter = 0; nCurrCharacter < pCrtc->GetRegisterValue(CCrtc::HORIZONTAL_DISPLAYED); nCurrCharacter++)
    {
      // Two bytes are read per character
      nSrcAddress = CONVERT_ADDRESS_CRTC_TO_MEMORY( scanLineStartCrtcAddress.MA + nCurrCharacter, scanLineStartCrtcAddress.RA );
      pRamBlock   = pMemory->GetRamBlock( (nSrcAddress & 0xC000) >> 14 );    // Bits 15,14 of the address determine which RAM block to read from.

      nVideoWord = (pRamBlock->ReadByte(nSrcAddress) << 8) |                 // Only bits 13-0 of address will be taken into account.
                    pRamBlock->ReadByte(nSrcAddress + 1);                    // Only bits 13-0 of address will be taken into account.

      // Decode pixels and write them to the destination buffer.
      // Mode 2 - Each word holds 16 pixels.
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x8000) >> 15 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x4000) >> 14 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x2000) >> 13 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x1000) >> 12 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0800) >> 11 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0400) >> 10 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0200) >>  9 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0100) >>  8 );

      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0080) >> 7 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0040) >> 6 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0020) >> 5 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0010) >> 4 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0008) >> 3 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0004) >> 2 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0002) >> 1 );
      *pDestPixel++ = pGateArray->GetPenRgb( (nVideoWord&0x0001) >> 0 );
    }

    return pDestPixel;
  }

} //namespace CPC
