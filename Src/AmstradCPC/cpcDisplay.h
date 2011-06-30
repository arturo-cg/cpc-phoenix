//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCDISPLAY_H_
#define _CPCDISPLAY_H_


#include "cpcSubSystem.h"
#include "cpcCrtc.h"


namespace CPC {


  /**
  ** 
  */
  class CDisplay : public CSubSystem
  {
  public:

    static const unsigned   IMAGEBUFFER_WIDTH  = 640;
    static const unsigned   IMAGEBUFFER_HEIGHT = 400;


                            CDisplay                  (CMachine *pMachine);
    virtual                ~CDisplay                  ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Decodes the display image for the current machine state.
    *** Pixels are written in RGBA format, with 8 bits per channel (giving 32 bits per pixel). pImageBuffer must point to a buffer large enough (>1000Kb). */
    void                    DecodeImage_B8G8R8X8      (unsigned char* pImageBuffer);

    /** Activates or deactivates the scan line effect. */
    void                    SetScanLineEffectActivated (bool bActivated)  { m_bScanLineEffectActivated = bActivated; }
    /** Returns true if the scan line effect is activated, or false otherwise. */
    bool                    IsScanLineEffectActivated  () const           { return m_bScanLineEffectActivated; }


  private:

    typedef                 CSubSystem                inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    unsigned*               DecodeScanLine_B8G8R8X8_Mode0 (unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress);
    unsigned*               DecodeScanLine_B8G8R8X8_Mode1 (unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress);
    unsigned*               DecodeScanLine_B8G8R8X8_Mode2 (unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress);


    bool                    m_bScanLineEffectActivated;

  };


} //namespace CPC

#endif // _CPCDISPLAY_H_
