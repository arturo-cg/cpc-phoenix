//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCVIDEOOUTPUT
#define _CPCVIDEOOUTPUT


namespace CPC {


class CMachine;


/**
** An instance of this class that has been registered in an emulated machine will be notified
** through its CVideoOutput::NotifyNewFrame method each time the machine is prepared to
** render a new frame (50 times per second).
**
** Host applications must derive this class and implement the method CVideoOutput::NotifyNewFrame
** in order to render the new frame (in a window, a texture or whatever).
*/
class CVideoOutput
{
public:

    enum
    {
        MAX_SCREEN_WIDTH  = 640,  // *** TODO - TODO - TODO - Support custom resolutions
        MAX_SCREEN_HEIGHT = 200,  // *** TODO - TODO - TODO - Support custom resolutions
    };

    virtual                ~CVideoOutput            ()  { FreeVars(); }

    /** Called by the emulation code to notify that a new frame is ready to be drawn. */
    virtual void            NotifyNewFrame            () = 0;


protected:

  // Constructor is protected to avoid instantiating this class
                            CVideoOutput            (CMachine *pMachine);

    void                    ResetVars                 ();
    void                    FreeVars                  ();

    /** Returns the machine this subsystem is part of. */
    inline CMachine        *GetMachine                ()        { return m_pMachine; }
    inline const CMachine  *GetMachine                () const  { return m_pMachine; }


    CMachine               *m_pMachine;


private:

};


} //namespace CPC

#endif // _CPCVIDEOOUTPUT
