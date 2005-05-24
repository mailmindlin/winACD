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

#include "stdafx.h"
#include "ACDMon.h"
#include "ACDBrightnessWnd.h"

#ifdef _DEBUG
# define new DEBUG_NEW
#endif

#include <dbt.h>

// CACDMonApp

BEGIN_MESSAGE_MAP(CACDMonApp, CWinApp)
END_MESSAGE_MAP()


/** CACDMonApp construction. */
CACDMonApp::CACDMonApp ()
{
    // enumerate the ACD controls.
    EnumHelper helper;
    CACDHidDevice::EnumDevices (helper);
}

/** CACDMonApp destructor */
CACDMonApp::~CACDMonApp ()
{
    for (INT_PTR i = 0; i <  m_DeviceArray.GetCount (); ++i)
	delete m_DeviceArray.ElementAt (i);
}

/** The one and only CACDMonApp object. */
CACDMonApp theApp;

/**
 * CACDMonApp initialization.
 */
BOOL
CACDMonApp::InitInstance ()
{
    // required on WinXP with the use of ComCtl32.dll version 6 or later.
    InitCommonControls ();

    // application initialization (we might not need the registry stuff).
    CWinApp::InitInstance ();
    SetRegistryKey (_T ("WinACD"));

    // create the background window.
    CACDBrightnessWnd* pBackgroundWnd = new CACDBrightnessWnd (
	CACDBrightnessWnd::ACD_BRIGHTNESS_WND_BACKGROUND);
    pBackgroundWnd->Create (NULL, NULL);

    // create the foreground window.
    CACDBrightnessWnd* pForegroundWnd = new CACDBrightnessWnd (
	CACDBrightnessWnd::ACD_BRIGHTNESS_WND_FOREGROUND,
	pBackgroundWnd
	);
    pForegroundWnd->Create (NULL, NULL);
    pForegroundWnd->SetOpacity (0);
    pForegroundWnd->ShowWindow (SW_HIDE);
    pForegroundWnd->UpdateWindow ();

    // the foreground window will be the one receiving events.
    m_pMainWnd = pForegroundWnd;

    // create & start the RegNotify thread.
    AfxBeginThread (RegNotifyThreadMain, 0);

    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory (&NotificationFilter, sizeof (NotificationFilter));
    NotificationFilter.dbcc_size = sizeof (DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    HidD_GetHidGuid (&NotificationFilter.dbcc_classguid);
    m_hDevNotify = RegisterDeviceNotification (
	pForegroundWnd->m_hWnd,
	&NotificationFilter,
	DEVICE_NOTIFY_WINDOW_HANDLE
	);

    return TRUE;
}

int
CACDMonApp::ExitInstance ()
{
    UnregisterDeviceNotification (m_hDevNotify);
    return CWinApp::ExitInstance ();
}


UINT
CACDMonApp::RegNotifyThreadMain (LPVOID pParam)
{
    HKEY hKey;
 
    if (RegCreateKeyEx (
	    HKEY_CURRENT_USER, "SOFTWARE\\WinACD\\Preferences", 0, 0,
	    REG_OPTION_NON_VOLATILE, KEY_NOTIFY, 0, &hKey, 0
	    ) != ERROR_SUCCESS)
	return -1;

    while (TRUE) {
	theApp.m_pMainWnd->SendMessage (ACD_WM_INIT_HOTKEYS, 0, 0);

	if (RegNotifyChangeKeyValue (hKey, FALSE,
		REG_NOTIFY_CHANGE_LAST_SET, 0, FALSE
		) != ERROR_SUCCESS)
	    break;
    }

    RegCloseKey (hKey);

    return 0;
}

CACDMonApp::EnumHelper::ENUMPROC_STATUS
CACDMonApp::EnumHelper::Callback (CACDHidDevice* pDevice)
{
    ENUMPROC_STATUS status = CACDHidDevice::EnumHelper::Callback (pDevice);

    if (status != ENUMPROC_STATUS_SUCCESS)
	return status;

    theApp.m_DeviceArray.Add (pDevice);
    return ENUMPROC_STATUS_SUCCESS;
}

BOOL
CACDMonApp::GetBrightness (PUCHAR pbBrightness)
{
    if (m_DeviceArray.GetCount () == 0)
	return FALSE;

    BOOL bRet = m_DeviceArray.ElementAt (0)->GetBrightness (pbBrightness);
    if (bRet)
	*pbBrightness /= 16;

    return bRet;
}

BOOL
CACDMonApp::SetBrightness (UCHAR nBrightness)
{
    nBrightness = min (nBrightness, 15);
    nBrightness = nBrightness * 16 + nBrightness;

    BOOL bRet = TRUE;
    for (INT_PTR i = 0; i <  m_DeviceArray.GetCount (); ++i)
	bRet &= m_DeviceArray.ElementAt (i)->SetBrightness (nBrightness);

    return bRet;
}

BOOL
CACDMonApp::OnDeviceChange (UINT nEventType, LPTSTR lpcDeviceName)
{
    if (nEventType == DBT_DEVICEREMOVECOMPLETE
	|| nEventType == DBT_DEVICEARRIVAL) {
	// at some point we might check if we actually have an open
	// handle for the disconnected device. for now, we just purge
	// all the HID device in the DeviceArray and reload them all.

	INT_PTR iCount = m_DeviceArray.GetCount ();
	for (INT_PTR i = 0; i <  iCount; ++i)
	    delete m_DeviceArray.ElementAt (i);

	m_DeviceArray.RemoveAll ();

	EnumHelper helper;
	CACDHidDevice::EnumDevices (helper);

	return iCount != m_DeviceArray.GetCount ();
    }

    return FALSE;
}
