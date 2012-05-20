//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _WINSOUNDOUTPUT_H_
#define _WINSOUNDOUTPUT_H_


#include "cpcSoundOutput.h"

class kmbFile;


/**
** This class implements the CPC::CSoundOutput interface to provide sound output functionality to the emulator
** using the low-level sound functions of the Windows SDK. Additionally, it allows recording the sound to
** a .WAV file.
*/
class CWinSoundOutput : public CPC::CSoundOutput
{
  friend void CALLBACK WinSoundOutput_waveOutProc(HWAVEOUT hDevice, UINT uMsg, DWORD_PTR dwUserData, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

public:

                          CWinSoundOutput           ();
  virtual                ~CWinSoundOutput           ()  { FreeVars(); }

  bool                    Init                      ();
  void                    End                       ();

  /** Called by the emulator when the emulated machine is reset. */
  virtual void            Reset                     ();

  /** From CPC::CSoundOutput */
  virtual void            WriteSample               (float fSample);

  /** Sets the volume of the sound sent to the device. Range [0,1]. */
  void                    SetVolume                 (float fVolume)  { m_fVolume = fVolume; }

  /** Starts recording to a WAV file. */
  bool                    StartRecording            (const string& sFileName);
  /** Stops recording to a WAV file. */
  void                    StopRecording             ();
  /** Returns true if it is currently recording to a WAV file. */
  bool                    IsRecording               () const  { return (m_pRecordFile != NULL); }


private:

  static const unsigned   SAMPLES_PER_SEC = 44100;                      // 44.1 kHz sample rate
  static const unsigned   BYTES_PER_SAMPLE = 2;                         // 8-bit samples

  static const unsigned   NUM_BLOCKS = 3;                               // Triple buffer
  static const unsigned   SAMPLES_PER_BLOCK = SAMPLES_PER_SEC / 20;     // 50 ms of sound data per block

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


  void                    ResetVars                 ();
  void                    FreeVars                  ();

  void                    CreateSoundBlocks         ();
  void                    DestroySoundBlocks        ();

  void                    SendSoundBlockToDevice    (SSoundBlock* pBlock);


  HWAVEOUT                m_hDevice;
  float                   m_fVolume;

  SSoundBlock             m_soundBlocks[NUM_BLOCKS];
  unsigned                m_nCurrBlock;
  unsigned                m_nCurrPos;    // The position (in samples) inside the m_nCurrBlock of the next byte to be written.

  kmbFile*                m_pRecordFile;
  unsigned                m_nRecordedSampleCount;

};

#endif // _WINSOUNDOUTPUT_H_
