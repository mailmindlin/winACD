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
#include <atlconv.h>
#include <setupapi.h>
#include <cfgmgr32.h>

extern "C" {
#include <powrprof.h>
#include <hidsdi.h>

typedef void (_stdcall *LPFNDLLFUNC) (HWND, HINSTANCE, LPCSTR, int);
}


// Virtual Controls Class
//
class CACDControl {
private:
    UCHAR m_InitialBrightness;

public:
    CACDHidDevice*  m_hDevice;
    LPCSTR	    m_InstanceId;

    // Default constructor.
    CACDControl ()
    : m_hDevice (NULL), m_InitialBrightness (0), m_InstanceId (NULL)
    {
    }

    ~CACDControl ()
    {
	if (m_hDevice)
	    delete m_hDevice;
	if (m_InstanceId)
	    free ((void*)m_InstanceId);
    }

    // Create a new HidDevice instance from the given handle.
    void Initialize (HANDLE hidDevice, LPCSTR instanceId = NULL)
    {
	if (m_hDevice)
	    delete m_hDevice;

	m_hDevice = new CACDHidDevice (hidDevice);
	assert (m_hDevice != NULL);

	m_InstanceId = instanceId ? strdup (instanceId) : NULL;
	m_InitialBrightness = m_hDevice->GetBrightness ();
    }

    // Return the HidDevice instance.
    CACDHidDevice* operator ->() const
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

// Map of opened monitor controls. TODO: use CArray?
std::map<int, CACDControl> Controls;

// Current device on the property sheet
CACDControl* CurrentControl = NULL;

static void
PspInitControls (HWND hwnd)
{
    USES_CONVERSION;
    assert (CurrentControl != NULL);

    // set the slider current position
    int brightness = (*CurrentControl)->GetBrightness ();

    SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_SLIDER,
	TBM_SETPOS, TRUE, brightness);

    char buffer [256];
    _snprintf (buffer, sizeof (buffer) - 1, "%d", (brightness * 100) / 0xFF);
    SendDlgItemMessage (
	hwnd, IDC_BRIGHTNESS_EDIT, WM_SETTEXT, 0, (LPARAM)buffer);

    // init the static text info
    (*CurrentControl)->GetProductString (buffer, sizeof (buffer));
    SendDlgItemMessage (
	hwnd, IDC_DESCRIPTION, WM_SETTEXT, 0, (LPARAM)W2A((LPCWSTR)buffer));

    _snprintf (buffer, sizeof (buffer) - 1, "%x",
	(*CurrentControl)->GetDeviceVersionNumber ());
    SendDlgItemMessage (
	hwnd, IDC_FIRMWARE_VERSION, WM_SETTEXT, 0, (LPARAM)buffer);

    EDID_STRUCT edid;
    (*CurrentControl)->GetEDID (&edid);

    _snprintf (buffer, sizeof (buffer) - 1, "%d, ISO week %d",
	edid.ProductIdentification.bManufacturingYear + 1990,
	edid.ProductIdentification.bManufacturingWeek);
    SendDlgItemMessage (
	hwnd, IDC_MANUFACTURING_DATE, WM_SETTEXT, 0, (LPARAM)buffer);

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

	CHAR name [128], hardwareId [128];
	sprintf (name, "HID%d: ", i);
	size_t start = strlen (name);

	ULONG length = (ULONG) (sizeof (name) - start);
	if (CM_Get_DevNode_Registry_Property (devInst,
		CM_DRP_DEVICEDESC, NULL, &name [start], &length, 0
		) != CR_SUCCESS) {
	    CloseHandle (hidDevice);
	    continue;
	}

	if (CM_Get_Device_ID (devInst, hardwareId, sizeof (hardwareId), 0
		) != CR_SUCCESS) {
	    CloseHandle (hidDevice);
	    continue;
	}

	SendDlgItemMessage (hwnd, IDC_VIRTUAL_CONTROL,
	    WM_SETTEXT, 0, (LPARAM) &name [start]);

	int index = (int)SendDlgItemMessage (hwnd, IDC_VIRTUAL_CONTROL_LIST, 
	    LB_ADDSTRING, (WPARAM)0, (LPARAM)name);
	if (index != -1) {
	    // NOTE: the CACDControl will close the handle in its
	    // destructor when the DLL is unloaded.
	    Controls [index].Initialize (hidDevice, hardwareId);
	}
	else
	    CloseHandle (hidDevice);

    }
    SetupDiDestroyDeviceInfoList (hDeviceInfo);

    switch (Controls.size ()) {
    case 0:
	// hide everything
	for (
	    HWND child = GetWindow (hwnd, GW_CHILD);
	    child != NULL;
	    child = GetNextWindow (child, GW_HWNDNEXT)
	) ShowWindow (child, SW_HIDE);

	ShowWindow (GetDlgItem (hwnd, IDC_STATIC_VCP), SW_SHOW);
	ShowWindow (GetDlgItem (hwnd, IDC_STATIC_VCP_ICON), SW_SHOW);
	ShowWindow (GetDlgItem (hwnd, IDC_VIRTUAL_CONTROL), SW_SHOW);
	return;

    case 1:
	// only 1 acd, no need for the 'Virtual Controls' list
	ShowWindow (GetDlgItem (hwnd, IDC_VIRTUAL_CONTROL_LIST), SW_HIDE);
	break;

    default:
	ShowWindow (GetDlgItem (hwnd, IDC_VIRTUAL_CONTROL), SW_HIDE);

        // Select the first device in the combo box.
	LRESULT result;
        result = SendDlgItemMessage (hwnd, IDC_VIRTUAL_CONTROL_LIST,
	    LB_SETCURSEL, 0, 0);
	result = SendDlgItemMessage (hwnd, IDC_VIRTUAL_CONTROL_LIST,
	    LB_GETCURSEL, 0, 0);
	assert (result == 0);
	break;
    }
    CurrentControl = &Controls [0];

    // Set the appropriate range for the brightness slider.
    SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_SLIDER,
	TBM_SETRANGEMIN, TRUE, 0x00);                                                 
    SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_SLIDER,
	TBM_SETRANGEMAX, 0, 0xFF);

    // Init the controls for the current device (AcdDevice)
    PspInitControls (hwnd);
}

static void
OptionsOnInitDialog (HWND hwnd)
{
    SendDlgItemMessage (hwnd, IDC_POWER_ACTION_COMBO,
	CB_ADDSTRING, 0, (LPARAM) "Do nothing");

    if (IsPwrSuspendAllowed ()) {
        SendDlgItemMessage (hwnd, IDC_POWER_ACTION_COMBO,
	    CB_ADDSTRING, 0, (LPARAM) "Stand by");
    }
    if (IsPwrHibernateAllowed ()) {
	SendDlgItemMessage (hwnd, IDC_POWER_ACTION_COMBO,
	    CB_ADDSTRING, 0, (LPARAM) "Hibernate");
    }
    if (IsPwrShutdownAllowed ()) {
	SendDlgItemMessage (hwnd, IDC_POWER_ACTION_COMBO,
	    CB_ADDSTRING, 0, (LPARAM) "Shut down");
    }

    // Select the action in the combobox
    SendDlgItemMessage (hwnd, IDC_POWER_ACTION_COMBO,
	CB_SETCURSEL, 0, 0);

    SendDlgItemMessage (hwnd, IDC_HOTKEY_INCREASE, HKM_SETHOTKEY,
	MAKEWORD (VK_ADD, (HOTKEYF_CONTROL | HOTKEYF_ALT)), 0);
    SendDlgItemMessage (hwnd, IDC_HOTKEY_DECREASE, HKM_SETHOTKEY,
	MAKEWORD (VK_SUBTRACT, (HOTKEYF_CONTROL | HOTKEYF_ALT)), 0);
}

static void
OptionsOnDisableClicked (HWND hwnd)
{
    BOOL checked = (BOOL) SendDlgItemMessage (
	hwnd, IDC_DISABLE_CHECK, BM_GETCHECK, 0, 0);

    EnableWindow (GetDlgItem (hwnd, IDC_POWER_ACTION_COMBO), !checked);
}

static BOOL CALLBACK
OptionsDlgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
	OptionsOnInitDialog (hwnd);
	return TRUE;

    case WM_COMMAND: 
	switch (LOWORD (wParam)) {
	case IDC_DISABLE_CHECK: 
	    if (HIWORD (wParam) == BN_CLICKED) {
		OptionsOnDisableClicked (hwnd);
		return TRUE;
	    }
	    break;

	case IDOK: 
	    // Fall through. 
	case IDCANCEL: 
	    EndDialog (hwnd, wParam); 
	    return TRUE; 
	} 
    } 
    return FALSE;
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
	    if (CurrentControl)
		PspInitControls (hwnd);
	    return TRUE;
	case PSN_APPLY:
	    // apply the current settings to all the open monitor controls.
	    for (it = Controls.begin (); it != Controls.end (); ++it)
		it->second.Apply ();	    
	    if (CurrentControl)
		PspInitControls (hwnd);
	    return TRUE;
	default :
	    break;
	}
	break;
    }

    case WM_COMMAND:
	switch (LOWORD (wParam)) {
	case IDC_VIRTUAL_CONTROL_LIST:
	    if (HIWORD (wParam) == LBN_SELCHANGE) {
		int i = (int) SendDlgItemMessage (
		    hwnd, IDC_VIRTUAL_CONTROL_LIST, LB_GETCURSEL, 0, 0);
		CurrentControl = &Controls [i];
		PspInitControls (hwnd);
		return TRUE;
	    }
	case IDC_PROPERTIES_BUTTON: {
	    if (HIWORD (wParam) != BN_CLICKED)
		return FALSE;

	    HINSTANCE hDLL = LoadLibrary (_TEXT ("DEVMGR.DLL"));
	    if (hDLL == NULL)
		return FALSE;

	    LPFNDLLFUNC DeviceProperties = (LPFNDLLFUNC) GetProcAddress(
		hDLL, "DevicePropertiesA");

	    if (DeviceProperties)
		DeviceProperties (hwnd, 0, CurrentControl->m_InstanceId, 0);

	    FreeLibrary (hDLL);
	    return TRUE;
	}
	case IDC_OPTIONS_BUTTON:
	    if (HIWORD (wParam) == BN_CLICKED) {
		DialogBox (
		    _AtlBaseModule.GetResourceInstance (),
		    MAKEINTRESOURCE (IDD_ACD_OPTIONS), hwnd,
		    OptionsDlgProc
		    );
		return TRUE;
	    }
	    return FALSE;

	case IDC_BRIGHTNESS_EDIT:
	    if (HIWORD (wParam) == EN_CHANGE) {

		if (GetFocus () != GetDlgItem (hwnd, IDC_BRIGHTNESS_EDIT))
		    break;

		char buffer [256];
		SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_EDIT,
		    WM_GETTEXT, sizeof (buffer), (LPARAM)buffer);

		int val = atoi (buffer);
		if (val < 0 || val > 100) {
		    val = val < 0 ? 0 : (val > 100 ? 100 : val);
		    _snprintf (buffer, sizeof (buffer) - 1, "%d", val);
		    SendDlgItemMessage (hwnd, IDC_BRIGHTNESS_EDIT,
			WM_SETTEXT, 0, (LPARAM)buffer);
		}
		int brightness = val * 255 / 100;
		// find the next best guess for brightness...
		while ((brightness * 100 / 255) != val) ++brightness;

		SendDlgItemMessage (
		    hwnd, IDC_BRIGHTNESS_SLIDER, TBM_SETPOS, TRUE, brightness);

		assert (CurrentControl != NULL);
		(*CurrentControl)->SetBrightness (brightness);

		/* notify the PSP that the settings have changed */
		SendMessage (GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0L);
		return TRUE;
	    }
	    return FALSE;

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

	    assert (CurrentControl != NULL);
	    (*CurrentControl)->SetBrightness (brightness);

	    char buffer [16];
	    _snprintf (buffer, sizeof (buffer) - 1, "%d", brightness * 100 / 255);
	    SendDlgItemMessage (
		hwnd, IDC_BRIGHTNESS_EDIT, WM_SETTEXT, 0, (LPARAM)buffer);

	    /* notify the PSP that the settings have changed */
	    SendMessage (GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0L);

	    return TRUE;
	} 

	default:
	    break;
	}
	break;

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