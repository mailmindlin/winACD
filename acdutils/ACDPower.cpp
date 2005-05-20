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
#include <dbt.h>

#include "ACDUtil.h"

#include <stdlib.h>
#include <iostream>
#include <tchar.h>
#include <atlconv.h>

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
    CACDHidDevice* pDevice = (CACDHidDevice*) pParam;

    UCHAR bFlags, bMask;
    ACDUtil::GetFlagsFromPrefs (bFlags, bMask);
    pDevice->SetFlags (pDevice->GetFlags () & ~bMask | bFlags);

    HANDLE hDevice = pDevice->GetHandle ();
    while (TRUE) {
	DWORD dwBytesRead;
	char report [2];

	if (!ReadFile (hDevice, report, sizeof (report), &dwBytesRead, NULL))
	    break;

	if (report [1] == 0x0A)
	    ACDDoPowerButtonAction ();
    }

    delete pDevice;
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
	CreateThread (0, 0, ACDServiceThreadProc, pDevice, 0, &dwThreadID);

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

static void
ACDHandleDeviceArrival (PDEV_BROADCAST_HDR pdb)
{
    USES_CONVERSION;

    if (pdb->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
	return;

    PDEV_BROADCAST_DEVICEINTERFACE pdbi =
	(PDEV_BROADCAST_DEVICEINTERFACE) pdb;

    HANDLE hDevice = CreateFile (
	W2CA ((LPCWSTR)pdbi->dbcc_name),    /* filename */
	GENERIC_READ | GENERIC_WRITE,	    /* desired access */
	FILE_SHARE_READ | FILE_SHARE_WRITE, /* share mode */
	NULL,				    /* security attributes */
	OPEN_EXISTING,			    /* creation disposition */
	0,				    /* flags and attributes */
	NULL				    /* template file */
	);

    if (hDevice == INVALID_HANDLE_VALUE)
	return;

    //
    // Check the collection USAGE_PAGE (must be monitor)
    // 

    PHIDP_PREPARSED_DATA ppData;
    if (!HidD_GetPreparsedData (hDevice, &ppData)) {
        CloseHandle (hDevice);
        return;
    }

    HIDP_CAPS caps;
    NTSTATUS hidStatus = HidP_GetCaps (ppData, &caps);
    if (hidStatus != HIDP_STATUS_SUCCESS
	|| caps.UsagePage != CUSBMonitorHidDevice::USAGE_PAGE_MONITOR) {
        HidD_FreePreparsedData (ppData);
        CloseHandle (hDevice);
        return;
    }

    CACDHidDevice *pDevice = new CACDHidDevice (hDevice, ppData);
    if (pDevice == NULL) {
        HidD_FreePreparsedData (ppData);
        CloseHandle (hDevice);
	return;
    }

    if (!pDevice->IsSupportedCinemaDisplay ()) {
	delete pDevice;
	return;
    }

    DWORD dwThreadID;
    CreateThread (0, 0, ACDServiceThreadProc, pDevice, 0, &dwThreadID);
}

DWORD WINAPI
ACDPowerServiceCtrlHandler (DWORD dwControl, DWORD dwEventType,
			    LPVOID lpEventData, LPVOID lpContext)
{
    switch (dwControl) {	
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
	return NO_ERROR;

    case SERVICE_CONTROL_DEVICEEVENT:
	if (dwEventType == DBT_DEVICEARRIVAL)
	    ACDHandleDeviceArrival ((PDEV_BROADCAST_HDR) lpEventData);
	return NO_ERROR;

    default:
	return ERROR_CALL_NOT_IMPLEMENTED;
    }
}

static void WINAPI
ACDPowerServiceMain (DWORD argc, LPTSTR *argv)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    hServiceStatus = RegisterServiceCtrlHandlerEx (
	"ACDPowerService", ACDPowerServiceCtrlHandler, NULL);

    if (!hServiceStatus)
	return;

    if (!ACDSetPowerServiceStatus (
	SERVICE_START_PENDING, NO_ERROR, 0, 1, 3000
	))
	return;

    ZeroMemory (&NotificationFilter, sizeof (NotificationFilter));
    NotificationFilter.dbcc_size = sizeof (DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    HidD_GetHidGuid (&NotificationFilter.dbcc_classguid);

    if (!RegisterDeviceNotification (
	hServiceStatus, &NotificationFilter, DEVICE_NOTIFY_SERVICE_HANDLE
	))
	return;

    if (!ACDSetPowerServiceStatus (
	SERVICE_START_PENDING, NO_ERROR, 0, 2, 3000
	))
	return;

    hShutdownEvent = CreateEvent (0, TRUE, FALSE, 0);
    if (!hShutdownEvent)
	return;

    if (!ACDSetPowerServiceStatus (
	SERVICE_START_PENDING, NO_ERROR, 0, 3, 3000
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
