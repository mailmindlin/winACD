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

// ACDPower.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <winsvc.h>

#include "ACDUtil.h"

#include <stdlib.h>
#include <iostream>
#include <tchar.h>

extern "C" {
#include <powrprof.h>
}

static SERVICE_STATUS_HANDLE hServiceStatus; 
static HANDLE hShutdownEvent;

void
ACDDoPowerButtonAction ()
{
    BOOL bForce = ACDUtil::GetForceShutdownPref ();

    switch (ACDUtil::GetPowerButtonActionPref ()) {
    case ACD_POWER_BUTTON_STAND_BY:
	if (IsPwrSuspendAllowed ())
	    SetSuspendState (FALSE, bForce, FALSE);
	break;

    case ACD_POWER_BUTTON_HIBERNATE:
	if (IsPwrHibernateAllowed ())
	    SetSuspendState (TRUE, bForce, FALSE);
	break;

    case ACD_POWER_BUTTON_SHUT_DOWN:
	if (!IsPwrShutdownAllowed ())
	    break;

	// we must acquire the SE_SHUTDOWN_NAME privilege
	TOKEN_PRIVILEGES Priv;
	HANDLE hToken;

	// open the process token
	if (!OpenProcessToken (GetCurrentProcess (),
	    TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	    break;;

	Priv.PrivilegeCount = 1;
	Priv.Privileges [0].Attributes = SE_PRIVILEGE_ENABLED;
	LookupPrivilegeValue (
	    NULL, SE_SHUTDOWN_NAME, &Priv.Privileges [0].Luid);

	// enable the privilege
	if (!AdjustTokenPrivileges (hToken, FALSE, &Priv, 0, NULL, NULL)) {
	    CloseHandle (hToken);
	    break;
        }
       
	ExitWindowsEx (
	    EWX_SHUTDOWN | (bForce ? EWX_FORCE : 0),
	    SHTDN_REASON_FLAG_PLANNED
	    );
	break;

    default:
	break;
    }
}

DWORD WINAPI ACDServiceThreadProc (LPVOID pParam)
{
    HANDLE hDevice = (HANDLE) pParam;

    while (TRUE) {
	DWORD dwBytesRead;
	char report [2];

	if (!ReadFile (hDevice, report, sizeof (report), &dwBytesRead, NULL))
	    continue;

	if (report [1] == 0x0A)
	    ACDDoPowerButtonAction ();
    }

    // should never reach here
    return 0;
}

struct EnumHelper : CACDHidDevice::EnumHelper
{
    ENUMPROC_STATUS Callback (CACDHidDevice* pDevice)
    {
	ENUMPROC_STATUS status = CACDHidDevice::EnumHelper::Callback (pDevice);
	if (status != ENUMPROC_STATUS_SUCCESS)
	    return status;

	DWORD dwThreadID;
	CreateThread (0, 0, ACDServiceThreadProc, m_hDevice, 0, &dwThreadID);

	return ENUMPROC_STATUS_SUCCESS;
    }
};

static BOOL
ACDSetPowerServiceStatus (
    DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    )
{
    SERVICE_STATUS ServiceStatus;

    ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwCurrentState = dwCurrentState;
    ServiceStatus.dwCheckPoint = dwCheckPoint;
    ServiceStatus.dwWaitHint = dwWaitHint;
    ServiceStatus.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
    ServiceStatus.dwWin32ExitCode = dwServiceSpecificExitCode == 0
	? dwWin32ExitCode : ERROR_SERVICE_SPECIFIC_ERROR;
    ServiceStatus.dwControlsAccepted = dwCurrentState == SERVICE_START_PENDING
	? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    return SetServiceStatus (hServiceStatus, &ServiceStatus);
}

void WINAPI
ACDPowerServiceCtrlHandler (DWORD nControlCode)
{
    switch (nControlCode) {	
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
	ACDSetPowerServiceStatus (
	    SERVICE_STOP_PENDING, NO_ERROR, 0, 1, 3000
	    );

	// signal the service main
	SetEvent (hShutdownEvent);

	ACDSetPowerServiceStatus (
	    SERVICE_STOPPED, NO_ERROR, 0, 0, 0
	    );
	return;
    default:
	break;
    }
}

static void WINAPI
ACDPowerServiceMain (DWORD argc, LPTSTR *argv)
{
    hServiceStatus = RegisterServiceCtrlHandler (
	"ACDPowerService", ACDPowerServiceCtrlHandler);

    if (!hServiceStatus)
	return;

    if (!ACDSetPowerServiceStatus (
	SERVICE_START_PENDING, NO_ERROR, 0, 1, 3000
	))
	return;

    hShutdownEvent = CreateEvent (0, TRUE, FALSE, 0);
    if (!hShutdownEvent)
	return;

    if (!ACDSetPowerServiceStatus (
	SERVICE_START_PENDING, NO_ERROR, 0, 2, 1000
	))
	return;

    EnumHelper helper;
    CACDHidDevice::EnumDevices (helper);

    if (!ACDSetPowerServiceStatus (
	SERVICE_RUNNING, NO_ERROR, 0, 0, 0
	))
	return;

    WaitForSingleObject (hShutdownEvent, INFINITE);
    CloseHandle (hShutdownEvent);
}

int
_tmain (int argc, _TCHAR* argv[])
{
    SERVICE_TABLE_ENTRY ServiceTable [] = {
	{ "ACDPowerService", ACDPowerServiceMain },
	{ NULL, NULL }
    };

    BOOL bRet = StartServiceCtrlDispatcher (ServiceTable);
    return bRet ? EXIT_SUCCESS : EXIT_FAILURE;
}
