//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "WinSoundOutput.h"
#include "cpcMachine.h"
#include "cpcTapeDeck.h"
#include "File/kmbFile.h"



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void CALLBACK WinSoundOutput_waveOutProc(HWAVEOUT hDevice, UINT uMsg, DWORD_PTR dwUserData, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    CWinSoundOutput* pThis = (CWinSoundOutput*)dwUserData;

    switch (uMsg)
    {
        case WOM_OPEN:      // The device has been opened with waveOutOpen
        {
        }
        break;

        case WOM_CLOSE:     // The device has been opened with waveOutClose
        {
        }
        break;

        case WOM_DONE:      // The device is finished with a data block sent with a previous waveOutWrite
        {
            // Get the corresponding CWinSoundOutput::SSoundBlock
            WAVEHDR* pHeader = (WAVEHDR*)dwParam1;
            CWinSoundOutput::SSoundBlock& block = pThis->m_soundBlocks[pHeader->dwUser];

            // Mark the block as not playing anymore
            block.bIsPlaying = false;
        }
        break;
    }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/*static*/ const float CWinSoundOutput::CYCLES_PER_SAMPLE = 1000000.f / 44100.f;  // chip_clock/sample_rate = 1Mhz/44.1kHz = 22.675737

namespace
{

    /**
     ** Converts a sample in the [-1, 1] range to the 16-bit format the sound device takes.
     */
    static short ToDeviceSample(float sample)
    {
        // Clamp first: removing the DC offset overshoots the range for a moment after a large step, and casting an
        // out-of-range value to short wraps around, which is heard as a loud crack.
        return (short)(std::clamp(sample, -1.f, 1.f) * 32767.f);
    }

}

//----------------------------------------------------------------------------
/**
**
*/
float CWinSoundChannel::Read()
{
    // Average the accumulated window, instead of taking the value the channel had at one instant
    const float sample = (m_accumCount > 0 ? m_accum / float(m_accumCount) : 0.f);
    ResetWindow();

    // The PSG output is unipolar, so it carries a DC offset: it sits at 0 while the channel is silent instead of
    // at the centre of the output range. Subtracting a running average of the signal removes it, and is also what
    // leaves the result centred on zero, so no explicit remapping to the [-1, 1] range is needed. That average has
    // to be measured rather than assumed: the centre of the signal is not a constant, it moves with what is being
    // played, so any fixed value picked for it would be wrong for some of the content.
    constexpr float pi = 3.14159265f;
    // Cut-off of the high-pass that removes the DC offset from a channel. It is deliberately this low: a higher
    // one droops within a half-cycle of the lowest notes the PSG can play, and the resulting overshoot clips.
    constexpr float dcCutoffHz = 2.f;
    // Rate at which the running average behind that high-pass has to follow the signal to place the cut-off there.
    constexpr float dcAverageRate = (2.f * pi * dcCutoffHz) / float(CWinSoundOutput::SAMPLES_PER_SEC);
    m_dcAverage += (sample - m_dcAverage) * dcAverageRate;

    return (sample - m_dcAverage);
}


//----------------------------------------------------------------------------
/**
**
*/
bool CWinSoundOutput::Init(OutputChannelCount outputChannelCount)
{
    bool bOk = true;

    ResetVars();

    m_numOutputChannels = (outputChannelCount == OutputChannelCount::Mono ? 1 : 2);

    // Open the sound playback device
    if (bOk)
    {
        WAVEFORMATEX waveFormat;
        ::memset(&waveFormat, 0, sizeof(WAVEFORMATEX));
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;            // Simple PCM format
        waveFormat.nChannels = m_numOutputChannels;         // Mono / stereo
        waveFormat.nSamplesPerSec = SAMPLES_PER_SEC;        // 44.1 kHz
        waveFormat.wBitsPerSample = BYTES_PER_SAMPLE * 8;   // 16 bits per sample
        waveFormat.nBlockAlign = BYTES_PER_SAMPLE * waveFormat.nChannels;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;

        MMRESULT result;
        result = ::waveOutOpen(&m_hDevice, WAVE_MAPPER, &waveFormat,
            (DWORD_PTR)WinSoundOutput_waveOutProc/*dwCallback*/, (DWORD_PTR)this/*dwCallbackInstance*/,
                               CALLBACK_FUNCTION);
        if (result != MMSYSERR_NOERROR)
        {
            char szErrorMessage[MAXERRORLENGTH];
            ::waveOutGetErrorText(result, szErrorMessage, sizeof(szErrorMessage));

            KMASSERTM(result == MMSYSERR_NOERROR, ("Failed to open the audio output device.\n\"%s\"", szErrorMessage));
            bOk = false;
        }
    }

    // Create and prepare sound blocks
    if (bOk)
    {
        CreateSoundBlocks();

        // Blocks of silence queued on startup: 50 ms of margin for ~60 ms of latency
        constexpr unsigned primedBlocks = 5;
        for (unsigned i = 0; i < primedBlocks; i++)
        {
            SendSoundBlockToDevice(&m_soundBlocks[i]);
        }

        m_nCurrBlock = primedBlocks;
        m_nCurrPos = 0;
    }

    return bOk;
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::End()
{
    // Stop recording to a file
    StopRecording();

    // Force the device to finish with current playing blocks
    if (m_hDevice != 0)
    {
        ::waveOutReset(m_hDevice);
    }

    // Destroy sound blocks
    DestroySoundBlocks();

    // Close the sound playback device
    if (m_hDevice != 0)
    {
        ::waveOutClose(m_hDevice);
        m_hDevice = 0;
    }
}

//----------------------------------------------------------------------------
/**
**
*/
CWinSoundOutput::CWinSoundOutput()
{
    //...
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void CWinSoundOutput::ResetVars()
{
    m_hDevice = 0;
    m_linearVolume = 1.f;
    m_exponentialVolume = ComputeExponentialVolumeFromLinear(m_linearVolume);
    m_numOutputChannels = 0;
    m_listener = nullptr;
    m_soundBlocks.clear();
    m_nCurrBlock = 0;
    m_nCurrPos = 0;
    m_pRecordFile = NULL;
    m_accumCycles = 0.f;
    for (CWinSoundChannel& channel : m_channels)
    {
        channel.Reset();
    }
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void CWinSoundOutput::FreeVars()
{
    //...
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::CreateSoundBlocks()
{
    KMASSERT(m_soundBlocks.size() == 0);

    unsigned samplesPerBlock = SAMPLES_PER_BLOCK_AND_CHANNEL * m_numOutputChannels;
    unsigned nBlockLength =  samplesPerBlock * BYTES_PER_SAMPLE;

    unsigned i;
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        SSoundBlock currBlock;

        // Reserve memory for the block data
        currBlock.pSamples = new short[samplesPerBlock];

        // Prepare the block header
        ::memset(&currBlock.header, 0, sizeof(currBlock.header));
        currBlock.header.lpData = (LPSTR)currBlock.pSamples;
        currBlock.header.dwBufferLength = nBlockLength;
        currBlock.header.dwBytesRecorded = 0;
        currBlock.header.dwUser = i;
        currBlock.header.dwFlags = 0;
        currBlock.header.dwLoops = 0;
        currBlock.header.lpNext = 0;
        currBlock.header.reserved = 0;

        MMRESULT result;
        result = ::waveOutPrepareHeader(m_hDevice, &currBlock.header, sizeof(currBlock.header));
        KMASSERT(result == MMSYSERR_NOERROR);

        // Fill the block with silence
        ::memset(currBlock.pSamples, 0, nBlockLength);

        currBlock.bIsPlaying = false;

        m_soundBlocks.push_back(currBlock);
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::DestroySoundBlocks()
{
    for (SSoundBlock& soundBlock : m_soundBlocks)
    {
        if (m_hDevice != 0)
        {
            MMRESULT result;
            result = ::waveOutUnprepareHeader(m_hDevice, &soundBlock.header, sizeof(soundBlock.header));
            KMASSERT(result == MMSYSERR_NOERROR);
        }

        delete[] soundBlock.pSamples;
    }

    m_soundBlocks.clear();
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ void CWinSoundOutput::Reset()
{
    for (CWinSoundChannel& channel : m_channels)
    {
        channel.ResetDcOffset();
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::WriteSample(const std::array<float, MAX_OUTPUT_CHANNELS> &samples)
{
    SSoundBlock& writeBlock = m_soundBlocks[m_nCurrBlock];

    // Check whether the current block to be written is not being played by the device.
    // If it is being played, ignore the sample. This will happen when the emulator is executing faster than 100%.
    if (!writeBlock.bIsPlaying)    // If the block to be written is not being used by the device...
    {
        // Write the sample of every output channel to the block
        for (unsigned channel = 0; channel < m_numOutputChannels; channel++)
        {
            *(writeBlock.pSamples + m_nCurrPos) = ToDeviceSample(samples[channel] * m_exponentialVolume);
            m_nCurrPos++;
        }

        // If the block is complete, send it to the sound device
        if (m_nCurrPos >= (SAMPLES_PER_BLOCK_AND_CHANNEL * m_numOutputChannels))     // If the block has been fully written...
        {
            // Write the block to the sound device
            SendSoundBlockToDevice(&writeBlock);

            // Select the next block to be written
            m_nCurrBlock = (m_nCurrBlock + 1) % NUM_BLOCKS;
            m_nCurrPos = 0;
        }
    }

    // If recording is active, write the sample to the file
    if (IsRecording())
    {
        for (unsigned channel = 0; channel < m_numOutputChannels; channel++)
        {
            short wavSample = ToDeviceSample(samples[channel]);
            m_pRecordFile->WriteBytes(wavSample);
            m_nRecordedSampleCount++;
        }
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::SendSoundBlockToDevice(SSoundBlock* pBlock)
{
    KMASSERT(pBlock != NULL);

    // Send the block to the device
    MMRESULT result;
    result = ::waveOutWrite(m_hDevice, &pBlock->header, sizeof(pBlock->header));
    KMASSERT(result == MMSYSERR_NOERROR);

    // Mark the block as being played
    pBlock->bIsPlaying = true;
}

//----------------------------------------------------------------------------
/**
**
*/
float CWinSoundOutput::ComputeExponentialVolumeFromLinear(float linearVolume) const
{
    static constexpr float a = 1e-3f;
    static constexpr float b = 6.908f;
    return a * expf(b * linearVolume);
}

//----------------------------------------------------------------------------
/**
**
*/
bool CWinSoundOutput::StartRecording(const std::string& sFileName)
{
    bool bRet = true;

    // Stop current recording, if any
    StopRecording();

    // Open the file with write permissions
    m_pRecordFile = new kmbFile;
    m_pRecordFile->Init(sFileName);

    if (m_pRecordFile->Open(kmbFile::ACCESS_WRITE, kmbFile::FILEMODE_BINARY))
    {
        // Write the WAV header with garbage. We will write it again with correct data at the end, once we know the final length.
        SWavFileHeader header;
        memset(&header, 0, sizeof(header));
        m_pRecordFile->WriteBytes(header);

        // The file pointer is now at the beginning of the data block ready for writing sound samples.
        m_nRecordedSampleCount = 0;
    }
    else
    {
        // Error opening the file for writing
        bRet = false;
    }

    return bRet;
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::StopRecording()
{
    if (m_pRecordFile != NULL)
    {
        // Write the file header with correct information
        SWavFileHeader header;
        header.nNumChannels = m_numOutputChannels;
        header.nBitsPerSample = 16;
        header.nSubChunk2Size = m_nRecordedSampleCount * header.nNumChannels * (header.nBitsPerSample / 8);

        header.nChunkId = 'R' | ('I' << 8) | ('F' << 16) | ('F' << 24);
        header.nChunkSize = 36 + header.nSubChunk2Size;
        header.nFormat = 'W' | ('A' << 8) | ('V' << 16) | ('E' << 24);

        header.nSubChunk1Id = 'f' | ('m' << 8) | ('t' << 16) | (' ' << 24);
        header.nSubChunk1Size = 16;
        header.nAudioFormat = 1;
        header.nSampleRate = SAMPLES_PER_SEC;
        header.nByteRate = header.nSampleRate * header.nNumChannels * (header.nBitsPerSample / 8);
        header.nBlockAlign = header.nNumChannels * (header.nBitsPerSample / 8);

        header.nSubChunk2Id = 'd' | ('a' << 8) | ('t' << 16) | ('a' << 24);

        m_pRecordFile->Seek(0);
        m_pRecordFile->WriteBytes(header);

        // Close the file
        m_pRecordFile->Close();
        delete m_pRecordFile;
        m_pRecordFile = NULL;
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::MixSamplesFromPsgAndTape(std::array<float, MAX_OUTPUT_CHANNELS> &samples)
{
    CPC::CPsg* psg = GetMachine()->GetPsg();
    float channelA = IsChannelEnabled(0) ? psg->GetChannelOutput(0) : 0.f;
    float channelB = IsChannelEnabled(1) ? psg->GetChannelOutput(1) : 0.f;
    float channelC = IsChannelEnabled(2) ? psg->GetChannelOutput(2) : 0.f;
    // Mono and stereo are not the same mix repeated per channel, they are different ones: mono adds the three
    // PSG channels together, stereo sends A to one side, C to the other and B to both. Hence the branch.
    if (m_numOutputChannels == 1)
    {
        samples[0] = (channelA + channelB + channelC) / 3.f;
    }
    else
    {
        // Left output channel = PSG A + some % of PSG B
        // Right output channel = PSG C + some % of PSG B
        // Divided by the largest value the sum can reach, so the mix uses as much of the output range as it can
        // without clipping once its DC offset is removed (see CWinSoundChannel::Read).
        constexpr float CHANNEL_B_LEVEL = 0.75f;
        constexpr float MAX_CHANNEL_SUM = 1.f + CHANNEL_B_LEVEL;
        samples[0] = (channelA + channelB * CHANNEL_B_LEVEL) / MAX_CHANNEL_SUM;
        samples[1] = (channelC + channelB * CHANNEL_B_LEVEL) / MAX_CHANNEL_SUM;
    }

    // Mix tape audio in.
    CPC::CTapeDeck* tapeDeck = GetMachine()->GetTapeDeck();
    bool isTapePlaying = (tapeDeck != nullptr) && tapeDeck->IsPlaying();
    if (isTapePlaying)
    {
        float tapeSample = (tapeDeck->GetDataReadSignal() ? 1.f : -1.f);
        for (unsigned channel = 0; channel < m_numOutputChannels; channel++)
        {
            samples[channel] = (samples[channel] + tapeSample) / 2.f;
        }
    }
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::Run(unsigned numCycles)
{
    if (GetMachine() != nullptr)
    {
        for (unsigned cycle = 0; cycle < numCycles; cycle++)
        {
            // Mix samples from the PSG channels and feed them to the output channels, which accumulate them.
            std::array<float, MAX_OUTPUT_CHANNELS> samples;
            MixSamplesFromPsgAndTape(samples);
            for (unsigned channel = 0; channel < m_numOutputChannels; channel++)
            {
                m_channels[channel].Write(samples[channel]);
            }

            // Update cycle accumulator.
            m_accumCycles += 1.f;
            if (m_accumCycles >= CYCLES_PER_SAMPLE)
            {
                m_accumCycles -= CYCLES_PER_SAMPLE;

                for (unsigned channel = 0; channel < m_numOutputChannels; channel++)
                {
                    samples[channel] = m_channels[channel].Read();
                }

                // Send the sample to the host audio system.
                WriteSample(samples);

                // Pass the sample to the listener, if any.
                if (m_listener != nullptr)
                {
                    CPC::CPsg* psg = GetMachine()->GetPsg();
                    m_listener->OnNewSoundSample(samples[0], (psg->GetChannelOutput(0) * 2.f) - 1.f, (psg->GetChannelOutput(1) * 2.f) - 1.f, (psg->GetChannelOutput(2) * 2.f) - 1.f);
                }
            }
        }
    }
}
