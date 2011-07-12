//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _WINSOUNDOUTPUT_H_
#define _WINSOUNDOUTPUT_H_


#include "cpcSoundOutput.h"


/**
** This class implements the CPC::CSoundOutput interface to provide sound output functionality to the emulator.
** It uses the low-level sound functions of the Windows SDK.
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

  /** Called by the emulator to output a new audio sample. The sample format is always a 32-bit float in the range [-1,1] with one channel (mono). */
  virtual void            WriteSample               (float fSample);

  /** Sets the volume of the sound sent to the device. Range [0,1]. */
  void                    SetVolume                 (float fVolume)  { m_fVolume = fVolume; }


private:

  static const unsigned   SAMPLES_PER_SEC = 44100;                      // 44.1 kHz sample rate
  static const unsigned   BYTES_PER_SAMPLE = 1;                         // 8-bit samples

  static const unsigned   NUM_BLOCKS = 3;                               // Triple buffer
  static const unsigned   SAMPLES_PER_BLOCK = SAMPLES_PER_SEC / 10;     // 100 ms of sound data per block

  struct SSoundBlock
  {
    WAVEHDR        header;               // Header
    unsigned char* pSamples;             // Samples
    bool           bIsPlaying;           // True if the block has been sent to the device and is playing or is queued for playback. False if ready for receiving new samples.
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

};

#endif // _WINSOUNDOUTPUT_H_
