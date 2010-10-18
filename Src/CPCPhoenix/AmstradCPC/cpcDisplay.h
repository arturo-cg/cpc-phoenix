//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCDISPLAY_H_
#define _CPCDISPLAY_H_


#include "cpcSubSystem.h"


namespace CPC {


  /**
  ** 
  */
  class CDisplay : public CSubSystem
  {
  public:

    static const unsigned   IMAGEBUFFER_WIDTH  = 640;
    static const unsigned   IMAGEBUFFER_HEIGHT = 400;


                            CDisplay                  (CMachine *pMachine);
    virtual                ~CDisplay                  ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Decodes the display image for the current machine state.
    *** Pixels are written in RGBA format, with 8 bits per channel (giving 32 bits per pixel). pImageBuffer must point to a buffer large enough (>1000Kb). */
    void                    DecodeImage_B8G8R8X8      (unsigned char* pImageBuffer);


  private:

    typedef                 CSubSystem                inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();


  };


} //namespace CPC

#endif // _CPCDISPLAY_H_
