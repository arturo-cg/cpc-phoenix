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

        // Send blocks 0 and 1 to the device and mark block 2 as the next to be written
        SendSoundBlockToDevice(&m_soundBlocks[0]);
        SendSoundBlockToDevice(&m_soundBlocks[1]);

        m_nCurrBlock = 2;
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
    //...
}

//----------------------------------------------------------------------------
/**
**
*/
void CWinSoundOutput::WriteSample(float leftSample, float rightSample)
{
    SSoundBlock& writeBlock = m_soundBlocks[m_nCurrBlock];

    // Check whether the current block to be written is not being played by the device.
    // If it is being played, ignore the sample. This will happen when the emulator is executing faster than 100%.
    if (!writeBlock.bIsPlaying)    // If the block to be written is not being used by the device...
    {
        // Write mono/left sample to the block
        short deviceSample;
        deviceSample = (short)(leftSample * m_exponentialVolume * 32767.f);
        *(writeBlock.pSamples + m_nCurrPos) = deviceSample;
        m_nCurrPos++;
        // Write right sample to the block, if stereo is being used
        if (m_numOutputChannels >= 2)
        {
            deviceSample = (short)(rightSample * m_exponentialVolume * 32767.f);
            *(writeBlock.pSamples + m_nCurrPos) = deviceSample;
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
        // Mono/left sample
        short wavSample;
        wavSample = short(leftSample * 32767.f);
        m_pRecordFile->WriteBytes(wavSample);
        m_nRecordedSampleCount++;
        // Right sample, if stereo is used.
        if (m_numOutputChannels >= 2)
        {
            wavSample = short(rightSample * 32767.f);
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
bool CWinSoundOutput::StartRecording(const string& sFileName)
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
void CWinSoundOutput::MixSamplesFromPsgAndTape(float* leftSample, float* rightSample)
{
    *leftSample = 0.f;
    *rightSample = 0.f;
    CPC::CPsg* psg = GetMachine()->GetPsg();
    float channelA = IsChannelEnabled(0) ? psg->GetChannelOutput(0) : 0.f;
    float channelB = IsChannelEnabled(1) ? psg->GetChannelOutput(1) : 0.f;
    float channelC = IsChannelEnabled(2) ? psg->GetChannelOutput(2) : 0.f;
    if (m_numOutputChannels == 1)
    {
        *leftSample = (channelA + channelB + channelC) / 3.f;
    }
    else
    {
        // Left output channel = PSG A + some % of PSG B
        // Right output channel = PSG C + some % of PSG B
        constexpr float CHANNEL_B_LEVEL = 0.75f;
        *leftSample = (channelA + channelB * CHANNEL_B_LEVEL) / 2.f;
        *rightSample = (channelC + channelB * CHANNEL_B_LEVEL) / 2.f;
    }

    // Transform samples range from [0, 1] to [-1, 1].
    *leftSample = (*leftSample * 2.f) - 1.f;
    if (m_numOutputChannels >= 2)
    {
        *rightSample = (*rightSample * 2.f) - 1.f;
    }

    // Mix tape audio in.
    CPC::CTapeDeck* tapeDeck = GetMachine()->GetTapeDeck();
    bool isTapePlaying = (tapeDeck != nullptr) && tapeDeck->IsPlaying();
    if (isTapePlaying)
    {
        float tapeSample = (tapeDeck->GetDataReadSignal() ? 1.f : -1.f);
        *leftSample = (*leftSample + tapeSample) / 2.f;
        if (m_numOutputChannels >= 2)
        {
            *rightSample = (*rightSample + tapeSample) / 2.f;
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
        m_accumCycles += (float)numCycles;

        while (m_accumCycles >= CYCLES_PER_SAMPLE)
        {
            // Mix samples from the PSG channels.
            float leftSample;
            float rightSample;
            MixSamplesFromPsgAndTape(&leftSample, &rightSample);

            // Send the sample to the host audio system.
            WriteSample(leftSample, rightSample);

            // Update cycle accumulator.
            m_accumCycles -= CYCLES_PER_SAMPLE;

            // Pass the sample to the listener, if any.
            if (m_listener != nullptr)
            {
                CPC::CPsg* psg = GetMachine()->GetPsg();
                m_listener->OnNewSoundSample(leftSample, (psg->GetChannelOutput(0) * 2.f) - 1.f, (psg->GetChannelOutput(1) * 2.f) - 1.f, (psg->GetChannelOutput(2) * 2.f) - 1.f);
            }
        }
    }
}
