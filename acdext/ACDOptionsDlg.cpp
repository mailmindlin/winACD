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

// ACDOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ACDExt.h"
#include "ACDOptionsDlg.h"
#include "ACDUtil.h"

extern "C" {
#include <powrprof.h>
}

// CACDOptionsDialog dialog

IMPLEMENT_DYNAMIC (CACDOptionsDialog, CDialog)
CACDOptionsDialog::CACDOptionsDialog (
    CACDVirtualCP::CVirtualCPArray& cArray,
    CWnd* pParent
    ) : CDialog (CACDOptionsDialog::IDD, pParent),
	m_VirtualControlPanels (cArray)
{
}

CACDOptionsDialog::~CACDOptionsDialog ()
{
}

BOOL
CACDOptionsDialog::OnInitDialog ()
{
    ACDPowerButtonAction action = ACDUtil::GetPowerButtonActionPref ();
    int index, selection;

    CDialog::OnInitDialog ();

    index = m_cPowerActionComboBox.AddString ("Turn monitor off");
    m_cPowerActionComboBox.SetItemData (index, ACD_POWER_BUTTON_DO_NOTHING);
    selection = index; // default

    if (IsPwrSuspendAllowed ()) {
        index = m_cPowerActionComboBox.AddString ("Stand by");
        m_cPowerActionComboBox.SetItemData (index, ACD_POWER_BUTTON_STAND_BY);
	if (action == ACD_POWER_BUTTON_STAND_BY)
	    selection = index;
    }
    if (IsPwrHibernateAllowed ()) {
	index = m_cPowerActionComboBox.AddString ("Hibernate");
	m_cPowerActionComboBox.SetItemData (index, ACD_POWER_BUTTON_HIBERNATE);
	if (action == ACD_POWER_BUTTON_HIBERNATE)
	    selection = index;
    }
    if (IsPwrShutdownAllowed ()) {
	index = m_cPowerActionComboBox.AddString ("Shut down");
	m_cPowerActionComboBox.SetItemData (index, ACD_POWER_BUTTON_SHUT_DOWN);
	if (action == ACD_POWER_BUTTON_SHUT_DOWN)
	    selection = index;
    }

    // select the action in the combo-list.
    m_cPowerActionComboBox.SetCurSel (selection);

    // set the force shutdown check.
    m_cForceShutdown.SetCheck (ACDUtil::GetForceShutdownPref ());

    // set the hotkeys.
    DWORD hotkey = ACDUtil::GetHotKeyPref (TRUE);
    m_cBrightnessIncreaseHK.SetHotKey (LOWORD (hotkey), HIWORD (hotkey));

    hotkey = ACDUtil::GetHotKeyPref (FALSE);
    m_cBrightnessDecreaseHK.SetHotKey (LOWORD (hotkey), HIWORD (hotkey));

    // set the disable check
    UCHAR bDisabledButtons = ACDUtil::GetDisabledButtonsPref ();
    m_cDisableBrightness.SetCheck (0 !=
	(bDisabledButtons & CACDHidDevice::ACD_FLAGS_BRIGHTNESS_BUTTON));
    m_cDisablePower.SetCheck (0 !=
	(bDisabledButtons & CACDHidDevice::ACD_FLAGS_POWER_BUTTON));

    // disable/enable the combo-list if cDisable is checked/unchecked.
    BOOL bEnable = !m_cDisablePower.GetCheck ();
    m_cPowerActionComboBox.EnableWindow (bEnable);
    bEnable &= action != ACD_POWER_BUTTON_DO_NOTHING;
    m_cForceShutdown.EnableWindow (bEnable);

    BOOL bIsAluminum = FALSE;
    for (INT_PTR i = 0; i < m_VirtualControlPanels.GetCount (); ++i)
	bIsAluminum |= m_VirtualControlPanels.ElementAt (i)
	    ->GetDevice ().IsAluminumCinemaDisplay ();

    if (!bIsAluminum) {
	GetDlgItem (IDC_POWER_BUTTON_GROUP)->EnableWindow (FALSE);
	GetDlgItem (IDC_POWER_COMBO_STATIC)->EnableWindow (FALSE);
	GetDlgItem (IDC_MONITOR_CONTROL_GROUP)->EnableWindow (FALSE);
	m_cPowerActionComboBox.EnableWindow (FALSE);
	m_cForceShutdown.EnableWindow (FALSE);
	m_cDisablePower.EnableWindow (FALSE);
	m_cDisableBrightness.EnableWindow (FALSE);
    }

    return TRUE;
}

void
CACDOptionsDialog::OnOK ()
{
    WORD wVirtualKeyCode, wModifiers;

    m_cBrightnessIncreaseHK.GetHotKey (wVirtualKeyCode, wModifiers);
    ACDUtil::SetHotKeyPref (TRUE, MAKELONG (wVirtualKeyCode, wModifiers));

    m_cBrightnessDecreaseHK.GetHotKey (wVirtualKeyCode, wModifiers);
    ACDUtil::SetHotKeyPref (FALSE, MAKELONG (wVirtualKeyCode, wModifiers));


    ACDUtil::SetDisabledButtonsPref (
	(m_cDisableBrightness.GetCheck ()
	    ? CACDHidDevice::ACD_FLAGS_BRIGHTNESS_BUTTON : 0)
	| (m_cDisablePower.GetCheck ()
	    ? CACDHidDevice::ACD_FLAGS_POWER_BUTTON : 0)
	);

    int index = m_cPowerActionComboBox.GetCurSel ();
    ACDUtil::SetPowerButtonActionPref ((ACDPowerButtonAction) 
	m_cPowerActionComboBox.GetItemData (index));

    ACDUtil::SetForceShutdownPref (m_cForceShutdown.GetCheck ());

    CDialog::OnOK ();
}

void
CACDOptionsDialog::DoDataExchange (CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control (pDX, IDC_POWER_ACTION_COMBO, m_cPowerActionComboBox);
    DDX_Control (pDX, IDC_HOTKEY_INCREASE, m_cBrightnessIncreaseHK);
    DDX_Control (pDX, IDC_HOTKEY_DECREASE, m_cBrightnessDecreaseHK);
    DDX_Control (pDX, IDC_DISABLE_BRIGHTNESS, m_cDisableBrightness);
    DDX_Control (pDX, IDC_DISABLE_POWER, m_cDisablePower);
    DDX_Control (pDX, IDC_FORCE_SHUTDOWN, m_cForceShutdown);
}


BEGIN_MESSAGE_MAP (CACDOptionsDialog, CDialog)
    ON_BN_CLICKED (IDC_DISABLE_POWER, OnBnClickedDisablePower)
    ON_CBN_SELCHANGE (IDC_POWER_ACTION_COMBO, OnCbnSelchangePowerActionCombo)
END_MESSAGE_MAP ()


// CACDOptionsDialog message handlers

void
CACDOptionsDialog::OnBnClickedDisablePower ()
{
    BOOL bEnable = !m_cDisablePower.GetCheck ();
    m_cPowerActionComboBox.EnableWindow (bEnable);
    bEnable &= m_cPowerActionComboBox.GetItemData (
	m_cPowerActionComboBox.GetCurSel ()
	) != ACD_POWER_BUTTON_DO_NOTHING;
    m_cForceShutdown.EnableWindow (bEnable);
}

void
CACDOptionsDialog::OnCbnSelchangePowerActionCombo ()
{
    int index = m_cPowerActionComboBox.GetCurSel ();

    m_cForceShutdown.EnableWindow (m_cPowerActionComboBox.GetItemData (index)
	!= ACD_POWER_BUTTON_DO_NOTHING);
}
