// CPCPhoenix.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CPCPhoenix.h"
#include "AppWnd.h"
#include "cpcMachine.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCPCPhoenixApp

BEGIN_MESSAGE_MAP(CCPCPhoenixApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CCPCPhoenixApp::OnAppAbout)
END_MESSAGE_MAP()


// CCPCPhoenixApp construction

//----------------------------------------------------------------------------
/**
** 
*/
CCPCPhoenixApp::CCPCPhoenixApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CCPCPhoenixApp object

CCPCPhoenixApp theApp;


// CCPCPhoenixApp initialization

//----------------------------------------------------------------------------
/**
** 
*/
BOOL CCPCPhoenixApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
////////////////////////	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    // Reset member variables
    m_pAppWnd     = NULL;
    m_pMachine    = NULL;
    m_uFrameCount = 0;

    // We create the emulated machine instance
    m_pMachine = new CPC::CMachine( CPC::CMachine::CPC_464 );
    m_pMachine->Reset();

    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object
    m_pAppWnd = new CAppWnd;

    if( !m_pAppWnd )
    {
        return FALSE;
    }

    m_pMainWnd = m_pAppWnd;   // Fill the MFC's pointer to the application window

    // create and load the frame with its resources
    m_pAppWnd->LoadFrame(IDR_MAINFRAME,
                         WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
                         NULL);

    m_pAppWnd->GetEmulatorWnd()->SetEmulatedMachine( m_pMachine );

    // The one and only window has been initialized, so show and update it
    m_pAppWnd->ShowWindow(SW_SHOW);
    m_pAppWnd->UpdateWindow();

    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand
    return TRUE;
}


// CCPCPhoenixApp message handlers

//----------------------------------------------------------------------------
/**
** 
*/
/*virtual*/ BOOL CCPCPhoenixApp::OnIdle(LONG lCount)
{
    unsigned nTimeStep = 100;

    // Let the base class do its stuff
    CWinApp::OnIdle( lCount );

    // Run the emulated machine
//****************************************** TODO - TODO - TODO ************************************************
//****************************************** TODO - TODO - TODO ************************************************
    m_pMachine->Run( nTimeStep );

    // Has the emulated machine completed a new video frame?
    if (m_uFrameCount < m_pMachine->GetFrameCount())
    {
      // Grab the new display image
      CEmulatorWnd *pEmulatorWnd;
      pEmulatorWnd = GetAppWnd()->GetEmulatorWnd();

      pEmulatorWnd->UpdateDisplayImage();
      pEmulatorWnd->Invalidate( FALSE );

      m_uFrameCount = m_pMachine->GetFrameCount();
    }
//****************************************** TODO - TODO - TODO ************************************************
//****************************************** TODO - TODO - TODO ************************************************

    return TRUE;  // We want all the available idle time
}




// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CCPCPhoenixApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}


// CCPCPhoenixApp message handlers

