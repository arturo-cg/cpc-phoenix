//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _DISPLAYWINDOW_H_
#define _DISPLAYWINDOW_H_


#include "Window/kmbWindow.h"


/**
**
*/
class DisplayWindow : public kmbWindow
{
public:

    DisplayWindow() { }
    virtual                  ~DisplayWindow() { End(false); }

    bool                      Init(const RECT& placement, kmbWindow* pParent);
    virtual void              End(bool bIncludedSuper = true);


    //
    // Message handlers (INTERNAL USE ONLY)
    //

    // Return false to continue with the normal message handler, or true to exit the window procedure without processing the message.
    virtual bool              _OnWindowProcedureProlog(UINT uMsg, WPARAM wParam, LPARAM lParam);
    // Returns 0 to continue the window creation, or -1 to cancel it.
    virtual LRESULT           _OnPaint(HDC hDc);
    virtual LRESULT           _OnEraseBkgnd();


private:

    typedef                   kmbWindow                 Super;


    void                      ResetVars();
    void                      FreeVars();

};

#endif // _DISPLAYWINDOW_H_
