//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCSOUNDOUTPUT_H_
#define _CPCSOUNDOUTPUT_H_


#include "cpcSubSystem.h"


namespace CPC {

  /**
  ** This class provides sound output functionality to the emulator. It is a pure virtual class. The front-end must implement
  ** a class derived from this one and register it via the method CMachine::SetSoundOutput.
  **
  ** As the emulator runs, it calls CSoundOutput::WriteSample periodically. The CSoundOutput-derived class is then responsible
  ** for making these samples sound through the speakers of the host machine.
  */
  class CSoundOutput
  {
  public:

                            CSoundOutput              ();
    virtual                ~CSoundOutput              ()  { FreeVars(); }

    /** Called by the emulator when the emulated machine is reset. */
    virtual void            Reset                     () = 0;

    /** Called by the emulator to output a new audio sample. The sample format is always a 32-bit float in the range [-1,1] with one channel (mono). */
    virtual void            WriteSample               (float fSample) = 0;


  private:

    void                    ResetVars                 ();
    void                    FreeVars                  ();

  };


} //namespace CPC

#endif // _CPCSOUNDOUTPUT_H_
