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

#include <assert.h>

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

    return TRUE;
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

UINT
CACDMonApp::GetBrightness ()
{
    if (m_DeviceArray.GetCount () == 0)
	return 0;

    return m_DeviceArray.ElementAt (0)->GetBrightness () / 16;
}

void
CACDMonApp::SetBrightness (UINT nBrightness)
{
    nBrightness = min (nBrightness, 15);
    nBrightness = nBrightness * 16 + nBrightness;

    for (INT_PTR i = 0; i <  m_DeviceArray.GetCount (); ++i)
	m_DeviceArray.ElementAt (i)->SetBrightness (nBrightness);
}