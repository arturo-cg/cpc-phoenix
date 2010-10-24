//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcDisplay.h"
#include "cpcMachine.h"
#include "cpcMemory.h"
#include "cpcMemoryBlock.h"
#include "cpcGateArray.h"
#include "cpcCrtc.h"


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
    //...
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
      CMemory* pMemory;
      pMemory = GetMachine()->GetMemory();

      CGateArray* pGateArray;
      pGateArray = GetMachine()->GetGateArray();

      CCrtc* pCrtc;
      pCrtc = GetMachine()->GetCrtc();

      const CCrtc::SGeneratedAddress* pCrtcAddressTable;
      pCrtcAddressTable = pCrtc->GetGeneratedAddressTable();

      unsigned* pDestPixel;
      pDestPixel = (unsigned*) pImageBuffer;

      unsigned nScanLineCount;
      nScanLineCount = pCrtc->GetRegisterValue(CCrtc::VERTICAL_DISPLAYED) * (pCrtc->GetRegisterValue(CCrtc::MAXIMUM_RASTER_ADDRESS) + 1);

      cpcWord nSrcAddress;
      cpcWord nVideoWord;
      CMemoryBlock* pRamBlock;
      unsigned nCurrScanLine;
      for (nCurrScanLine = 0; nCurrScanLine < nScanLineCount; nCurrScanLine++)
      {
        const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress = pCrtcAddressTable[nCurrScanLine];

        unsigned nCurrCharacter;
        for (nCurrCharacter = 0; nCurrCharacter < pCrtc->GetRegisterValue(CCrtc::HORIZONTAL_DISPLAYED); nCurrCharacter++)
        {
          // Two bytes are read per character
          nSrcAddress = CONVERT_ADDRESS_CRTC_TO_MEMORY( scanLineStartCrtcAddress.MA + nCurrCharacter, scanLineStartCrtcAddress.RA );
          pRamBlock   = pMemory->GetRamBlock( (nSrcAddress & 0xC000) >> 14 );    // Bits 15,14 of the address determine which RAM block to read from.

          nVideoWord = (pRamBlock->ReadByte(nSrcAddress) << 8) |                 // Only bits 13-0 of address will be taken into account.
                        pRamBlock->ReadByte(nSrcAddress + 1);                    // Only bits 13-0 of address will be taken into account.

          // Decode pixels and write them to the destination buffer. This depends on the current screen mode.
          switch (pGateArray->GetScreenMode())
          {
            case CGateArray::SCREEN_MODE_0:   // 160x200 resolution, 16 colors (4bpp)
            {
            }
            break;

            case CGateArray::SCREEN_MODE_1:   // 320x200 resolution, 4 colors (2bpp)
            {
              // Each word holds 8 pixels. Additionally, we write each pixel twice in the output image buffer
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0800) >> 10) | ((nVideoWord&0x8000) >> 15) );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0800) >> 10) | ((nVideoWord&0x8000) >> 15) );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).

              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0400) >> 9) | ((nVideoWord&0x4000) >> 14)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0400) >> 9) | ((nVideoWord&0x4000) >> 14)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).

              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0200) >> 8) | ((nVideoWord&0x2000) >> 13)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0200) >> 8) | ((nVideoWord&0x2000) >> 13)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).

              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0100) >> 7) | ((nVideoWord&0x1000) >> 12)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0100) >> 7) | ((nVideoWord&0x1000) >> 12)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).


              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0008) >> 2) | ((nVideoWord&0x0080) >> 7) );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0008) >> 2) | ((nVideoWord&0x0080) >> 7) );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).

              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0004) >> 1) | ((nVideoWord&0x0040) >> 6)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0004) >> 1) | ((nVideoWord&0x0040) >> 6)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).

              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0002)     ) | ((nVideoWord&0x0020) >> 5)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0002)     ) | ((nVideoWord&0x0020) >> 5)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).

              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0001) << 1) | ((nVideoWord&0x0010) >> 4)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
              *pDestPixel = pGateArray->GetPenRgb( ((nVideoWord&0x0001) << 1) | ((nVideoWord&0x0010) >> 4)  );
              pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
            }
            break;

            case CGateArray::SCREEN_MODE_2:   // 640x200 resolution, 2 colors (1bpp)
            {
            }
            break;

            case CGateArray::SCREEN_MODE_3:   // 160x200 resolution, 4 colors (2bpp) (unofficial)
            {
            }
            break;
          }
        }
      }
    }


    //if (pImageBuffer != NULL)
    //{
    //  CMemory* pMemory;
    //  pMemory = GetMachine()->GetMemory();

    //  //CGateArray* pGateArray;
    //  //pGateArray = GetMachine()->GetGateArray();

    //  CCrtc* pCrtc;
    //  pCrtc = GetMachine()->GetCrtc();

    //  const CCrtc::SGeneratedAddress* pCrtcAddressTable;
    //  pCrtcAddressTable = pCrtc->GetGeneratedAddressTable();

    //  unsigned* pDestPixel;
    //  pDestPixel = (unsigned*) pImageBuffer;

    //  unsigned nScanLine = 0;
    //  cpcWord nSrcAddress;
    //  cpcByte nVideoByte;
    //  CMemoryBlock* pRamBlock;
    //  while (nScanLine < IMAGEBUFFER_HEIGHT)
    //  {
    //    unsigned x;

    //    // Decode scanline and draw it twice
    //    x = 0;
    //    while (x < IMAGEBUFFER_WIDTH)
    //    {
    //      // Get final video memory address
    //      nSrcAddress = ComputeVideoMemoryAddress( pCrtcAddressTable[nScanLine] +  );

    //      // Decode pixel RGB
    //      pRamBlock = pMemory->GetRamBlock( (nSrcAddress & 0x4000) >> 14 );    // Bits 15,14 of the address determine which RAM block to read from.
    //      nVideoByte = pRamBlock->ReadByte( nSrcAddress );                     // Only bits 13-0 will be taken into account.

    //      // Write pixel
    //      unsigned uOutPixel;
    //      uOutPixel = (nVideoByte << 16) |        // Red
    //                  (nVideoByte <<  8) |        // Green
    //                  (nVideoByte      );         // Blue

    //      *pDestPixel = uOutPixel;
    //      pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
    //      nSrcAddress++;
    //      x++;
    //    }

    //    x = 0;
    //    while (x < IMAGEBUFFER_WIDTH)
    //    {
    //      // Decode pixel RGB
    //      //...

    //      // Write pixel
    //      unsigned uOutPixel;
    //      uOutPixel = (0 << 16) |        // Red
    //                  (0 <<  8) |        // Green
    //                  (0      );         // Blue

    //      *pDestPixel = uOutPixel;
    //      pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
    //      x++;
    //    }

    //    // Advance a scan line
    //    nScanLine++;
    //  }
    //}
  }

} //namespace CPC
