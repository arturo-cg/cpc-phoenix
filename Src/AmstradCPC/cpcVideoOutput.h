//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCVIDEOOUTPUT_H_
#define _CPCVIDEOOUTPUT_H_


#include "cpcSubSystem.h"
#include "cpcCrtc.h"


namespace CPC {

    /**
    ** This class represents the emulated monitor and provides video output functionality to the emulator.
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

        // Minimum buffer dimensions. The actual buffer can be any size equal to or larger than this.
        // The CPC video hardware emulation writes pixels into this buffer.
        static const unsigned BUFFER_WIDTH = 1024;//912;   // 57 CRTC characters * 16 mode 2 pixels. When this config is used, HSYNC width must be 7 CRTC characters to have 64 characters per scan line.
        static const unsigned BUFFER_HEIGHT = 312;  // 64 CRTC characters per scan line * 312 scan lines = 19968 microsecs per frame = 50.08 Hz
        // The CTM monitor is calibrated to display only a subset of the video information that the CPC generates.
        // This determines what is actually shown to the user.
        // TODO: Check these numbers.
        static const unsigned VIEWPORT_LEFT = 128;
        static const unsigned VIEWPORT_TOP = 23;
        static const unsigned VIEWPORT_WIDTH = 768;
        static const unsigned VIEWPORT_HEIGHT = 270;

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


        CVideoOutput(CMachine *pMachine);
        virtual                ~CVideoOutput() { FreeVars(); }

        /** Resets the subsystem. */
        virtual void            Reset();

        /** */
        void                    Run();

        /** Returns the current frame count. */
        unsigned                GetFrameCount() const { return m_frameCount; }

        /** HSYNC signal's rising edge notification. */
        void                    OnHSyncBegin();
        /** HSYNC signal's falling edge notification. */
        void                    OnHSyncEnd();
        /** VSYNC signal's rising edge notification. */
        void                    OnVSyncBegin();
        /** VSYNC signal's falling edge notification. */
        void                    OnVSyncEnd();


    protected:

        /** Called by the emulator to get the properties of the buffer(s) provided by the derived class. These properties should never change. */
        virtual const SBufferProperties& GetBufferProperties() const = 0;
        /** Called by the emulator to get a pointer to the buffer that it should write to. */
        virtual unsigned char*  GetBuffer() = 0;
        /** Called by the emulator to indicate the current image buffer is complete and should be shown to the user, and a new one is going to be written.
        *** Notes:
        ***  - If the derived class uses a double buffer technique, this is when front and back buffers should be swapped.
        ***  - The new buffer must be prepared for writing, i.e. if the buffer is a OpenGL or DirectX texture, it must be locked.
        */
        virtual void            OnBufferComplete() = 0;


    private:

        typedef                 CSubSystem                inherited;


        void                    ResetVars();
        void                    FreeVars();


        bool                    m_HSyncActive;
        bool                    m_VSyncActive;
        unsigned                m_beamX;
        unsigned                m_beamY;
        unsigned                m_frameCount;

    };


} //namespace CPC

#endif // _CPCVIDEOOUTPUT_H_
