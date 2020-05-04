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
