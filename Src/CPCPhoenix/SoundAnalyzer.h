//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

#include "WinSoundOutput.h"

/**
**
*/
class SoundAnalyzer : IWinSoundOutputListener
{
public:

    SoundAnalyzer() { m_bOk = false; }
    virtual ~SoundAnalyzer() { End(); }

    bool Init();
    virtual void End();
    bool IsOk() const { return m_bOk; }

    // Start or stop the debugger.
    void SetActive(bool active);
    // Whether the emulated machine is being debugged or not.
    bool IsActive() const { return m_active; }

    /** From IWinSoundOutputListener. */
    virtual void OnNewSoundSample(float sampleMixed, float sampleChannelA, float sampleChannelB, float sampleChannelC) override;

    void DrawGui();

private:

    static constexpr unsigned NumSamples = unsigned(0.5/*seconds*/ * double(CWinSoundOutput::SAMPLES_PER_SEC));

    void ResetVars();
    void FreeVars();

    void ClearBuffers();

    void DrawChannel(const char* label, int channelIndex, float* samples, ImVec2 graphSize);

    bool m_bOk;
    bool m_active;

    float m_samplesMixed[NumSamples];
    float m_samplesChannelA[NumSamples];
    float m_samplesChannelB[NumSamples];
    float m_samplesChannelC[NumSamples];
    unsigned m_nextPosition;
};
