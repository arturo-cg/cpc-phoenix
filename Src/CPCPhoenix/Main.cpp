//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "Application.h"



//----------------------------------------------------------------------------
/**
** WinMain - The entry point to the executable
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    bool bOk = true;

    // Initialize Windows stuff

    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    // Initialize the application
    new Application;
    bOk = Application::Singleton()->Init(hInstance);

    if (bOk)
    {
        // Run the application
        Application::Singleton()->Run();

        // Terminate the application
        Application::Singleton()->End();
    }

    // Destroy the application
    delete Application::Singleton();

    return (bOk ? 0 : 1);
}
