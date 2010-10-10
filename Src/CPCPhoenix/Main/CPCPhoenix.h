#ifndef _CPCPHOENIX_H_
#define _CPCPHOENIX_H_


#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif


#include "resource.h"       // main symbols

class CAppWnd;

namespace CPC {
    class CMachine;
}


class CCPCPhoenixApp : public CWinApp
{
public:

                              CCPCPhoenixApp            ();

    virtual BOOL              InitInstance              ();

    /** Returns the application main window. */
    CAppWnd                  *GetAppWnd                 ()  { return m_pAppWnd; }

    /** Returns the emulated machine. */
    CPC::CMachine            *GetMachine                ()  { return m_pMachine; }


    afx_msg void              OnAppAbout                ();


private:


    virtual BOOL              OnIdle                    (LONG lCount);


    /** The application main window. */
    CAppWnd                  *m_pAppWnd;

    /** The emulated machine. */
    CPC::CMachine            *m_pMachine;


    DECLARE_MESSAGE_MAP()

};

extern CCPCPhoenixApp theApp;


#endif //_CPCPHOENIX_H_
