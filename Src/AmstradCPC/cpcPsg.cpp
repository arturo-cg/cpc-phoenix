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
  CPsg::CPsg(CMachine *pMachine) : inherited( pMachine )
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
    m_fAccumCycles      = 0.f;
    m_fAngle            = 0.f;
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
        m_anRegisters[m_eSelectedRegister] = GetMachine()->GetPpi()->GetPortOutputValue( CPpi::PORT_A );
        break;

      case FUNCTION_SELECT_REGISTER:
        // We take the register index from PPI port A and remember it for subsequent register reads/writes.
        m_eSelectedRegister = (ERegister) (GetMachine()->GetPpi()->GetPortOutputValue( CPpi::PORT_A ) & 0x0F);   // Bits 3-0 of PPI port A value contain the register index.
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  float CPsg::GenerateSample(unsigned nTonePeriod, unsigned nFixedAmplitude, int/*EGenerateSampleFlags*/ nFlags)
  {
    float fRet = 0.f;

    // Tone
    if (nFlags & GENSAMPLE_TONE_ENABLED)
    {
      float fFrequency;
      fFrequency = 1000000.f / ( (float(nTonePeriod<<1) * 8.f) + 1.f );     // nTonePeriod is half the period of the complete wave, that's why it is multiplied by 2.

      float fSinParam;
      fSinParam = fmod( m_fAngle * fFrequency, 2.f * PI );

      fRet = ::sinf( fSinParam );
      fRet = ( fRet<0.f ? -1.f : 1.f );     // Convert to square wave

      // Apply amplitude
      float fAmplitude;
      fAmplitude = ( (nFlags&GENSAMPLE_USE_ENVELOPE) ? 1.f :                                //***** TODO - TODO - TODO *****
                                                       float(nFixedAmplitude) / 15.f );     // nFixedAmplitude is in the range [0,15].
      fRet *= fAmplitude;
    }

    return fRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  float CPsg::GenerateChannelSample(unsigned nRegToneLow, unsigned nRegToneHigh, unsigned nRegAmplitude, unsigned nMixerOffset)
  {
    float fRet = 0.f;

    int nFlags = 0;
    if ((m_anRegisters[REG_MIXER] & (0x01<<nMixerOffset)) == 0)
    {
      nFlags |= GENSAMPLE_TONE_ENABLED;
    }
    if ((m_anRegisters[REG_MIXER] & (0x08<<nMixerOffset)) == 0)
    {
      nFlags |= GENSAMPLE_NOISE_ENABLED;
    }
    if ((m_anRegisters[nRegAmplitude]&0x10) != 0)
    {
      nFlags |= GENSAMPLE_USE_ENVELOPE;
    }

    if ((nFlags & (GENSAMPLE_TONE_ENABLED | GENSAMPLE_NOISE_ENABLED)) != 0)   // If the channel is active...
    {
      fRet = GenerateSample( ((m_anRegisters[nRegToneHigh]&0x0F) << 8) | m_anRegisters[nRegToneLow],
                             m_anRegisters[nRegAmplitude] & 0x0F,
                             nFlags );
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
      m_fAccumCycles += (float) nNumCycles;

      // Generate sound samples while there are enough accumulated cycles
      while (m_fAccumCycles >= CYCLES_PER_SAMPLE)
      {
        // Generate a sample for each channel
        float fSampleA;
        float fSampleB;
        float fSampleC;
        fSampleA = GenerateChannelSample( REG_A_TONE_PERIOD_LOW, REG_A_TONE_PERIOD_HIGH, REG_A_AMPLITUDE, 0 );
        fSampleB = GenerateChannelSample( REG_B_TONE_PERIOD_LOW, REG_B_TONE_PERIOD_HIGH, REG_B_AMPLITUDE, 1 );
        fSampleC = GenerateChannelSample( REG_C_TONE_PERIOD_LOW, REG_C_TONE_PERIOD_HIGH, REG_C_AMPLITUDE, 2 );

        // Mix samples from each channel and write the resulting sample to the sound output
        float fSample;
        fSample = (fSampleA + fSampleB + fSampleC) / 3.f/*num channels*/;

        GetMachine()->GetSoundOutput()->WriteSample( fSample );

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
