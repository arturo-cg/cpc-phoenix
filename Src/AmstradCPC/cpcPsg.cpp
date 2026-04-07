//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcPsg.h"
#include "cpcMachine.h"
#include "cpcPpi.h"
#include "cpcSoundOutput.h"


namespace CPC {

    /*static*/ const CPsg::Envelope CPsg::Envelopes[16] = {
        /*  0 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::ConstantLow }, false },
        /*  1 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::ConstantLow }, false },
        /*  2 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::ConstantLow }, false },
        /*  3 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::ConstantLow }, false },
        /*  4 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::ConstantLow }, false },
        /*  5 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::ConstantLow }, false },
        /*  6 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::ConstantLow }, false },
        /*  7 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::ConstantLow }, false },
        /*  8 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::HighToLow }, true },
        /*  9 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::ConstantLow }, false },
        /* 10 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::LowToHigh }, true },
        /* 11 */ { { EnvelopeCycle::HighToLow, EnvelopeCycle::ConstantHigh }, false },
        /* 12 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::LowToHigh }, true },
        /* 13 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::ConstantHigh }, false },
        /* 14 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::HighToLow }, true },
        /* 15 */ { { EnvelopeCycle::LowToHigh, EnvelopeCycle::ConstantLow }, false },
    };

    //----------------------------------------------------------------------------
    /**
    **
    */
    CPsg::CPsg(CMachine* pMachine) : inherited(pMachine)
    {
        // Reset members
        ResetVars();

        //...
    }

    //----------------------------------------------------------------------------
    /**
    ** ResetVars
    */
    void CPsg::ResetVars()
    {
        m_eSelectedRegister = REG_A_TONE_PERIOD_LOW;
        memset(m_anRegisters, 0, sizeof(m_anRegisters));

        m_noiseGenerator.Reset();
        m_envelopeGenerator.Reset();
        for (int i = 0; i < 3; i++)
        {
            m_toneGenerator[i].Reset();
            m_channelOutputs[i] = 0.f;
        }
    }

    //----------------------------------------------------------------------------
    /**
    ** FreeVars
    */
    void CPsg::FreeVars()
    {
        //...
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ void CPsg::Reset()
    {
        ResetVars();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPsg::TakeSnapshot(Snapshot* snapshot) const
    {
        snapshot->selectedRegister = m_eSelectedRegister;
        std::copy(std::begin(m_anRegisters), std::end(m_anRegisters), std::begin(snapshot->registers));
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPsg::ApplySnapshot(const Snapshot& snapshot)
    {
        m_eSelectedRegister = snapshot.selectedRegister;
        // Update registers.
        std::copy(std::begin(snapshot.registers), std::end(snapshot.registers), std::begin(m_anRegisters));
        // Update generators.
        WriteRegister(REG_A_TONE_PERIOD_LOW, m_anRegisters[REG_A_TONE_PERIOD_LOW]);
        WriteRegister(REG_A_TONE_PERIOD_HIGH, m_anRegisters[REG_A_TONE_PERIOD_HIGH]);
        WriteRegister(REG_B_TONE_PERIOD_LOW, m_anRegisters[REG_B_TONE_PERIOD_LOW]);
        WriteRegister(REG_B_TONE_PERIOD_HIGH, m_anRegisters[REG_B_TONE_PERIOD_HIGH]);
        WriteRegister(REG_C_TONE_PERIOD_LOW, m_anRegisters[REG_C_TONE_PERIOD_LOW]);
        WriteRegister(REG_C_TONE_PERIOD_HIGH, m_anRegisters[REG_C_TONE_PERIOD_HIGH]);
        WriteRegister(REG_NOISE_PERIOD, m_anRegisters[REG_NOISE_PERIOD]);
        WriteRegister(REG_ENVELOPE_PERIOD_LOW, m_anRegisters[REG_ENVELOPE_PERIOD_LOW]);
        WriteRegister(REG_ENVELOPE_PERIOD_HIGH, m_anRegisters[REG_ENVELOPE_PERIOD_HIGH]);
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPsg::SelectFunction(EFunction eFunction)
    {
        switch (eFunction)
        {
            case FUNCTION_INACTIVE:
                // Inactive - Do nothing.
                break;

            case FUNCTION_READ_REGISTER:
                // We do nothing right now, CPpi will eventually call CPsg::GetSelectedRegisterValue to actually read the selected register.
                break;

            case FUNCTION_WRITE_REGISTER:
                // We take the value from PPI port A and write it to the currently selected PSG register.
                WriteRegister(m_eSelectedRegister, GetMachine()->GetPpi()->GetPortOutputValue(CPpi::PORT_A));
                break;

            case FUNCTION_SELECT_REGISTER:
                // We take the register index from PPI port A and remember it for subsequent register reads/writes.
                m_eSelectedRegister = (ERegister)(GetMachine()->GetPpi()->GetPortOutputValue(CPpi::PORT_A) & 0x0F);   // Bits 3-0 of PPI port A value contain the register index.
                break;
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPC::CPsg::WriteRegister(ERegister reg, cpcByte value)
    {
        // Store the value in the register.
        m_anRegisters[reg] = value;

        // Update the corresponding generator, if needed.
        switch (reg)
        {
            case REG_A_TONE_PERIOD_LOW:
            case REG_A_TONE_PERIOD_HIGH:
            case REG_B_TONE_PERIOD_LOW:
            case REG_B_TONE_PERIOD_HIGH:
            case REG_C_TONE_PERIOD_LOW:
            case REG_C_TONE_PERIOD_HIGH:
            {
                int registerLow = (reg & 0xFE);
                cpcWord period = ((m_anRegisters[registerLow + 1] & 0x0F) << 8) | m_anRegisters[registerLow];
                int channel = (reg >> 1);
                m_toneGenerator[channel].SetPeriod(period);
                break;
            }
            case REG_NOISE_PERIOD:
            {
                m_noiseGenerator.SetPeriod(m_anRegisters[REG_NOISE_PERIOD] & 0x1F);
                break;
            }
            case REG_ENVELOPE_PERIOD_LOW:
            case REG_ENVELOPE_PERIOD_HIGH:
            {
                uint32_t period = (m_anRegisters[REG_ENVELOPE_PERIOD_HIGH] << 8) | m_anRegisters[REG_ENVELOPE_PERIOD_LOW];
                m_envelopeGenerator.SetPeriod(period);
                break;
            }
            case REG_ENVELOPE_SHAPE:
            {
                m_envelopeGenerator.counter = 0;
                m_envelopeGenerator.cycle = 0;
                const Envelope& selectedEnvelope = Envelopes[m_anRegisters[REG_ENVELOPE_SHAPE] & 0x0F];
                m_envelopeGenerator.ComputeAmplitude(selectedEnvelope);
                break;
            }
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPsg::UpdateChannel(int channel)
    {
        bool toneEnabled = (m_anRegisters[REG_MIXER] & (0x01 << channel)) == 0;
        bool noiseEnabled = (m_anRegisters[REG_MIXER] & (0x08 << channel)) == 0;

        // Mix tone and noise.
        // The code below is equivalent to: mixedState = (!toneEnabled || m_toneGenerator[channel].state) && (!noiseEnabled || m_noiseGenerator.state);
        bool mixedState;
        if (toneEnabled && noiseEnabled)    // Tone and noise enabled.
        {
            // Both generator states are ANDed together.
            mixedState = m_toneGenerator[channel].state && m_noiseGenerator.state;
        }
        else if (toneEnabled)               // Only tone enabled.
        {
            mixedState = m_toneGenerator[channel].state;
        }
        else if (noiseEnabled)              // Only noise enabled.
        {
            mixedState = m_noiseGenerator.state;
        }
        else                                // Tone and noise disabled.
        {
            // Output is set to high. Some software uses this in combination with amplitude control to play digitized sound.
            mixedState = true;
        }

        // Determine amplitude.
        int amplitudeRegister = REG_A_AMPLITUDE + channel;
        bool useEnvelope = ((m_anRegisters[amplitudeRegister] & 0x10) != 0);
        uint8_t amplitude = useEnvelope ? m_envelopeGenerator.amplitude : (m_anRegisters[amplitudeRegister] & 0x0F);

        // Generate analog output (D/A converter).
        static const double SQRT_2 = sqrt(2.0);
        m_channelOutputs[channel] = (mixedState && (amplitude > 0) ? 1.f / float(pow(SQRT_2, 15 - amplitude)) : 0.f);
    }

    float CPsg::GetChannelOutput(int channel) const
    {
        if ((channel >= 0) && (channel <= 2))
        {
            return m_channelOutputs[channel];
        }
        else
        {
            return 0.f;
        }
    }

    uint32_t CPC::CPsg::GetChannelTonePeriod(int channel) const
    {
        return ((channel >= 0) && (channel < 3) ? m_toneGenerator[channel].programmedCount : 0);
    }

    uint32_t CPC::CPsg::GetNoisePeriod() const
    {
        return m_noiseGenerator.programmedCount;
    }

    uint32_t CPsg::GetEnvelopePeriod() const
    {
        return m_envelopeGenerator.programmedCount;
    }

    bool CPsg::IsChannelToneEnabled(int channel) const
    {
        if ((channel >= 0) && (channel < 3))
        {
            return (m_anRegisters[REG_MIXER] & (0x01 << channel)) == 0;
        }
        else
        {
            return false;
        }
    }

    bool CPsg::IsChannelNoiseEnabled(int channel) const
    {
        if ((channel >= 0) && (channel < 3))
        {
            return (m_anRegisters[REG_MIXER] & (0x08 << channel)) == 0;
        }
        else
        {
            return false;
        }
    }

    bool CPsg::IsChannelAmplitudeControlledByEnvelope(int channel) const
    {
        if ((channel >= 0) && (channel < 3))
        {
            int amplitudeRegister = REG_A_AMPLITUDE + channel;
            return ((m_anRegisters[amplitudeRegister] & 0x10) != 0);
        }
        else
        {
            return false;
        }
    }

    uint32_t CPsg::GetChannelConstantAmplitude(int channel) const
    {
        // It always returns the value in bits 3-0, even if the envelope is enabled.
        int amplitudeRegister = REG_A_AMPLITUDE + channel;
        return (m_anRegisters[amplitudeRegister] & 0x0F);
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPsg::Run(unsigned nNumCycles)
    {
        for (unsigned cycle = 0; cycle < nNumCycles; cycle++)
        {
            // Update generators (tone x 3, noise and envelope).
            m_noiseGenerator.Tick();
            m_envelopeGenerator.Tick(Envelopes[m_anRegisters[REG_ENVELOPE_SHAPE] & 0x0F]);

            for (int i = 0; i < 3; i++)
            {
                m_toneGenerator[i].Tick();
            }

            // Update channels and generate their analog output.
            for (int i = 0; i < 3; i++)
            {
                // Mix tone and noise, apply amplitude and generate the analog output.
                UpdateChannel(i);
            }
        }
    }

    void CPC::CPsg::ToneGenerator::Reset()
    {
        SetPeriod(0);
        counter = 0;
        state = false;
    }

    void CPsg::ToneGenerator::SetPeriod(uint32_t period)
    {
        if (period > 0)
        {
            programmedCount = period << (4 - 1);
        }
        else
        {
            programmedCount = 1;
        }
    }

    void CPsg::ToneGenerator::Tick()
    {
        counter++;
        while (counter >= programmedCount)
        {
            counter -= programmedCount;
            state = !state;
        }
    }

    void CPC::CPsg::NoiseGenerator::Reset()
    {
        SetPeriod(0);
        counter = 0;
        shiftRegister = 0x1FFFF;
        state = false;
    }

    void CPsg::NoiseGenerator::SetPeriod(uint32_t period)
    {
        if (period > 0)
        {
            programmedCount = period << (4 - 1);
        }
        else
        {
            programmedCount = 1;
        }
    }

    void CPC::CPsg::NoiseGenerator::Tick()
    {
        counter++;
        while (counter >= programmedCount)
        {
            counter -= programmedCount;

            // Update state.
            uint32_t bit0 = shiftRegister & 0x01;
            uint32_t bit3 = (shiftRegister & 0x10) >> 4;
            uint32_t msb = bit0 ^ bit3;
            shiftRegister = (shiftRegister >> 1) | (msb << 16);

            uint32_t stateInt = (state ? 1 : 0);
            stateInt = stateInt ^ bit0;
            state = (stateInt != 0);
        }
    }

    void CPC::CPsg::EnvelopeGenerator::Reset()
    {
        SetPeriod(0);
        counter = 0;
        cycle = 0;
        amplitude = 0;
    }

    void CPsg::EnvelopeGenerator::SetPeriod(uint32_t period)
    {
        // Period 0 is half the duration of period 1: period 1 -> 256 ticks, period 0 -> 128 ticks.
        if (period > 0)
        {
            programmedCount = period << 8;
        }
        else
        {
            programmedCount = 128;
        }
    }

    void CPsg::EnvelopeGenerator::ComputeAmplitude(const Envelope& selectedEnvelope)
    {
        uint32_t stepDuration = (programmedCount >> 4);        // 16 steps per cycle.
        stepDuration = (stepDuration > 0 ? stepDuration : 1);
        uint32_t step = counter / stepDuration;
        switch (selectedEnvelope.cycles[cycle])
        {
            case EnvelopeCycle::ConstantLow: amplitude = 0; break;
            case EnvelopeCycle::ConstantHigh: amplitude = 15; break;
            case EnvelopeCycle::LowToHigh: amplitude = step; break;  // From 0 to 15.
            case EnvelopeCycle::HighToLow: amplitude = 15 - step; break;   // From 15 to 0.
        }
    }

    void CPC::CPsg::EnvelopeGenerator::Tick(const Envelope& selectedEnvelope)
    {
        counter++;
        while (counter >= programmedCount)
        {
            // Cycle completed.
            counter -= programmedCount;
            cycle = (selectedEnvelope.repeatBothCycles ? (cycle + 1) % 2 : 1);
        }

        // Update amplitude.
        ComputeAmplitude(selectedEnvelope);
    }

} //namespace CPC
