// Copyright (C) 2005 Laurent Morichetti
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// ACDMon.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "ACDMon.h"
#include "ACDOsd.h"

#include <tlhelp32.h>
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CACDCommandLineInfo

class CACDCommandLineInfo : public CCommandLineInfo
{
public:
    virtual void ParseParam (LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
};

// CACDMonApp

BEGIN_MESSAGE_MAP (CACDMonApp, CWinApp)
END_MESSAGE_MAP ()


// CACDMonApp construction

CACDMonApp::CACDMonApp ()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CACDMonApp object

CACDMonApp theApp;

// CACDMonApp initialization

BOOL CACDMonApp::InitInstance ()
{
    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    InitCommonControls ();

    CWinApp::InitInstance ();

    // Standard initialization
    SetRegistryKey (_T ("WinACD"));

    // Parse the command line
    CACDCommandLineInfo cmdInfo;
    ParseCommandLine (cmdInfo);

    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object
    CACDBrightnessOsd* mainWnd = new CACDBrightnessOsd ();
    if (!mainWnd)
	return FALSE;

    mainWnd->Create (NULL, NULL);
    m_pMainWnd = mainWnd;

    ::RegisterHotKey (mainWnd->m_hWnd, ACD_BRIGHTNESS_DOWN,
	MOD_CONTROL | MOD_ALT, VK_SUBTRACT);
    ::RegisterHotKey (mainWnd->m_hWnd, ACD_BRIGHTNESS_UP,
	MOD_CONTROL | MOD_ALT, VK_ADD);

    
    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand
    return TRUE;
}

static BOOL CALLBACK
SendReloadCallback (HWND hwnd, LPARAM lParam)
{
    DWORD pid; 

    GetWindowThreadProcessId (hwnd, &pid); 
    if (pid == (ULONG) lParam) {
	SendMessage (hwnd, ACD_WM_RELOAD, 0, 0);
    }
    return TRUE;
}

void
CACDCommandLineInfo::ParseParam (LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
    BOOL stop = FALSE;
    BOOL reload = FALSE;

    if (!bFlag)
	return;

    char cmdLine [_MAX_PATH];
    GetModuleFileName(GetModuleHandle (NULL), cmdLine, sizeof (cmdLine));
	
    if (lstrcmpi (_T ("start"), lpszParam) == 0) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory (&si, sizeof (STARTUPINFO));
	si.cb = sizeof (STARTUPINFO );

	BOOL success = CreateProcess (NULL, cmdLine, 
	    NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL,
	    &si, &pi);

	ExitProcess (success ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    else if (lstrcmpi (_T ("stop"), lpszParam) == 0) {
	stop = TRUE;
    }
    else if (lstrcmpi (_T ("reload"), lpszParam) == 0) {
	reload = TRUE;
    }

    if (!stop && !reload)
	return;

    CString myExeName = PathFindFileName (cmdLine);
    ULONG myPID = GetCurrentProcessId ();

    HANDLE hProcesses = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0); 
    if (hProcesses == INVALID_HANDLE_VALUE)
	exit (EXIT_FAILURE);

    PROCESSENTRY32 pe32;
    ZeroMemory (&pe32, sizeof (pe32));
    pe32.dwSize = sizeof (PROCESSENTRY32);

    if (Process32First (hProcesses, &pe32)) do {
	CString exeName = PathFindFileName (pe32.szExeFile);

	if (pe32.th32ProcessID == myPID
	    || exeName.CompareNoCase (myExeName) != 0)
	    continue;

	HANDLE hProcess = OpenProcess (
	    PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);

	if (stop)
	    TerminateProcess (hProcess, EXIT_SUCCESS);
	else
	    EnumWindows (SendReloadCallback, pe32.th32ProcessID);

	CloseHandle (hProcess); 
    }
    while (Process32Next(hProcesses, &pe32)); 

    CloseHandle (hProcesses);
    exit (EXIT_SUCCESS);
}
