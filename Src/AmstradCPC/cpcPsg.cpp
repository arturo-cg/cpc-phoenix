//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcPsg.h"
#include "cpcMachine.h"
#include "cpcPpi.h"
#include "cpcSoundOutput.h"



static const float PI = 3.1415926535897932384626433832795f;

/*static*/ const float CPC::CPsg::CYCLES_PER_SAMPLE = 1000000.f / 44100.f;  // chip_clock/sample_rate = 1Mhz/44.1kHz = 22.675737
/*static*/ const float CPC::CPsg::ANGLE_INC_PER_SAMPLE = (2.f * PI) / 44100.f;



namespace CPC {


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
        m_fAccumCycles = 0.f;
        m_fAngle = 0.f;
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
                m_anRegisters[m_eSelectedRegister] = GetMachine()->GetPpi()->GetPortOutputValue(CPpi::PORT_A);
                break;

            case FUNCTION_SELECT_REGISTER:
                // We take the register index from PPI port A and remember it for subsequent register reads/writes.
                m_eSelectedRegister = (ERegister)(GetMachine()->GetPpi()->GetPortOutputValue(CPpi::PORT_A) & 0x0F);   // Bits 3-0 of PPI port A value contain the register index.
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    float CPsg::GenerateNoiseSample()
    {
        // 1. Generate white noise (i.e. random number that is either -1 or 1, both with same probability to be picked).
        // 2. Modulate it with the programmed frequency.
        float fNoise = (rand() < (RAND_MAX / 2) ? -1.f : 1.f) * 0.2f/*noise amplitude*/;

        //////unsigned nNoisePeriod = m_anRegisters[REG_NOISE_PERIOD] & 0x1F;
        //////float fNoisePeriod = float( nNoisePeriod!=0 ? nNoisePeriod : 1 );
        //////float fFrequency = 1000000.f / (16.f * fNoisePeriod);                          // Formula from manufacturer's chip datasheet.
        //////float fRet = ::sinf( (fFrequency + fNoise) * m_fAngle );
        //////return fRet;
        return fNoise;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    float CPsg::GenerateSample(unsigned nTonePeriod, unsigned nFixedAmplitude, float fNoiseSample, int/*EGenerateSampleFlags*/ nFlags)
    {
        float fRet = 0.f;

        // Tone
        if (nFlags & GENSAMPLE_TONE_ENABLED)
        {
            float fTonePeriod = float(nTonePeriod != 0 ? nTonePeriod : 1);
            float fFrequency = 1000000.f / (16.f * fTonePeriod);                          // Formula from manufacturer's chip datasheet.

            fRet = ::sinf(fFrequency * m_fAngle);
            fRet = (fRet < 0.f ? -1.f : 1.f);     // Convert to square wave
        }

        // Mix tone and noise
        if ((nFlags & GENSAMPLE_TONE_ENABLED) &&
            (nFlags & GENSAMPLE_NOISE_ENABLED))
        {
            // Tone + noise
            fRet = (fRet + fNoiseSample) * 0.5f;
        }
        else if (nFlags & GENSAMPLE_TONE_ENABLED)
        {
            // Tone only
            // Nothing to do, fRet already contains the tone sample
        }
        else if (nFlags & GENSAMPLE_NOISE_ENABLED)
        {
            // Noise only
            fRet = fNoiseSample;
        }
        else
        {
            // None of them enabled. The PSG outputs 1.f in this case.
            // This, in combination with amplitude control, is used by some software to play raw PCM data (e.g. digitized voice, sampled music, etc.).
            fRet = 1.f;
        }

        // Amplitude
        // TODO: Envelopes
        float fAmplitude = ((nFlags & GENSAMPLE_USE_ENVELOPE) ? 1.f :                               //***** TODO - TODO - TODO *****
                                                                float(nFixedAmplitude) / 15.f);     // nFixedAmplitude is in the range [0,15].
        fRet *= fAmplitude;

        return fRet;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    float CPsg::GenerateChannelSample(unsigned nRegToneLow, unsigned nRegToneHigh, unsigned nRegAmplitude, unsigned nMixerOffset, float fNoiseSample)
    {
        float fRet = 0.f;

        int nFlags = 0;
        if ((m_anRegisters[REG_MIXER] & (0x01 << nMixerOffset)) == 0)
        {
            nFlags |= GENSAMPLE_TONE_ENABLED;
        }
        if ((m_anRegisters[REG_MIXER] & (0x08 << nMixerOffset)) == 0)
        {
            nFlags |= GENSAMPLE_NOISE_ENABLED;
        }
        if ((m_anRegisters[nRegAmplitude] & 0x10) != 0)
        {
            nFlags |= GENSAMPLE_USE_ENVELOPE;
        }

        if ((nFlags & (GENSAMPLE_TONE_ENABLED | GENSAMPLE_NOISE_ENABLED)) != 0)   // If the channel is active...
        {
            fRet = GenerateSample(((m_anRegisters[nRegToneHigh] & 0x0F) << 8) | m_anRegisters[nRegToneLow],
                                  m_anRegisters[nRegAmplitude] & 0x0F,
                                  fNoiseSample,
                                  nFlags);
        }

        return fRet;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPsg::Run(unsigned nNumCycles)
    {
        if (GetMachine()->GetSoundOutput() != NULL)
        {
            m_fAccumCycles += (float)nNumCycles;

            // Generate sound samples while there are enough accumulated cycles
            while (m_fAccumCycles >= CYCLES_PER_SAMPLE)
            {
                // Noise generation
                float fNoiseSample = GenerateNoiseSample();

                // Generate a sample for each channel
                float fSampleA;
                float fSampleB;
                float fSampleC;
                fSampleA = GenerateChannelSample(REG_A_TONE_PERIOD_LOW, REG_A_TONE_PERIOD_HIGH, REG_A_AMPLITUDE, 0, fNoiseSample);
                fSampleB = GenerateChannelSample(REG_B_TONE_PERIOD_LOW, REG_B_TONE_PERIOD_HIGH, REG_B_AMPLITUDE, 1, fNoiseSample);
                fSampleC = GenerateChannelSample(REG_C_TONE_PERIOD_LOW, REG_C_TONE_PERIOD_HIGH, REG_C_AMPLITUDE, 2, fNoiseSample);

                // Mix samples from each channel and write the resulting sample to the sound output
                float fSample;
                fSample = (fSampleA + fSampleB + fSampleC) / 3.f/*num channels*/;

                //////float fNoiseOffset = float(rand()) / float(RAND_MAX) * 10.f/*noise factor*/;

                //////float fSinParam;
                //////fSinParam = fmod( m_fAngle * (500.f + fNoiseOffset), 2.f * PI );

                //////float fSample = ::sinf( fSinParam );

                GetMachine()->GetSoundOutput()->WriteSample(fSample);

                // Update cycle accumulator, angle, etc.
                m_fAccumCycles -= CYCLES_PER_SAMPLE;
                m_fAngle += ANGLE_INC_PER_SAMPLE;
                while (m_fAngle >= 2.f * PI)
                {
                    m_fAngle -= 2.f * PI;
                }
            }
        }
    }

} //namespace CPC
