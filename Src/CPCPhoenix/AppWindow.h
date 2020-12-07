//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _APPWINDOW_H_
#define _APPWINDOW_H_


#include "Window/kmbWindow.h"

class DisplayWindow;

namespace CPC
{
    enum ECpcKey;
}


/**
**
*/
class AppWindow : public kmbWindow
{
public:

    AppWindow() { }
    virtual                  ~AppWindow() { End(false); }

    bool                      Init();
    virtual void              End(bool bIncludedSuper = true);


    /** Returns the accelerators used by this window. */
    HACCEL                    GetAccelerators() const { return m_hAccelerators; }

    /** Resizes the app window and the display window to fit the specified scale level. */
    void                      ResizeToScale(float scale);

    //
    // Message handlers (INTERNAL USE ONLY)
    //

    // Return false to continue with the normal message handler, or true to exit the window procedure without processing the message.
    virtual bool              _OnWindowProcedureProlog(UINT uMsg, WPARAM wParam, LPARAM lParam);
    // Returns 0 to continue the window creation, or -1 to cancel it.
    virtual LRESULT           _OnClose();
    virtual LRESULT           _OnSizing(LPRECT prRect);
    virtual LRESULT           _OnSize(int iWidth, int iHeight);
    virtual LRESULT           _OnKeyDown(unsigned nVirtualKey);


private:

    typedef                   kmbWindow                 Super;


    void                      ResetVars();
    void                      FreeVars();

    void                      ComputeDisplayWindowSize(int* out_width, int* out_height);


    HACCEL                    m_hAccelerators;

};

#endif // _APPWINDOW_H_
