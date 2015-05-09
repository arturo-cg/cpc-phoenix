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
      FUNCTION_INACTIVE        = 0x00,
      FUNCTION_READ_REGISTER   = 0x01,    // Read from the selected PSG register. The register content is then readable from PPI port A.
      FUNCTION_WRITE_REGISTER  = 0x02,    // Write to the selected PSG register. The register content is taken from PPI port A.
      FUNCTION_SELECT_REGISTER = 0x03,    // Selects the specified PSG register. The register to select is taken from PPI port A.

      FUNCTION_LAST,
      FUNCTION_INVALID = 0x7FFFFFFF
    };

    enum ERegister
    {
      REG_A_TONE_PERIOD_LOW    = 0x00,         // [Channel A] Lowest 8 bits of the period of the generated square wave.
      REG_A_TONE_PERIOD_HIGH   = 0x01,         // [Channel A] Highest 4 bits of the period of the generated square wave.
      REG_B_TONE_PERIOD_LOW    = 0x02,         // [Channel B] Lowest 8 bits of the period of the generated square wave.
      REG_B_TONE_PERIOD_HIGH   = 0x03,         // [Channel B] Highest 4 bits of the period of the generated square wave.
      REG_C_TONE_PERIOD_LOW    = 0x04,         // [Channel C] Lowest 8 bits of the period of the generated square wave.
      REG_C_TONE_PERIOD_HIGH   = 0x05,         // [Channel C] Highest 4 bits of the period of the generated square wave.
      REG_NOISE_PERIOD         = 0x06,         // Period of the noise (lower 5 bits). The generated noise can be mixed into any of the three channels.
      REG_MIXER                = 0x07,         // Enables or disables tone and noise generation on each channel.
      REG_A_AMPLITUDE          = 0x08,         // [Channel A] If bit 4 is set, amplitude is controlled by the envelope. If bit 4 is clear, bits 3-0 determine a constant amplitude.
      REG_B_AMPLITUDE          = 0x09,         // [Channel B] If bit 4 is set, amplitude is controlled by the envelope. If bit 4 is clear, bits 3-0 determine a constant amplitude.
      REG_C_AMPLITUDE          = 0x0A,         // [Channel C] If bit 4 is set, amplitude is controlled by the envelope. If bit 4 is clear, bits 3-0 determine a constant amplitude.
      REG_ENVELOPE_PERIOD_LOW  = 0x0B,         // Lowest 8 bits of the period of the envelope.
      REG_ENVELOPE_PERIOD_HIGH = 0x0C,         // Highest 8 bits of the period of the envelope.
      REG_ENVELOPE_SHAPE       = 0x0D,         // Envelope shape (see data sheet).
      REG_IO_PORT              = 0x0E,         // I/O Port A (the only one in the AY-3-8912). In the CPC, it is connected to the selected keyboard matrix.

      REG_COUNT
    };


                            CPsg                      (CMachine *pMachine);
    virtual                ~CPsg                      ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Selects the PSG function to perform. */
    void                    SelectFunction            (EFunction eFunction);

    /** Writes a new value into the specified register.
    *** This is used by CKeyboard each time a new keyboard matrix line is selected or a key is pressed or released. */
    void                    SetRegisterValue          (ERegister eRegister, cpcByte nValue)  { m_anRegisters[eRegister] = nValue; }
    /** Writes a new value into the currently selected register. */
    void                    SetSelectedRegisterValue  (cpcByte nValue)                 { m_anRegisters[m_eSelectedRegister] = nValue; }
    /** Returns the value of the currently selected value. */
    cpcByte                 GetSelectedRegisterValue  () const                         { return m_anRegisters[m_eSelectedRegister]; }

    /** Runs the PSG for the given number of cycles. */
    void                    Run                       (unsigned nNumCycles);


  private:

    typedef                 CSubSystem                inherited;

    static const float      CYCLES_PER_SAMPLE;        // Every how many cycles we need to generate a sample (chip_clock/sample_rate = 1Mhz/44.1kHz).
    static const float      ANGLE_INC_PER_SAMPLE;     // Angle increment at every sample.

    enum EGenerateSampleFlags
    {
      GENSAMPLE_TONE_ENABLED  = 0x01,      // Mix tone wave in.
      GENSAMPLE_NOISE_ENABLED = 0x02,      // Min noise wave in.
      GENSAMPLE_USE_ENVELOPE  = 0x04,      // Amplitude is controlled by the envelope.
    };


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    float                   GenerateNoiseSample       ();
    float                   GenerateChannelSample     (unsigned nRegToneLow, unsigned nRegToneHigh, unsigned nRegAmplitude, unsigned nMixerOffset, float fNoiseSample);
    float                   GenerateSample            (unsigned nTonePeriod, unsigned nFixedAmplitude, float fNoiseSample, int/*EGenerateSampleFlags*/ nFlags);


    ERegister               m_eSelectedRegister;
    cpcByte                 m_anRegisters[REG_COUNT];

    float                   m_fAccumCycles;           // Used to determine when to compute a new sound sample.
    float                   m_fAngle;

  };


} //namespace CPC

#endif // _CPCPSG_H_
