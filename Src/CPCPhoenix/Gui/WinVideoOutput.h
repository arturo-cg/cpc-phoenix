//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _WINVIDEOOUTPUT_
#define _WINVIDEOOUTPUT_


#include "cpcVideoOutput.h"


/**
** A frame listener that draws pixels into a window.
*/
class CWinVideoOutput : public CPC::CVideoOutput
{
public:

                            CWinVideoOutput           (CPC::CMachine *pMachine, CDC *pTargetDc);
    virtual                ~CWinVideoOutput           ()  { FreeVars(); }

    /** Called by the emulation code to notify that a new frame is ready to be drawn. */
    virtual void            NotifyNewFrame            ();


private:

    typedef                 CPC::CVideoOutput         inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();


    /** The device context where the pixels are drawn. */
    CDC                    *m_pDc;

};


#endif // _WINVIDEOOUTPUT_
