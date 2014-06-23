//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCVIDEOOUTPUT_H_
#define _CPCVIDEOOUTPUT_H_


#include "cpcSubSystem.h"
#include "cpcCrtc.h"


namespace CPC {

  /**
  ** This class provides video output functionality to the emulator. It is a mix of monitor emulation and Gate Array's video signal generation.
  ** It is a pure virtual class. The front-end must implement a class derived from this one and register it via the method CMachine::SetVideoOutput.
  **
  ** As the emulator runs, it calls methods in this class periodically. When a new frame has been drawn, the method CVideoOutput::OnBufferComplete
  ** is called to indicate the image should be made visible to the user and a new frame is about to be drawn. If the derived class uses
  ** a double buffer technique, this is when the front and back buffers should be swapped.
  **
  ** The emulator writes pixels to an image buffer provided by this class. The emulator assumes the buffer is always prepared to be written into.
  ** For example, if the derived class uses OpenGL or DirectX textures, it must make sure the texture has been locked before passing
  ** it to the emulator, and unlock it once CVideoOutput::OnBufferComplete is called and the texture is going to be used for rendering.
  */
  class CVideoOutput : public CSubSystem
  {
  public:

    static const unsigned   BUFFER_WIDTH  = 640;
    static const unsigned   BUFFER_HEIGHT = 400;

    enum EPixelFormat
    {
      PIXELFORMAT_B8G8R8X8 = 0,      // 32-bit pixel with four 8-bit channels: Blue, green, red and another unused one.

      PIXELFORMAT_LAST,
      PIXELFORMAT_INVALID = 0x7FFFFFFF
    };

    struct SBufferProperties
    {
      unsigned     nWidth;            // Width of the buffer in pixels. Must be >= BUFFER_WIDTH.
      //unsigned     nHeight;           // Height of the buffer in pixels.
      unsigned     nStride;           // Length of the gap in bytes between two consecutive scan lines.
      EPixelFormat eFormat;           // Pixel format.
    };


                            CVideoOutput              (CMachine *pMachine);
    virtual                ~CVideoOutput              ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Returns the current frame count. */
    unsigned                GetFrameCount             () const  { return m_nFrameCount; }

    /** Activates or deactivates the scan line effect. */
    void                    SetScanLineEffectActivated(bool bActivated)  { m_bScanLineEffectActivated = bActivated; }
    /** Returns true if the scan line effect is activated, or false otherwise. */
    bool                    IsScanLineEffectActivated () const           { return m_bScanLineEffectActivated; }

    /** Notification from the Gate Array that the HSYNC signal just started. */
    void                    OnHSyncBegin              ();
    /** Notification from the Gate Array that the VSYNC signal just started. */
    void                    OnVSyncBegin              ();


  protected:

    /** Called by the emulator to get the properties of the buffer(s) provided by the derived class. These properties should never change. */
    virtual const SBufferProperties& GetBufferProperties () const = 0;
    /** Called by the emulator to get a pointer to the buffer that it should write to. */
    virtual unsigned char*  GetBuffer                 () = 0;
    /** Called by the emulator to indicate the current image buffer is complete and should be shown to the user, and a new one is going to be written.
    *** Notes:
    ***  - If the derived class uses a double buffer technique, this is when front and back buffers should be swapped.
    ***  - The new buffer must be prepared for writing, i.e. if the buffer is a OpenGL or DirectX texture, it must be locked.
    */
    virtual void            OnBufferComplete          () = 0;


  private:

    typedef                 CSubSystem                inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    void                    DecodeVisibleScanLine_B8G8R8X8();
    unsigned*               DecodeScanLine_B8G8R8X8_Mode0 (unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress);
    unsigned*               DecodeScanLine_B8G8R8X8_Mode1 (unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress);
    unsigned*               DecodeScanLine_B8G8R8X8_Mode2 (unsigned* pDestPixel, const CCrtc::SGeneratedAddress& scanLineStartCrtcAddress);


    bool                    m_bScanLineEffectActivated;
    unsigned                m_nScanLineCount;
    unsigned                m_nFrameCount;

  };


} //namespace CPC

#endif // _CPCVIDEOOUTPUT_H_
