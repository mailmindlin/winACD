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

// ACDPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "ACDExt.h"
#include "ACDPropertyPage.h"
#include "ACDOptionsDlg.h"
#include "ACDUtil.h"

#include <dbt.h>

// CACDPropertyPage dialog

IMPLEMENT_DYNAMIC (CACDPropertyPage, CPropertyPage)
CACDPropertyPage::CACDPropertyPage ()
    : CPropertyPage (CACDPropertyPage::IDD), m_pCurrentVirtualCP (NULL), m_hDevNotify(NULL)
{
    m_psp.dwFlags |= PSP_USEICONID | PSP_USEREFPARENT;
    m_psp.pszIcon = MAKEINTRESOURCE (IDI_ICON);
    m_psp.pcRefParent = (UINT*) &(AfxGetModuleState ()->m_nObjectCount);

    CACDVirtualCP::FindKnownVirtualCPs (m_VirtualControlPanels);
}

CACDPropertyPage::~CACDPropertyPage ()
{
    for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i)
	delete m_VirtualControlPanels.ElementAt (i);
}

void
CACDPropertyPage::PostNcDestroy ()
{
    CPropertyPage::PostNcDestroy ();

    UnregisterDeviceNotification (m_hDevNotify);

    delete this;
}

void
CACDPropertyPage::UpdateBrightnessEdit ()
{
    char buffer [4];

    // get the slider position.
    int pos = m_cBrightnessSlider.GetPos ();

    // set the spinner text.
    _snprintf (buffer, sizeof (buffer), "%d", pos * 100 / 255);
    buffer [sizeof (buffer) - 1] = '\0';
    m_cBrightnessEdit.SetWindowText (buffer);
}

void
CACDPropertyPage::UpdateControls ()
{
    CACDHidDevice& Device = m_pCurrentVirtualCP->GetDevice ();
    USES_CONVERSION;
    char buffer [128];

    // set the slider position.
    int pos = m_pCurrentVirtualCP->GetBrightness ();
    m_cBrightnessSlider.SetPos (pos);

    // set the spinner text.
    UpdateBrightnessEdit ();

    // set the device description.
    if (!Device.GetProductString (buffer, sizeof (buffer))) 
	GetDlgItem (IDC_DESCRIPTION)->SetWindowText ("(Not Available)");
    else
	GetDlgItem (IDC_DESCRIPTION)->SetWindowText (W2T ((LPCWSTR)buffer));

    // set the firmware revision.
    int nVersion = Device.GetDeviceVersionNumber ();
    _snprintf (buffer, sizeof (buffer), "%x", nVersion);
    buffer [sizeof (buffer) - 1] = '\0';
    GetDlgItem (IDC_FIRMWARE_VERSION)->SetWindowText (buffer);

    // set the manufacturing date.
    EDID_STRUCT edid;
    Device.GetEDID (&edid);
    _snprintf (buffer, sizeof (buffer), "%d, ISO week %d",
	edid.ProductIdentification.bManufacturingYear + 1990,
	edid.ProductIdentification.bManufacturingWeek);
    buffer [sizeof (buffer) - 1] = '\0';
    GetDlgItem (IDC_MANUFACTURING_DATE)->SetWindowText (buffer);
}

void
CACDPropertyPage::InitializeControls ()
{
    BOOL bShow = m_VirtualControlPanels.GetCount () != 0;

    for (CWnd* child = GetWindow (GW_CHILD);
	child != NULL;
        child = child->GetNextWindow ()
	)
	child->ShowWindow (bShow);

    switch (m_VirtualControlPanels.GetCount ()) {
    case 0:
	// no ACD found
	GetDlgItem (IDC_VIRTUAL_CONTROL)->SetWindowText ("None present");
	GetDlgItem (IDC_STATIC_VCP)->ShowWindow (SW_SHOW);
	GetDlgItem (IDC_STATIC_VCP_ICON)->ShowWindow (SW_SHOW);
	GetDlgItem (IDC_VIRTUAL_CONTROL)->ShowWindow (SW_SHOW);
	return;

    case 1:
	// one ACD connected
	m_pCurrentVirtualCP = m_VirtualControlPanels.ElementAt (0);
	m_cVirtualControlList.ShowWindow (SW_HIDE);
	GetDlgItem (IDC_VIRTUAL_CONTROL)->SetWindowText (
	    m_pCurrentVirtualCP->GetDeviceName ());
	break;

    default: 
	// multiple ACDs connected
	for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i) {
	    CACDVirtualCP* pVCP = m_VirtualControlPanels.ElementAt (i);

	    int pos = m_cVirtualControlList.AddString (pVCP->GetDeviceName ());
	    if (pos >= 0)
		m_cVirtualControlList.SetItemDataPtr (pos, pVCP);
	}

	// select the first item in the list.
	m_pCurrentVirtualCP = (CACDVirtualCP*)
	    m_cVirtualControlList.GetItemDataPtr (0);
	m_cVirtualControlList.SetCurSel (0);
	GetDlgItem (IDC_VIRTUAL_CONTROL)->ShowWindow (SW_HIDE);
	break;
    }

    UpdateControls ();
}

BOOL
CACDPropertyPage::OnInitDialog ()
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    CPropertyPage::OnInitDialog ();

    // register the HID device notification.
    ZeroMemory (&NotificationFilter, sizeof (NotificationFilter));
    NotificationFilter.dbcc_size = sizeof (DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    HidD_GetHidGuid (&NotificationFilter.dbcc_classguid);
    m_hDevNotify = RegisterDeviceNotification (
	m_hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

    // initialize the brightness slider range.
    m_cBrightnessSlider.SetRange (0, 255);
    // we only need to type 0-100, limit edit text to 3 chars.
    m_cBrightnessEdit.SetLimitText (3);

    InitializeControls ();

    if (m_VirtualControlPanels.GetCount () > 0)
	SetTimer (ACD_SETTINGS_TIMER, 500, 0);

    return TRUE;
}

BOOL
CACDPropertyPage::OnApply ()
{
    BOOL status = CPropertyPage::OnApply ();
    if (!status)
	return status;

    // enumerate the virtual control panels and apply individual settings.
    for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i)
	m_VirtualControlPanels.ElementAt (i)->Apply ();

    return TRUE;
}

void
CACDPropertyPage::OnReset ()
{
    CPropertyPage::OnReset ();

    // reset settings on all virtual control panels.
    for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i)
	m_VirtualControlPanels.ElementAt (i)->Reset ();
}

void
CACDPropertyPage::SetModified (BOOL bChanged)
{
    if (bChanged)
	CPropertyPage::SetModified (TRUE);
    else {
	// if any of the VCPs have modified settings, we can't clear the
	// modified flags.
	for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i)
	    bChanged |= m_VirtualControlPanels.ElementAt (i)->Modified ();

	CPropertyPage::SetModified (bChanged);
    }
}

void
CACDPropertyPage::DoDataExchange (CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange (pDX);
    DDX_Control (pDX, IDC_BRIGHTNESS_SLIDER, m_cBrightnessSlider);
    DDX_Control (pDX, IDC_BRIGHTNESS_EDIT, m_cBrightnessEdit);
    DDX_Control (pDX, IDC_VIRTUAL_CONTROL_LIST, m_cVirtualControlList);
}

BEGIN_MESSAGE_MAP (CACDPropertyPage, CPropertyPage)
    ON_WM_DEVICECHANGE ()
    ON_WM_HSCROLL ()
    ON_WM_TIMER ()
    ON_BN_CLICKED (IDC_PROPERTIES_BUTTON, OnBnClickedPropertiesButton)
    ON_BN_CLICKED (IDC_OPTIONS_BUTTON, OnBnClickedOptionsButton)
    ON_LBN_SELCHANGE (IDC_VIRTUAL_CONTROL_LIST, OnLbnSelchangeVirtualControlList)
    ON_EN_CHANGE (IDC_BRIGHTNESS_EDIT, OnEnChangeBrightnessEdit)
END_MESSAGE_MAP ()

//
// CACDPropertyPage message handlers
//

void CACDPropertyPage::OnLbnSelchangeVirtualControlList()
{
    int index = m_cVirtualControlList.GetCurSel ();
    m_pCurrentVirtualCP = (CACDVirtualCP*)
	m_cVirtualControlList.GetItemDataPtr (index);

    UpdateControls ();
}

void
CACDPropertyPage::OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if ((CSliderCtrl*) pScrollBar == &m_cBrightnessSlider) {
	int pos = m_cBrightnessSlider.GetPos ();
	m_pCurrentVirtualCP->SetBrightness (pos);
        UpdateBrightnessEdit ();
	SetModified (TRUE);
    }
    else
	CPropertyPage::OnHScroll (nSBCode, nPos, pScrollBar);
}

void
CACDPropertyPage::OnTimer (UINT nIDEvent) 
{
    if (nIDEvent == ACD_SETTINGS_TIMER) {
	if (!m_pCurrentVirtualCP->CheckBrightness ()) {
	    SetModified (FALSE);
	    if (m_pCurrentVirtualCP->Reset ())
		UpdateControls ();
	}
    }
    else
	CPropertyPage::OnTimer (nIDEvent);
}

void
CACDPropertyPage::OnBnClickedPropertiesButton ()
{
    typedef void (_stdcall *LPFNDLLFUNC) (HWND, HINSTANCE, LPCSTR, int);

    HINSTANCE hDLL = LoadLibrary (_TEXT ("DEVMGR.DLL"));
    if (hDLL == NULL)
	return;

    LPFNDLLFUNC DeviceProperties = (LPFNDLLFUNC) GetProcAddress(
	hDLL, "DevicePropertiesA");

    if (DeviceProperties)
	DeviceProperties (m_hWnd, 0, m_pCurrentVirtualCP->GetDeviceID (), 0);

    FreeLibrary (hDLL);
}

void
CACDPropertyPage::OnBnClickedOptionsButton ()
{
    CACDOptionsDialog dlg (m_VirtualControlPanels, this);
    if (dlg.DoModal () != IDOK)
	return;

    UCHAR bFlags, bMask;
    ACDUtil::GetFlagsFromPrefs (bFlags, bMask);

    for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i) {
	CACDHidDevice& Device = m_VirtualControlPanels.ElementAt (i)
	    ->GetDevice ();

	if (Device.IsAluminumCinemaDisplay ())
	    Device.SetFlags (Device.GetFlags () & ~bMask | bFlags);
    }
}

void
CACDPropertyPage::OnEnChangeBrightnessEdit ()
{
    if (!::IsWindow(m_cBrightnessEdit.m_hWnd /* not initialized */)
	|| GetFocus () != &m_cBrightnessEdit /* no focus */)
	return;

    char buffer [4]; // text limit is set to 3 chars
    m_cBrightnessEdit.GetWindowText (buffer, sizeof (buffer));
    buffer [sizeof (buffer) - 1] = '\0';
    int value = atoi (buffer);

    // check for out-of-range.
    if (value < 0 || value > 100) {
	value = min (0, max (value, 100));
	_snprintf (buffer, sizeof (buffer), "%d", value);
	buffer [sizeof (buffer) - 1] = '\0';

	m_cBrightnessEdit.SetWindowText (buffer);
    }

    if (m_cBrightnessSlider.GetPos () * 100 / 255 == value)
	// does not result in a brightness change.
	return;

    // convert to absolute [0-255]
    int brightness = value * 255 / 100;
    // ajust so that (brightness * 100 / 255) * 255 / 100 == brightness.
    while ((brightness * 100 / 255) != value) ++brightness;

    m_cBrightnessSlider.SetPos (brightness);
    m_pCurrentVirtualCP->SetBrightness (brightness);

    SetModified (TRUE);
}

BOOL
CACDPropertyPage::OnDeviceChange (UINT nEventType, DWORD_PTR dwData)
{
    PDEV_BROADCAST_HDR pdb = (PDEV_BROADCAST_HDR) dwData;

    switch (nEventType) {
    case DBT_DEVICEARRIVAL:
    case DBT_DEVICEREMOVECOMPLETE:
	if (pdb->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
	    break;

        KillTimer (ACD_SETTINGS_TIMER);

	for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i)
	    delete m_VirtualControlPanels.ElementAt (i);
	m_VirtualControlPanels.RemoveAll ();

        CACDVirtualCP::FindKnownVirtualCPs (m_VirtualControlPanels);
	InitializeControls ();
        SetModified (FALSE);

	if (m_VirtualControlPanels.GetCount () > 0)
	    SetTimer (ACD_SETTINGS_TIMER, 500, 0);
	break;

    default:
	break;
    }
    return CPropertyPage::OnDeviceChange (nEventType, dwData);
}
