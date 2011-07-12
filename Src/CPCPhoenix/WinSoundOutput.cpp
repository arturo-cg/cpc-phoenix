//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "WinSoundOutput.h"



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void CALLBACK WinSoundOutput_waveOutProc(HWAVEOUT hDevice, UINT uMsg, DWORD_PTR dwUserData, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  CWinSoundOutput* pThis = (CWinSoundOutput*) dwUserData;

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
      WAVEHDR* pHeader = (WAVEHDR*) dwParam1;
      CWinSoundOutput::SSoundBlock& block = pThis->m_soundBlocks[pHeader->dwUser];

      // Mark the block as not playing anymore
      block.bIsPlaying = false;
    }
    break;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
/**
** 
*/
bool CWinSoundOutput::Init()
{
  bool bOk = true;

  ResetVars();

  // Open the sound playback device
  if (bOk)
  {
    WAVEFORMATEX waveFormat;
    ::memset( &waveFormat, 0, sizeof(WAVEFORMATEX) );
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;            // Simple PCM format
    waveFormat.nChannels = 1;                           // Mono
    waveFormat.nSamplesPerSec = SAMPLES_PER_SEC;        // 44.1 kHz
    waveFormat.wBitsPerSample = BYTES_PER_SAMPLE * 8;   // 8 bits per sample
    waveFormat.nBlockAlign = BYTES_PER_SAMPLE * waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    MMRESULT result;
    result = ::waveOutOpen( &m_hDevice, WAVE_MAPPER, &waveFormat,
                            (DWORD_PTR)WinSoundOutput_waveOutProc/*dwCallback*/, (DWORD_PTR)this/*dwCallbackInstance*/,
                            CALLBACK_FUNCTION );
    if (result != MMSYSERR_NOERROR)
    {
      char szErrorMessage[MAXERRORLENGTH];
      ::waveOutGetErrorText( result, szErrorMessage, sizeof(szErrorMessage) );

      KMASSERTM( result == MMSYSERR_NOERROR, ("Failed to open the audio output device.\n\"%s\"", szErrorMessage) );
      bOk = false;
    }
  }

  // Create and prepare sound blocks
  if (bOk)
  {
    CreateSoundBlocks();

    // Send blocks 0 and 1 to the device and mark block 2 as the next to be written
    SendSoundBlockToDevice( &m_soundBlocks[0] );
    SendSoundBlockToDevice( &m_soundBlocks[1] );

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
  // Force the device to finish with current playing blocks
  if (m_hDevice != 0)
  {
    ::waveOutReset( m_hDevice );
  }

  // Destroy sound blocks
  DestroySoundBlocks();

  // Close the sound playback device
  if (m_hDevice != 0)
  {
    ::waveOutClose( m_hDevice );
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
  m_fVolume = 1.f;

  unsigned i;
  for (i = 0; i < NUM_BLOCKS; i++)
  {
    SSoundBlock& currBlock = m_soundBlocks[i];
    currBlock.pSamples = NULL;
    currBlock.bIsPlaying = false;
  }

  m_nCurrBlock = 0;
  m_nCurrPos = 0;
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
  unsigned nBlockLength;
  nBlockLength = SAMPLES_PER_BLOCK * BYTES_PER_SAMPLE;

  unsigned i;
  for (i = 0; i < NUM_BLOCKS; i++)
  {
    SSoundBlock& currBlock = m_soundBlocks[i];

    // Reserve memory for the block data
    currBlock.pSamples = new unsigned char [nBlockLength];

    // Prepare the block header
    ::memset( &currBlock.header, 0, sizeof(currBlock.header) );
    currBlock.header.lpData = (LPSTR) currBlock.pSamples;
    currBlock.header.dwBufferLength = nBlockLength;
    currBlock.header.dwBytesRecorded = 0;
    currBlock.header.dwUser = i;
    currBlock.header.dwFlags = 0;
    currBlock.header.dwLoops = 0;
    currBlock.header.lpNext = 0;
    currBlock.header.reserved = 0;

    MMRESULT result;
    result = ::waveOutPrepareHeader( m_hDevice, &currBlock.header, sizeof(currBlock.header) );
    KMASSERT( result == MMSYSERR_NOERROR );

    // Fill the block with silence
    ::memset( currBlock.pSamples, 128, nBlockLength );

    currBlock.bIsPlaying = false;
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
void CWinSoundOutput::DestroySoundBlocks()
{
  unsigned i;
  for (i = 0; i < NUM_BLOCKS; i++)
  {
    SSoundBlock& currBlock = m_soundBlocks[i];

    if (m_hDevice != 0)
    {
      MMRESULT result;
      result = ::waveOutUnprepareHeader( m_hDevice, &currBlock.header, sizeof(currBlock.header) );
      KMASSERT( result == MMSYSERR_NOERROR );
    }

    delete [] currBlock.pSamples;
  }
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
/*virtual*/ void CWinSoundOutput::WriteSample(float fSample)
{
  SSoundBlock& writeBlock = m_soundBlocks[m_nCurrBlock];

  // Check whether the current block to be written is not being played by the device.
  // If it is being played, ignore the sample. This will happen when the emulator is executing faster than 100%.
  if (!writeBlock.bIsPlaying)    // If the block to be written is not being used by the device...
  {
    // Convert the sample to the device format
    unsigned char nSample;
    nSample = (unsigned char) ( (fSample * m_fVolume * 127.5f) + 127.5f );

    // Write the sample to the current block
    *(writeBlock.pSamples + (m_nCurrPos * BYTES_PER_SAMPLE)) = nSample;

////{
////  static unsigned char nLastSample = nSample;
////  if (nSample != nLastSample)
////  {
////    static char szBuffer[300];
////    _snprintf( szBuffer, sizeof(szBuffer), "Sample: %.2f --> %d\n", fSample, nSample );
////    OutputDebugString( szBuffer );
////
////    nLastSample = nSample;
////  }
////}

    // Advance position
    m_nCurrPos++;
    if (m_nCurrPos >= SAMPLES_PER_BLOCK)     // If the block has been fully written...
    {
      // Write the block to the sound device
      SendSoundBlockToDevice( &writeBlock );

      // Set the position to the next block
      m_nCurrBlock = (m_nCurrBlock + 1) % NUM_BLOCKS;
      m_nCurrPos = 0;
    }
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
void CWinSoundOutput::SendSoundBlockToDevice(SSoundBlock* pBlock)
{
  KMASSERT( pBlock != NULL );

  // Send the block to the device
  MMRESULT result;
  result = ::waveOutWrite( m_hDevice, &pBlock->header, sizeof(pBlock->header) );
  KMASSERT( result == MMSYSERR_NOERROR );

  // Mark the block as being played
  pBlock->bIsPlaying = true;
}
