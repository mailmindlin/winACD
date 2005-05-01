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

// ACDShlExt.cpp : Implementation of CACDShlExt

#include "stdafx.h"
#include "ACDShlExt.h"
#include "ACDHidDev.h"

#include <map>

// CACDShlExt

#include <assert.h>
#include <setupapi.h>
#include <cfgmgr32.h>

extern "C" {
#include <hidsdi.h>
}


// Virtual Controls Class
//
class CACDControl {
private:
    UCHAR m_InitialBrightness;

    CACDHidDevice* m_hDevice; //!< The ACD hid device

public:
    // Default constructor.
    CACDControl () : m_hDevice (NULL), m_InitialBrightness (0) { }

    ~CACDControl ()
    {
	if (m_hDevice)
	    delete m_hDevice;
    }

    // Create a new HidDevice instance from the given handle.
    void Initialize (HANDLE hidDevice)
    {
	if (m_hDevice)
	    delete m_hDevice;

	m_hDevice = new CACDHidDevice (hidDevice);
	assert (m_hDevice != NULL);

	m_InitialBrightness = m_hDevice->GetBrightness ();
    }

    // Return the HidDevice instance.
    operator CACDHidDevice* const () 
    {
	assert (m_hDevice);
	return m_hDevice;
    }

    // Apply the current settings
    void Apply (void)
    {
	assert (m_hDevice != NULL);
	m_InitialBrightness = m_hDevice->GetBrightness ();
    }

    // Reset to the initial settings
    void Reset (void) const
    {
	assert (m_hDevice != NULL);
	m_hDevice->SetBrightness (m_InitialBrightness);
    }
};

// Map of opened monitor controls.
std::map<int, CACDControl> Controls;

// Current device on the property sheet
CACDHidDevice* CurrentDevice = NULL;

static void
PspInitControls (HWND hwnd)
{
    assert (CurrentDevice != NULL);

    // set the slider current position
    int brightness = CurrentDevice->GetBrightness ();

    SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_SLIDER,
	TBM_SETPOS, TRUE, brightness);

    char buffer [16];
    _snprintf (buffer, sizeof (buffer) - 1, "%d", (brightness * 100) / 0xFF);
    SendDlgItemMessage (
	hwnd, IDC_BRIGHTNESS_EDIT, WM_SETTEXT, 0, (LPARAM)buffer);

    // TODO: init the info and options.
}

static void
PspOnInitDialog (HWND hwnd)
{
    // Uncomment this if we can't get the themes to work
    // properly with the manifest resource.
    // ::EnableThemeDialogTexture (hwnd, ETDT_ENABLETAB); 

    // get the HIDClass guid.
    GUID hidGuid;
    HidD_GetHidGuid (&hidGuid);

    // get the hid PnP node.
    HDEVINFO hDeviceInfo = SetupDiGetClassDevs (&hidGuid,
	NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    for (DWORD i = 0;; ++i) {
	SP_DEVICE_INTERFACE_DATA deviceInterface;

	deviceInterface.cbSize = sizeof (deviceInterface);
	if (!SetupDiEnumDeviceInterfaces (hDeviceInfo, NULL, &hidGuid,
	    i, &deviceInterface))
	    break;

	// Get the interface detail data.
	DWORD idSize;
	SetupDiGetDeviceInterfaceDetail (hDeviceInfo, &deviceInterface,
	    NULL, 0, &idSize, NULL);

	PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetail;
	pInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (idSize);
	if (pInterfaceDetail == NULL)
	    continue;

	pInterfaceDetail->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
	if (!SetupDiGetDeviceInterfaceDetail (hDeviceInfo, &deviceInterface,
	    pInterfaceDetail, idSize, NULL, NULL)) {
	    free (pInterfaceDetail);
	    continue;
	}

	// Open the driver.
	HANDLE hidDevice = CreateFile (pInterfaceDetail->DevicePath,
	    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
	    NULL, OPEN_EXISTING, 0, NULL);

	free (pInterfaceDetail);

	if (hidDevice == INVALID_HANDLE_VALUE)
	    continue;

	if (!CACDHidDevice::IsACDHidDevice (hidDevice)) {
	    CloseHandle (hidDevice);
	    continue;
	}

	// Get the device's name
	SP_DEVINFO_DATA	devInfo;
	devInfo.cbSize = sizeof (devInfo);

	if (!SetupDiEnumDeviceInfo (hDeviceInfo, i, &devInfo)) {
	    CloseHandle (hidDevice);
	    continue;
	}

	DEVINST devInst;
	CM_Get_Parent (&devInst, devInfo.DevInst, 0);

	CHAR name [128];
	sprintf (name, "HID%d: ", i);
	size_t start = strlen (name);

	ULONG length = (ULONG) (sizeof (name) - start);
	if (CM_Get_DevNode_Registry_Property (devInst,
		CM_DRP_DEVICEDESC, NULL, &name [start], &length, 0
		) != CR_SUCCESS) {
	    CloseHandle (hidDevice);
	    continue;
	}

	int index = (int)SendDlgItemMessage (hwnd, IDC_AVAILABLE_MONITORS, 
	    CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
	if (index != -1) {
	    // NOTE: the CACDControl will close the handle in its
	    // destructor when the DLL is unloaded.
	    Controls [index].Initialize (hidDevice);
	}
	else
	    CloseHandle (hidDevice);

    }
    SetupDiDestroyDeviceInfoList (hDeviceInfo);

    switch (Controls.size ()) {
    case 0:
	/* device not found, disable UI */
	EnableWindow (GetDlgItem (hwnd, IDC_BRIGHTNESS_SLIDER), FALSE);
	EnableWindow (GetDlgItem (hwnd, IDC_STATIC_0), FALSE);
	EnableWindow (GetDlgItem (hwnd, IDC_STATIC_100), FALSE);
	EnableWindow (GetDlgItem (hwnd, IDC_STATIC_BRIGHTNESS), FALSE);
	EnableWindow (GetDlgItem (hwnd, IDC_CONTROLS_GROUP), FALSE);
	ShowWindow (GetDlgItem (hwnd, IDC_STATIC_MONITORS), SW_HIDE);
	ShowWindow (GetDlgItem (hwnd, IDC_AVAILABLE_MONITORS), SW_HIDE);
	return;

    case 1:
	/* only 1 acd, no need for 'Monitors' combo-box */
	ShowWindow (GetDlgItem (hwnd, IDC_STATIC_MONITORS), SW_HIDE);
	ShowWindow (GetDlgItem (hwnd, IDC_AVAILABLE_MONITORS), SW_HIDE);
	break;

    default:
        // Select the first device in the combo box.
	LRESULT result;
        result = SendDlgItemMessage (hwnd, IDC_AVAILABLE_MONITORS,
	    CB_SETCURSEL, 0, 0);
	result = SendDlgItemMessage (hwnd, IDC_AVAILABLE_MONITORS,
	    CB_GETCURSEL, 0, 0);
	assert (result == 0);
	break;
    }
    CurrentDevice = Controls [0];

    // Set the appropriate range for the brightness slider.
    SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_SLIDER,
	TBM_SETRANGEMIN, TRUE, 0x00);                                                 
    SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_SLIDER,
	TBM_SETRANGEMAX, 0, 0xFF);

    // Init the controls for the current device (AcdDevice)
    PspInitControls (hwnd);
}

static BOOL CALLBACK
PspDlgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::map<int, CACDControl>::iterator it;

    switch (uMsg) {
    case WM_INITDIALOG:
	PspOnInitDialog (hwnd);
	return TRUE;

    case WM_NOTIFY: {
	NMHDR FAR *lpnm = (NMHDR FAR *)lParam;
	switch (lpnm->code) {
        case PSN_RESET:
	    // reset all monitor controls to their initial values.
	    for (it = Controls.begin (); it != Controls.end (); ++it)
		it->second.Reset ();	    
	    if (CurrentDevice)
		PspInitControls (hwnd);
	    return TRUE;
	case PSN_APPLY:
	    // apply the current settings to all the open monitor controls.
	    for (it = Controls.begin (); it != Controls.end (); ++it)
		it->second.Apply ();	    
	    if (CurrentDevice)
		PspInitControls (hwnd);
	    return TRUE;
	default :
	    break;
	}
	break;
    }

    case WM_COMMAND:
	switch (LOWORD (wParam)) {
	case IDC_AVAILABLE_MONITORS:
	    if (HIWORD (wParam) == CBN_SELCHANGE) {
		int i = (int) SendDlgItemMessage (
		    hwnd, IDC_AVAILABLE_MONITORS, CB_GETCURSEL, 0, 0);
		CurrentDevice = Controls [i];
		PspInitControls (hwnd);
		return TRUE;
	    }
	default:
	    break;
	}
	break;

    case WM_HSCROLL:
	if (GetWindowLong ((HWND)lParam, GWL_ID) != IDC_BRIGHTNESS_SLIDER)
	    break;

	switch (LOWORD (wParam)) {
	case TB_PAGEDOWN:
	case TB_LINEDOWN:
	case TB_PAGEUP:
	case TB_LINEUP:
	case TB_TOP:
	case TB_BOTTOM: 
	    wParam = MAKEWPARAM (0, SendDlgItemMessage (
		hwnd, IDC_BRIGHTNESS_SLIDER, TBM_GETPOS, 0, 0));
	    /* fall-through */
	case TB_THUMBPOSITION :
	case TB_THUMBTRACK : {
	    int brightness = (int) HIWORD (wParam);

	    assert (CurrentDevice != NULL);
	    CurrentDevice->SetBrightness (brightness);

	    char buffer [16];
	    _snprintf (buffer, sizeof (buffer) - 1, "%d", (brightness * 100) / 0xFF);
	    SendDlgItemMessage (
		hwnd, IDC_BRIGHTNESS_EDIT, WM_SETTEXT, 0, (LPARAM)buffer);

	    /* notify the PSP that the settings have changed */
	    SendMessage (GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0L);

	    return TRUE;
	} 

	default:
	    break;
	}

    default:
	break;
    }

    return FALSE;
}

UINT CALLBACK PspCallbackProc (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    if (PSPCB_RELEASE == uMsg) {
	/* we are unloading */
    }

    return 1;                           // used for PSPCB_CREATE - let the page be created
}


//
// IShellExtInit interface:
//

HRESULT
CACDShlExt::Initialize (
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
    return S_OK;
}

//
//IShellPropSheetExt interface:
//

HRESULT
CACDShlExt::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPageProc, LPARAM lParam)
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    if (lpfnAddPageProc == NULL) {
	return E_INVALIDARG;
    }

    psp.dwSize      = sizeof (psp);
    psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE
	| PSP_USEICONID | PSP_USECALLBACK;
    psp.hInstance   = _AtlBaseModule.GetResourceInstance ();
    psp.pszTemplate = MAKEINTRESOURCE (IDD_ACD_PROPPAGE);
    psp.pszIcon     = MAKEINTRESOURCE (IDI_ICON);
    psp.pszTitle    = "Controls";
    psp.pfnDlgProc  = PspDlgProc;
    psp.pfnCallback = PspCallbackProc;
    /* TODO: how do we add RefCount ??? */

    hpsp = CreatePropertySheetPage (&psp);
    if (hpsp == NULL) {
	return E_FAIL;
    }

    if (!lpfnAddPageProc (hpsp, lParam)) {
	DestroyPropertySheetPage (hpsp);
	return E_FAIL;
    }

    return S_OK;
}