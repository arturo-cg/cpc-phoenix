#pragma once

#include "cpcSoundOutput.h"

class kmbFile;

class IWinSoundOutputListener
{
public:
    virtual void OnNewSoundSample(float sampleMixed, float sampleChannelA, float sampleChannelB, float sampleChannelC) = 0;
};

/**
** This class implements the CPC::CSoundOutput interface to provide sound output functionality to the emulator
** using the low-level sound functions of the Windows SDK. Additionally, it allows recording the sound to
** a .WAV file.
*/
class CWinSoundOutput : public CPC::CSoundOutput
{
    friend void CALLBACK WinSoundOutput_waveOutProc(HWAVEOUT hDevice, UINT uMsg, DWORD_PTR dwUserData, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

public:

    static const unsigned   SAMPLES_PER_SEC = 44100;                      // 44.1 kHz sample rate

    CWinSoundOutput();
    virtual                ~CWinSoundOutput() { FreeVars(); }

    bool                    Init(OutputChannelCount outputChannelCount);
    void                    End();

    /** Called by the emulator when the emulated machine is reset. */
    virtual void            Reset();

    /** Sets the volume of the sound sent to the device. Range [0,1]. */
    void                    SetVolume(float fVolume) { m_linearVolume = fVolume; m_exponentialVolume = ComputeExponentialVolumeFromLinear(m_linearVolume); }
    /** Gets the volume of the sound sent to the device. Range [0,1]. */
    float                   GetVolume() const { return m_linearVolume; }

    /** Starts recording to a WAV file. */
    bool                    StartRecording(const string& sFileName);
    /** Stops recording to a WAV file. */
    void                    StopRecording();
    /** Returns true if it is currently recording to a WAV file. */
    bool                    IsRecording() const { return (m_pRecordFile != NULL); }

    void                     SetListener(IWinSoundOutputListener* listener) { m_listener = listener; }
    IWinSoundOutputListener* GetListener() const { return m_listener; }

    /** From CPC::CSoundOutput */
    virtual void            Run(unsigned numCycles) override;

private:

    using inherited = CPC::CSoundOutput;

    static const unsigned   BYTES_PER_SAMPLE = 2;                         // 16-bit samples

    static const unsigned   NUM_BLOCKS = 3;                               // Triple buffer
    static const unsigned   SAMPLES_PER_BLOCK_AND_CHANNEL = SAMPLES_PER_SEC / 20;     // 50 ms of sound data per block

    static const float      CYCLES_PER_SAMPLE;        // Every how many cycles we need to generate a sample (chip_clock/sample_rate = 1Mhz/44.1kHz).

    struct SSoundBlock
    {
        WAVEHDR  header;                // Header
        short*   pSamples;              // Samples
        bool     bIsPlaying;            // True if the block has been sent to the device and is playing or is queued for playback. False if ready for receiving new samples.
    };

    struct SWavFileHeader
    {
        // RIFF chunk descriptor
        unsigned nChunkId;              // Contains "RIFF"
        unsigned nChunkSize;            // Equals to 36 + nSubChunk2Size
        unsigned nFormat;               // Contains "WAVE"

        // The "fmt " sub-chunk
        unsigned nSubChunk1Id;          // Contains "fmt "
        unsigned nSubChunk1Size;        // Always 16 for PCM data.
        unsigned short nAudioFormat;    // 1=PCM, 3=IEEE float, 6=8-bit ITU-T G.711 A-law, etc.
        unsigned short nNumChannels;    // 1=Mono, 2=Stereo, etc.
        unsigned nSampleRate;           // 8000, 44100, etc.
        unsigned nByteRate;             // Equals to: nSampleRate * nNumChannels * nBitsPerSample/8
        unsigned short nBlockAlign;     // Equals to: nNumChannels * nBitsPerSample/8
        unsigned short nBitsPerSample;  // 8, 16, 32, etc.

        // The "data" sub-chunk
        unsigned nSubChunk2Id;          // Contains "data"
        unsigned nSubChunk2Size;        // Size of the sample data in bytes: Num_samples * nNumChannels * nBitsPerSample/8;

        // ... Here go the actual sound samples (which is nSubChunk2Size bytes long) ...
    };


    void                    ResetVars();
    void                    FreeVars();

    void                    CreateSoundBlocks();
    void                    DestroySoundBlocks();

    void                    WriteSample(float leftSample, float rightSample = 0.f);
    void                    SendSoundBlockToDevice(SSoundBlock* pBlock);

    void                    MixSamplesFromPsgAndTape(float* leftSample, float* rightSample);
    float                   ComputeExponentialVolumeFromLinear(float linearVolume) const;


    HWAVEOUT                m_hDevice;
    float                   m_linearVolume;
    float                   m_exponentialVolume;
    unsigned                m_numOutputChannels;    // 1 for mono, 2 for stereo.
    IWinSoundOutputListener* m_listener;

    std::vector<SSoundBlock> m_soundBlocks;
    unsigned                m_nCurrBlock;
    unsigned                m_nCurrPos;    // The position (in samples) inside the m_nCurrBlock of the next byte to be written.

    kmbFile*                m_pRecordFile;
    unsigned                m_nRecordedSampleCount;

    float                   m_accumCycles;           // Used to determine when to write a new sample.
};
