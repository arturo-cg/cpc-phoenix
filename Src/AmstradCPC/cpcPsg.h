//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCPSG_H_
#define _CPCPSG_H_


#include "cpcSubSystem.h"


namespace CPC {


    /**
    ** The AY-3-8912 Programmable Sound Generator (PSG), made by General Instruments, is the responsible for generating sound in the Amstrad CPC.
    ** In addition, the CPC uses the PSG to scan the keyboard matrix to read the state of the keyboard and joysticks.
    **
    ** In the Amstrad CPC, The AY-3-8912 PSG is accessed through the 8255 PPI.
    **
    ** Datasheet: http://www.cpcwiki.eu/imgs/d/dc/Ay3-891x.pdf
    */
    class CPsg : public CSubSystem
    {
    public:

        enum EFunction
        {
            FUNCTION_INACTIVE = 0x00,
            FUNCTION_READ_REGISTER = 0x01,      // Read from the selected PSG register. The register content is then readable from PPI port A.
            FUNCTION_WRITE_REGISTER = 0x02,     // Write to the selected PSG register. The register content is taken from PPI port A.
            FUNCTION_SELECT_REGISTER = 0x03,    // Selects the specified PSG register. The register to select is taken from PPI port A.

            FUNCTION_LAST,
            FUNCTION_INVALID = 0x7FFFFFFF
        };

        enum ERegister
        {
            REG_A_TONE_PERIOD_LOW = 0x00,       // [Channel A] Lowest 8 bits of the period of the generated square wave.
            REG_A_TONE_PERIOD_HIGH = 0x01,      // [Channel A] Highest 4 bits of the period of the generated square wave.
            REG_B_TONE_PERIOD_LOW = 0x02,       // [Channel B] Lowest 8 bits of the period of the generated square wave.
            REG_B_TONE_PERIOD_HIGH = 0x03,      // [Channel B] Highest 4 bits of the period of the generated square wave.
            REG_C_TONE_PERIOD_LOW = 0x04,       // [Channel C] Lowest 8 bits of the period of the generated square wave.
            REG_C_TONE_PERIOD_HIGH = 0x05,      // [Channel C] Highest 4 bits of the period of the generated square wave.
            REG_NOISE_PERIOD = 0x06,            // Period of the noise (lower 5 bits). The generated noise can be mixed into any of the three channels.
            REG_MIXER = 0x07,                   // Enables or disables tone and noise generation on each channel.
            REG_A_AMPLITUDE = 0x08,             // [Channel A] If bit 4 is set, amplitude is controlled by the envelope. If bit 4 is clear, bits 3-0 determine a constant amplitude.
            REG_B_AMPLITUDE = 0x09,             // [Channel B] If bit 4 is set, amplitude is controlled by the envelope. If bit 4 is clear, bits 3-0 determine a constant amplitude.
            REG_C_AMPLITUDE = 0x0A,             // [Channel C] If bit 4 is set, amplitude is controlled by the envelope. If bit 4 is clear, bits 3-0 determine a constant amplitude.
            REG_ENVELOPE_PERIOD_LOW = 0x0B,     // Lowest 8 bits of the period of the envelope.
            REG_ENVELOPE_PERIOD_HIGH = 0x0C,    // Highest 8 bits of the period of the envelope.
            REG_ENVELOPE_SHAPE = 0x0D,          // Envelope shape (see data sheet).
            REG_IO_PORT = 0x0E,                 // I/O Port A (the only one in the AY-3-8912). In the CPC, it is connected to the selected keyboard matrix.

            REG_COUNT
        };

        struct Snapshot
        {
            ERegister selectedRegister;         // Currently selected register.
            cpcByte registers[REG_COUNT];       // Register values.
        };


        CPsg(CMachine* pMachine);
        virtual ~CPsg() { FreeVars(); }

        /** Resets the subsystem. */
        virtual void Reset();

        /** Takes a snapshot of the current state of the device. */
        void TakeSnapshot(Snapshot* snapshot) const;
        /** Applies the values from the specified snaphot. */
        void ApplySnapshot(const Snapshot& snapshot);

        /** Selects the PSG function to perform. */
        void SelectFunction(EFunction eFunction);

        /** Writes a new value into the IO port register.
        *** This is used by CKeyboard each time a new keyboard matrix line is selected or a key is pressed or released. */
        void SetIOPortRegisterValue(cpcByte nValue) { m_anRegisters[REG_IO_PORT] = nValue; }
        /** Returns the value of the currently selected value. */
        cpcByte GetRegisterValue(ERegister reg) const { return m_anRegisters[reg]; }
        /** Returns the value of the currently selected value. */
        cpcByte GetSelectedRegisterValue() const { return m_anRegisters[m_eSelectedRegister]; }

        /** Returns the current analog output for the specfied channel in the range [0, 1]. */
        float GetChannelOutput(int channel) const;

        /** Utility functions. */
        uint32_t GetChannelTonePeriod(int channel) const;
        uint32_t GetNoisePeriod() const;
        uint32_t GetEnvelopePeriod() const;
        bool IsChannelToneEnabled(int channel) const;
        bool IsChannelNoiseEnabled(int channel) const;
        bool IsChannelAmplitudeControlledByEnvelope(int channel) const;
        uint32_t GetChannelConstantAmplitude(int channel) const;

        /** Runs the PSG for the given number of cycles. */
        void Run(unsigned nNumCycles);


    private:

        using inherited = CSubSystem;

        struct ToneGenerator
        {
            uint32_t programmedCount;
            uint32_t counter;
            bool state;

            void Reset();
            void SetPeriod(uint32_t period);
            void Tick();
        };

        struct NoiseGenerator
        {
            uint32_t programmedCount;
            uint32_t counter;
            uint32_t shiftRegister;
            bool state;

            void Reset();
            void SetPeriod(uint32_t period);
            void Tick();
        };

        enum class EnvelopeCycle
        {
            ConstantLow,
            ConstantHigh,
            LowToHigh,
            HighToLow
        };

        struct Envelope
        {
            EnvelopeCycle cycles[2];        // First two cycles.
            bool repeatBothCycles;          // If true, both cycles are repeated indefinitely (0-1-0-1-0-1-0-1); if false, only the second cycle is repeated (0-1-1-1-1-1-1-1).
        };

        struct EnvelopeGenerator
        {
            uint32_t programmedCount;
            uint32_t counter;
            uint8_t cycle;                  // Always 0 or 1.
            uint8_t amplitude;              // Current amplitude [0-15].

            void Reset();
            void SetPeriod(uint32_t period);
            void Tick(const Envelope& selectedEnvelope);
        };

        void ResetVars();
        void FreeVars();

        void WriteRegister(ERegister reg, cpcByte value);

        // Ticks the channel's tone generator, mixes tone and noise, applies amplitude and generates the analog output for the channel.
        void UpdateChannel(int channel);

        ERegister m_eSelectedRegister;
        cpcByte m_anRegisters[REG_COUNT];

        ToneGenerator m_toneGenerator[3];
        NoiseGenerator m_noiseGenerator;
        static const Envelope Envelopes[16];
        EnvelopeGenerator m_envelopeGenerator;
        float m_channelOutputs[3];      // Analog outputs.
    };


} //namespace CPC

#endif // _CPCPSG_H_
