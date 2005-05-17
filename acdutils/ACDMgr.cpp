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

// ACDMgr.cpp : Defines the entry point for the console application.

#include "resource.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include "ACDUtil.h"

#include <stdlib.h>
#include <iostream>
#include <tchar.h>
#include <string.h>

using namespace std;

/** Return the installation directory path */
static char* GetInstallPath ();

/** EnumWindows helper (send WM_CLOSE to the hwnd owned by PID lParam */
static BOOL CALLBACK SendWmClose (HWND hwnd, LPARAM lParam);

struct FactoryDefaultHelper : CACDHidDevice::EnumHelper
{
    ENUMPROC_STATUS Callback (CACDHidDevice* pDevice)
    {
	ENUMPROC_STATUS status = CACDHidDevice::EnumHelper::Callback (pDevice);
	if (status != ENUMPROC_STATUS_SUCCESS)
	    return status;

	pDevice->SetFlags (
	      CACDHidDevice::ACD_FLAGS_POWER_BUTTON
	    | CACDHidDevice::ACD_FLAGS_BRIGHTNESS_BUTTON
	    | CACDHidDevice::ACD_FLAGS_MANAGED_POWER
		);

	return ENUMPROC_STATUS_CONTINUE;
    }
};

static void
restore_FactoryDefault ()
{
    FactoryDefaultHelper helper;
    CACDHidDevice::EnumDevices (helper);
}

/** Spawn a new ACDMon.exe instance */
static int
start_ACDMon ()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char cmdLine [_MAX_PATH];

    char* path = GetInstallPath ();
    _snprintf (cmdLine, sizeof (cmdLine), "%s\\i386\\acdmon.exe", path);
    cmdLine [sizeof (cmdLine) - 1] = '\0';

    memset (&si, '\0', sizeof (STARTUPINFO));
    si.cb = sizeof (STARTUPINFO );

    BOOL success = CreateProcess (0, cmdLine,
        NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL,
        &si, &pi);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

/** Terminate all instances of ACDMon.exe */
static int
stop_ACDMon ()
{
    ULONG myPID = GetCurrentProcessId ();

    HANDLE hProcesses = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0); 
    if (hProcesses == INVALID_HANDLE_VALUE)
	return EXIT_FAILURE;

    PROCESSENTRY32 pe32;
    ZeroMemory (&pe32, sizeof (pe32));
    pe32.dwSize = sizeof (PROCESSENTRY32);

    if (Process32First (hProcesses, &pe32)) do {
	char* exeName = strrchr (pe32.szExeFile, '\\');
	exeName = exeName ? ++exeName : pe32.szExeFile;

	if (_stricmp (exeName, "acdmon.exe") != 0)
	    continue;

	HANDLE hProcess = OpenProcess (
	    SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);

	if (!hProcess) {
	    // enable SE_DEBUG_NAME and try again

	    TOKEN_PRIVILEGES Priv, PrevPriv;
	    HANDLE hToken;

	    // open the process token
	    if (!OpenProcessToken (GetCurrentProcess (),
		    TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
                continue;

	    Priv.PrivilegeCount = 1;
	    Priv.Privileges [0].Attributes = SE_PRIVILEGE_ENABLED;
	    LookupPrivilegeValue (
		NULL, SE_DEBUG_NAME, &Priv.Privileges [0].Luid);

	    // enable the privilege
	    DWORD dwLength;
	    if (!AdjustTokenPrivileges (hToken, FALSE, &Priv, sizeof (PrevPriv),
		    &PrevPriv, &dwLength)) {
   		CloseHandle (hToken);
		continue;
	    }

	    // try now that we have SE_DEBUG_NAME
	    hProcess = OpenProcess (
		SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
        
	    // restore the previous privilege state
	    AdjustTokenPrivileges (hToken, FALSE, &PrevPriv, 0, NULL, NULL);

	    CloseHandle (hToken);
	    if (!hProcess)
		continue;
	}

	// first, try to send the WM_CLOSE message.
	::EnumWindows (SendWmClose, pe32.th32ProcessID);

	if (WaitForSingleObject (hProcess, 1000 != WAIT_OBJECT_0))
	    // WM_CLOSE did not succeed, Kill it!
	    TerminateProcess (hProcess, EXIT_SUCCESS);

	CloseHandle (hProcess); 
    }
    while (Process32Next(hProcesses, &pe32)); 

    CloseHandle (hProcesses);

    return EXIT_SUCCESS;
}

static void
install_ACDPowerService ()
{
    SC_HANDLE hACDPowerService, hSCM;
    char exePath [_MAX_PATH];

    char* path = GetInstallPath ();
    _snprintf (exePath, sizeof (exePath), "%s\\i386\\acdpower.exe", path);
    exePath [sizeof (exePath) - 1] = '\0';

    hSCM = OpenSCManager (0, 0, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM)
	return;

    // create the service
    hACDPowerService = CreateService (hSCM,
	"ACDPowerService",
	"WinACD Power Button Service",
	SERVICE_ALL_ACCESS,
	SERVICE_WIN32_OWN_PROCESS,
	SERVICE_AUTO_START,
	SERVICE_ERROR_NORMAL,
	exePath, 0, 0, 0, 0, 0
	);

    if (!hACDPowerService) {
	CloseServiceHandle (hSCM);
	return;
    }

    SERVICE_DESCRIPTION desc = {
	"Manages the Apple Cinema Display power button"
    }; 
    ChangeServiceConfig2 (hACDPowerService, SERVICE_CONFIG_DESCRIPTION, &desc);

    // start the service
    StartService (hACDPowerService, 0, 0);

    CloseServiceHandle (hACDPowerService);
    CloseServiceHandle (hSCM);
}

static void
uninstall_ACDPowerService ()
{
    SC_HANDLE hACDPowerService, hSCM;

    hSCM = OpenSCManager (0, 0, SC_MANAGER_ALL_ACCESS);
    if (!hSCM)
	return;

    hACDPowerService = OpenService (hSCM, "ACDPowerService",
	SERVICE_ALL_ACCESS);

    if (hACDPowerService) {
	SERVICE_STATUS status;

	ControlService (hACDPowerService, SERVICE_CONTROL_STOP, &status);
        DeleteService (hACDPowerService);
    }

    CloseServiceHandle (hSCM);
}

int APIENTRY
_tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    for (int i = 1; i < __argc; ++i) {
	if (_stricmp (__argv [i], "/Install") == 0) {
	    restore_FactoryDefault ();
	    install_ACDPowerService ();
	    start_ACDMon ();
	}
	else if (_stricmp (__argv [i], "/Uninstall") == 0) {
	    restore_FactoryDefault ();
	    uninstall_ACDPowerService ();
	    stop_ACDMon ();
	}
	else {
	    cerr << "Unknown option '" << __argv [i] << "'.";
	    return EXIT_FAILURE;
	}
    }

    // should not reach this
    return EXIT_SUCCESS;
}

static BOOL CALLBACK
SendWmClose (HWND hwnd, LPARAM lParam)
{
    DWORD dwProcessID ;

    GetWindowThreadProcessId(hwnd, &dwProcessID) ;

    if(dwProcessID == (DWORD)lParam)
	PostMessage(hwnd, WM_CLOSE, 0, 0) ;

    return TRUE;
}

static char*
GetInstallPath ()
{
    HKEY hKey;
    LONG lRet;

    lRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "SOFTWARE\\WinACD",
	0, KEY_READ, &hKey);
    if (lRet != ERROR_SUCCESS)
	return NULL;

    static char path [_MAX_PATH];
    DWORD type, len = sizeof (path);
    RegQueryValueEx (hKey, "InstallPath", 0, &type, (LPBYTE) path, &len);
    path [sizeof (path) - 1] = '\0';

    return path;
}
