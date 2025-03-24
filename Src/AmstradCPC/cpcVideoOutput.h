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
    **
    ** PC1031 and PC1378 were the horizontal and vertical deflector circuits used by the CPC monitor (TODO: which monitor exactly?).
    */
    class CVideoOutput
    {
    public:

        // Minimum buffer dimensions. The actual buffer can be any size equal to or larger than this.
        // The CPC video hardware emulation writes pixels into this buffer.
        // It includes the entire area where the beam could potentially draw pixels, even though pixels will always be drawn in a subset
        // of it. Where pixels are drawn is determined by the CRTC's configuration. The rest of the area is where HSYNC and VSYNC are active.
        static const unsigned BUFFER_WIDTH = 1024;  // 64 CRTC characters * 16 mode 2 pixels.
        static const unsigned BUFFER_HEIGHT = 312;  // 64 CRTC characters per scan line * 312 scan lines = 19968 microsecs per frame = 50.08 Hz. It includes top & bottom borders and VSYNC.
        // The CTM monitor is calibrated to display only a subset of the video information that the CPC generates.
        // This determines what is actually shown to the user.
        // TODO: Check these numbers.
        static const unsigned VIEWPORT_LEFT = 128;
        static const unsigned VIEWPORT_TOP = 27;
        static const unsigned VIEWPORT_WIDTH = 768;
        static const unsigned VIEWPORT_HEIGHT = 270;

        enum EPixelFormat
        {
            PIXELFORMAT_B8G8R8A8 = 0,      // 32-bit pixel with four 8-bit channels: Blue, green, red and alpha.

            PIXELFORMAT_LAST,
            PIXELFORMAT_INVALID = 0x7FFFFFFF
        };

        struct SBufferProperties
        {
            unsigned     nWidth;            // Width of the buffer in pixels. Must be >= BUFFER_WIDTH.
            //unsigned     nHeight;           // Height of the buffer in pixels.
            unsigned     nStride;           // Number of bytes between the start of a scan lines and the start of the next one.
            EPixelFormat eFormat;           // Pixel format.
            unsigned char* data;            // Pointer to the buffer data.
        };


        CVideoOutput();
        virtual                ~CVideoOutput() { FreeVars(); }

        /** Resets the subsystem. */
        virtual void            Reset();

        /** Sets the CMachine that this CVideoOutput is going to be used on. */
        void                    SetMachine(CMachine* machine)  { m_machine = machine; }
        /** Gets the CMachine that this CVideoOutput is being used on. */
        CMachine*               GetMachine()  { return m_machine; }
        const CMachine*         GetMachine() const  { return m_machine; }

        /** */
        void                    Run();

        /** Returns the current X coordinate of the beam position. */
        unsigned                GetBeamX() const;
        /** Returns the current Y coordinate of the beam position. */
        unsigned                GetBeamY() const;

        /** HSYNC signal's rising edge notification. */
        void                    OnGateArrayHSyncBegin();
        /** HSYNC signal's falling edge notification. */
        void                    OnGateArrayHSyncEnd();
        /** VSYNC signal's rising edge notification. */
        void                    OnGateArrayVSyncBegin();
        /** VSYNC signal's falling edge notification. */
        void                    OnGateArrayVSyncEnd();


    protected:

        /** Called by the emulator to get the properties of the buffer(s) provided by the derived class. */
        virtual const SBufferProperties& GetBufferProperties() const = 0;
        /** Called by the emulator to indicate the current image buffer is complete and should be shown to the user, and a new one is going to be written.
        *** Notes:
        ***  - If the derived class uses a double buffer technique, this is when front and back buffers should be swapped.
        ***  - The new buffer must be prepared for writing, i.e. if the buffer is a OpenGL or DirectX texture, it must be locked.
        */
        virtual void            OnBufferComplete() = 0;


        CMachine*               m_machine;

    private:

        struct Oscillator
        {
            int period;        // Total clock cycles.
            int counter;       // Clock cycle counter.
            int gateArrayCounter;
            int offset;        // Difference between the Gate Array's and the monitor's SYNC signals.

            void Initialize(int idealPeriod);
            bool Tick();
            void OnGateArraySync();
            int OffsetCounter() const;
        };

        static const unsigned IdealScanlinePeriod = 64;     // In 1 Mhz clock cycles.
        static const unsigned IdealFramePeriod = 19968;     // In 1 Mhz clock cycles. Approx. 50.080128hz

        void ResetVars();
        void FreeVars();

        Oscillator m_horizontalOscillator;
        Oscillator m_verticalOscillator;
    };


} //namespace CPC

#endif // _CPCVIDEOOUTPUT_H_
