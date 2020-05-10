//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AppWindow.h"
#include "Application.h"
#include "DisplayWindow.h"
#include "StatusBar.h"
#include "WinVideoOutput.h"
#include "cpcMachine.h"
#include "cpcKeyboard.h"

#include <Windows.h>
#include "resource.h"


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



//----------------------------------------------------------------------------
/**
** Init
*/
bool AppWindow::Init()
{
    bool bRet = true;

    End(false);
    ResetVars();

    // Initialize base class
    if (bRet)
    {
        DWORD dwStyles;
        dwStyles = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

        bRet = Super::Init("CPCPhoenix", dwStyles, 0/*x*/, 0/*y*/, 1/*width*/, 1/*height*/, NULL/*hParentOrOwner*/);
    }

    // Check parameters
    if (bRet)
    {
        //...
    }

    // Main menu
    if (bRet)
    {
        m_hMainMenu = ::LoadMenu(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINMENU));
        KMASSERTM(m_hMainMenu != NULL, ("Could not create the main menu. GetLastError() == %d", ::GetLastError()));
        if (m_hMainMenu != NULL)
        {
            ::SetMenu(this->GetHWnd(), m_hMainMenu);
        }
    }

    // DisplayWindow
    if (bRet)
    {
        RECT rWndRect;
        ::SetRect(&rWndRect, 0, 0, CPC::CVideoOutput::VIEWPORT_WIDTH, CPC::CVideoOutput::VIEWPORT_HEIGHT);

        m_pDisplayWindow = new DisplayWindow;
        m_pDisplayWindow->Init(rWndRect, this);
    }

    // Status bar
    if (bRet)
    {
        m_pStatusBar = new StatusBar;
        m_pStatusBar->Init(this);
    }

    // Resize application window
    if (bRet)
    {
        ResizeToScale(Application::Singleton()->GetSettings()->GetScale());
    }

    // Key accelerators
    if (bRet)
    {
        m_hAccelerators = ::LoadAccelerators(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_APPWINDOWACCELERATORS));
    }

    // Update GUI based on settings
    if (bRet)
    {
        OnApplicationSettingsChanged();
    }


    if (!bRet)
    {
        FreeVars();
        m_bOk = false;
    }

    return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void AppWindow::End(bool bIncludedSuper/* = true*/)
{
    if (IsOk())
    {
        FreeVars();

        if (bIncludedSuper)
        {
            Super::End();
        }
    }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void AppWindow::ResetVars()
{
    m_hMainMenu = NULL;
    m_hAccelerators = NULL;
    m_pDisplayWindow = NULL;
    m_pStatusBar = NULL;
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void AppWindow::FreeVars()
{
    delete m_pStatusBar;
    delete m_pDisplayWindow;
}

//----------------------------------------------------------------------------
/**
**
*/
void AppWindow::ResizeToScale(float scale)
{
    // Calculate the window rectangle
    DWORD dwStyles;
    dwStyles = (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

    RECT rWndRect;
    int nNewWidth = int(CPC::CVideoOutput::VIEWPORT_WIDTH * scale);
    int nNewHeight = int(CPC::CVideoOutput::VIEWPORT_HEIGHT * 2 * scale) + m_pStatusBar->GetHeight();

    ::SetRect(&rWndRect, 0, 0, nNewWidth, nNewHeight);
    ::AdjustWindowRect(&rWndRect, dwStyles, TRUE/*bMenu*/);
    ::OffsetRect(&rWndRect, -rWndRect.left, -rWndRect.top);

    // Resize it
    SetRect(rWndRect);
}

//----------------------------------------------------------------------------
/**
**
*/
void AppWindow::OnApplicationSettingsChanged()
{
    const Settings* pSettings;
    pSettings = Application::Singleton()->GetSettings();

    // Update the menu
    UINT nItem;
    nItem = ID_SETTINGS_CPCMODEL_CPC464/*first item*/ + Application::Singleton()->FindMachineSpecificationsOrderedPosition(pSettings->GetMachineSpecificationName());
    ::CheckMenuRadioItem(m_hMainMenu, ID_SETTINGS_CPCMODEL_CPC464, ID_SETTINGS_CPCMODEL_CPC6128_MAXAM, nItem, MF_BYCOMMAND);

    switch (pSettings->GetMonitorType())
    {
        case CPC::CGateArray::RGBCONVERSIONTABLE_COLOR:  nItem = ID_SETTINGS_MONITORTYPE_COLOR; break;
        case CPC::CGateArray::RGBCONVERSIONTABLE_GREEN:  nItem = ID_SETTINGS_MONITORTYPE_GREEN; break;
        default:                                         KMASSERT(false); nItem = ID_SETTINGS_MONITORTYPE_COLOR; break;
    }
    ::CheckMenuRadioItem(m_hMainMenu, ID_SETTINGS_MONITORTYPE_COLOR, ID_SETTINGS_MONITORTYPE_GREEN, nItem, MF_BYCOMMAND);

    if (pSettings->GetScale() == 1.0f)
    {
        nItem = ID_SETTINGS_SCALE_1X;
    }
    else if (pSettings->GetScale() == 1.5f)
    {
        nItem = ID_SETTINGS_SCALE_1_5X;
    }
    else
    {
        nItem = ID_SETTINGS_SCALE_1X;
    }
    ::CheckMenuRadioItem(m_hMainMenu, ID_SETTINGS_SCALE_1X, ID_SETTINGS_SCALE_1_5X, nItem, MF_BYCOMMAND);

    ::CheckMenuItem(m_hMainMenu, ID_SETTINGS_DRAWSCANLINES, /*MF_BYCOMMAND | */ pSettings->GetDrawScanLines() ? MF_CHECKED : MF_UNCHECKED);

    if (pSettings->GetEmulationSpeed() < 0.f)
    {
        nItem = ID_SETTINGS_EMULATIONSPEED_UNLIMITED;
    }
    else
    {
        if (pSettings->GetEmulationSpeed() <= 0.251f)
        {
            nItem = ID_SETTINGS_EMULATIONSPEED_25;
        }
        else
        {
            if (pSettings->GetEmulationSpeed() <= 0.51f)
            {
                nItem = ID_SETTINGS_EMULATIONSPEED_50;
            }
            else
            {
                if (pSettings->GetEmulationSpeed() <= 1.01f)
                {
                    nItem = ID_SETTINGS_EMULATIONSPEED_100;
                }
                else
                {
                    if (pSettings->GetEmulationSpeed() <= 1.21f)
                    {
                        nItem = ID_SETTINGS_EMULATIONSPEED_120;
                    }
                    else
                    {
                        nItem = ID_SETTINGS_EMULATIONSPEED_UNLIMITED;
                    }
                }
            }
        }
    }
    ::CheckMenuRadioItem(m_hMainMenu, ID_SETTINGS_EMULATIONSPEED_25, ID_SETTINGS_EMULATIONSPEED_UNLIMITED, nItem, MF_BYCOMMAND);

    // Update the status bar
    m_pStatusBar->SetInsertedDiskNames(pSettings->GetDiskImage(0), pSettings->GetDiskImage(1));
}

//----------------------------------------------------------------------------
/**
**
*/
void AppWindow::OpenLoadDiskImageDialog(unsigned nDrive)
{
    // Show the File Dialog to let the user pick a file
    char szCurrentDir[1000];
    ::GetCurrentDirectory(sizeof(szCurrentDir), szCurrentDir);

    string sInitialDir;
    sInitialDir = szCurrentDir + string("\\Disks");

    char szFileFullPath[1000];
    char szFileName[1000];

    OPENFILENAME openFileName;
    memset(&openFileName, 0, sizeof(openFileName));
    openFileName.lStructSize = sizeof(OPENFILENAME);
    openFileName.hwndOwner = GetHWnd();
    openFileName.lpstrFilter = "DSK disk images (*.dsk)\0*.dsk\0\0";
    //strncpy( szFileFullPath, "", sizeof(szFileFullPath) );
    szFileFullPath[0] = '\0';
    openFileName.lpstrFile = szFileFullPath;
    openFileName.nMaxFile = sizeof(szFileFullPath);
    openFileName.lpstrFileTitle = szFileName;
    openFileName.nMaxFileTitle = sizeof(szFileName);
    openFileName.lpstrInitialDir = sInitialDir.c_str();
    openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;  // The flag OFN_NOCHANGEDIR is ignored on Windows XP and below.
    openFileName.FlagsEx = OFN_EX_NOPLACESBAR;

    if (::GetOpenFileName(&openFileName) != FALSE)
    {
        Application::Singleton()->SetDisk(nDrive, szFileFullPath);
        OnApplicationSettingsChanged();
    }

    // Restore the working directory, changed by the Open File Dialog
    ::SetCurrentDirectory(szCurrentDir);
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ bool AppWindow::_OnWindowProcedureProlog(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Let Dear ImGui process every message.
    bool skipMessage = (ImGui_ImplWin32_WndProcHandler(GetHWnd(), uMsg, wParam, lParam) != 0);
    return skipMessage;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnClose()
{
    Application::Singleton()->_OnAppWindowCloseRequest(this);
    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnSize(int iWidth, int iHeight)
{
    // Resize the status bar
    if (m_pStatusBar != NULL)
    {
        ::SendMessage(m_pStatusBar->GetHWnd(), WM_SIZE, 0, LOWORD(iWidth) | HIWORD(iHeight));
        ::InvalidateRect(m_pStatusBar->GetHWnd(), NULL, FALSE);
    }

    // Resize the display window
    if (m_pDisplayWindow != NULL)
    {
        m_pDisplayWindow->SetSize(iWidth, iHeight - m_pStatusBar->GetHeight());
        m_pDisplayWindow->InvalidateAll(FALSE);
    }

    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
LRESULT AppWindow::_OnMenuCommand(WORD nItemId, bool bFromAccelerator)
{
    Application* pApplication;
    pApplication = Application::Singleton();

    CPC::CMachine* pEmulatedMachine;
    pEmulatedMachine = pApplication->GetEmulatedMachine();

    switch (nItemId)
    {
        //
        // File Menu
        //

        case ID_DRIVEA_INSERTDISK:  OpenLoadDiskImageDialog(0); break;
        case ID_DRIVEA_EJECTDISK:   pApplication->SetDisk(0, ""); OnApplicationSettingsChanged(); break;
        case ID_DRIVEB_INSERTDISK:  OpenLoadDiskImageDialog(1); break;
        case ID_DRIVEB_EJECTDISK:   pApplication->SetDisk(1, ""); OnApplicationSettingsChanged(); break;

        case ID_FILE_EXIT:  RequestClose(); break;


        //
        // Settings Menu
        //

        case ID_SETTINGS_CPCMODEL_CPC464:
        case ID_SETTINGS_CPCMODEL_CPC664:
        case ID_SETTINGS_CPCMODEL_CPC6128:
        case ID_SETTINGS_CPCMODEL_CPC6128_MAXAM:
        {
            unsigned position = nItemId - ID_SETTINGS_CPCMODEL_CPC464/*first item*/;
            const string* name = pApplication->GetMachineSpecificationsNameAtPosition(position);
            pApplication->ChangeMachineSpecificationName(*name);
            break;
        }

        case ID_SETTINGS_MONITORTYPE_COLOR:  pApplication->ChangeMonitorTypeSetting(CPC::CGateArray::RGBCONVERSIONTABLE_COLOR); break;
        case ID_SETTINGS_MONITORTYPE_GREEN:  pApplication->ChangeMonitorTypeSetting(CPC::CGateArray::RGBCONVERSIONTABLE_GREEN); break;

        case ID_SETTINGS_SCALE_1X:    pApplication->ChangeScaleSetting(1.0f); break;
        case ID_SETTINGS_SCALE_1_5X:  pApplication->ChangeScaleSetting(1.5f); break;

        case ID_SETTINGS_DRAWSCANLINES:  pApplication->ChangeDrawScanLinesSetting(!pApplication->GetSettings()->GetDrawScanLines()); break;

        case ID_SETTINGS_EMULATIONSPEED_25:         pApplication->ChangeEmulationSpeedSetting(0.25f); break;
        case ID_SETTINGS_EMULATIONSPEED_50:         pApplication->ChangeEmulationSpeedSetting(0.5f); break;
        case ID_SETTINGS_EMULATIONSPEED_100:        pApplication->ChangeEmulationSpeedSetting(1.f); break;
        case ID_SETTINGS_EMULATIONSPEED_120:        pApplication->ChangeEmulationSpeedSetting(1.19808f); break;     // = 19968.0 (50.08 fps) / 16666.6667 (60 fps)  <- It speeds up the emulation so that it completes a new frame at a 60 Hz rate.
        case ID_SETTINGS_EMULATIONSPEED_UNLIMITED:  pApplication->ChangeEmulationSpeedSetting(-1.f); break;

        case ID_SETTINGS_RESET:  pEmulatedMachine->Reset(); break;
    }

    return 0;
}

//----------------------------------------------------------------------------
/**
**
*/
/*virtual*/ LRESULT AppWindow::_OnKeyDown(unsigned nVirtualKey)
{
    return 0;
}
