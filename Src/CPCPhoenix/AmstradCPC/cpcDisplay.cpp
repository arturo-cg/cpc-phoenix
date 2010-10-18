//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcDisplay.h"



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
      unsigned* pDestPixel;
      pDestPixel = (unsigned*) pImageBuffer;

      unsigned y = 0;
      while (y < IMAGEBUFFER_HEIGHT)
      {
        unsigned x;

        // Decode scanline and draw it twice
        x = 0;
        while (x < IMAGEBUFFER_WIDTH)
        {
          // Decode pixel RGB
          //...

          // Write pixel
          unsigned uOutPixel;
          uOutPixel = ((y % 255) << 16) |        // Red
                      (        0 <<  8) |        // Green
                      (        0      );         // Blue

          *pDestPixel = uOutPixel;
          pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
          x++;
        }

        x = 0;
        while (x < IMAGEBUFFER_WIDTH)
        {
          // Decode pixel RGB
          //...

          // Write pixel
          unsigned uOutPixel;
          uOutPixel = (0 << 16) |        // Red
                      (0 <<  8) |        // Green
                      (0      );         // Blue

          *pDestPixel = uOutPixel;
          pDestPixel++;     // Advance to next pixel (note: pDestPixel is unsigned*, so this actually advances 4 bytes).
          x++;
        }

        // Advance 2 scanlines vertically
        y += 2;
      }
    }
  }

} //namespace CPC
