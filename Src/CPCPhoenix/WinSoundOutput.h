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
public:

                          CWinSoundOutput           ();
  virtual                ~CWinSoundOutput           ()  { FreeVars(); }

  /** Called by the emulator when the emulated machine is reset. */
  virtual void            Reset                     ();

  /** Called by the emulator to output a new audio sample. The sample format is always 8-bit mono. */
  virtual void            WriteSample               (cpcByte nSample);


private:

  void                    ResetVars                 ();
  void                    FreeVars                  ();

};

#endif // _WINSOUNDOUTPUT_H_
