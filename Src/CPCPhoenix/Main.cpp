#include "stdafx.h"
#include "Application.h"

void ParseCommandLine(SettingsOverrides* overrides);

//----------------------------------------------------------------------------
/**
** WinMain - The entry point to the executable
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    bool bOk = true;

    // Parse the command line.
    SettingsOverrides overridesFromCommandLine;
    ParseCommandLine(&overridesFromCommandLine);

    // Initialize the application
    new Application;
    bOk = Application::Singleton()->Init(overridesFromCommandLine);

    if (bOk)
    {
        // Enter the application's main loop.
        Application::Singleton()->MainLoop();

        // Terminate the application
        Application::Singleton()->End();
    }

    // Destroy the application
    delete Application::Singleton();

    return (bOk ? 0 : 1);
}

void ParseCommandLine(SettingsOverrides* overrides)
{
    if (overrides != nullptr)
    {
        // Reset overrides.
        *overrides = SettingsOverrides();

        // Read values from the command line.
        for (int i = 1; i < __argc; i++)
        {
            if (strcmp(__argv[i], "--diskA") == 0)
            {
                overrides->diskA.assign(__argv[i + 1]);
            }
            else if (strcmp(__argv[i], "--diskA_archive") == 0)
            {
                overrides->diskA_archive.assign(__argv[i + 1]);
            }
            else if (strcmp(__argv[i], "--diskB") == 0)
            {
                overrides->diskB.assign(__argv[i + 1]);
            }
            else if (strcmp(__argv[i], "--diskB_archive") == 0)
            {
                overrides->diskB_archive.assign(__argv[i + 1]);
            }
            else if (strcmp(__argv[i], "--tape") == 0)
            {
                overrides->tape.assign(__argv[i + 1]);
            }
            else if (strcmp(__argv[i], "--autotype") == 0)
            {
                overrides->autotype.assign(__argv[i + 1]);
            }
        }
    }
}
