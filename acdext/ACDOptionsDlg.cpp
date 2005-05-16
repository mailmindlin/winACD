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
CACDOptionsDialog::CACDOptionsDialog (CWnd* pParent)
    : CDialog (CACDOptionsDialog::IDD, pParent)
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
    m_cPowerActionComboBox.EnableWindow (!m_cDisablePower.GetCheck ());

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
}


BEGIN_MESSAGE_MAP (CACDOptionsDialog, CDialog)
    ON_BN_CLICKED (IDC_DISABLE_POWER, OnBnClickedDisablePower)
END_MESSAGE_MAP ()


// CACDOptionsDialog message handlers

void
CACDOptionsDialog::OnBnClickedDisablePower ()
{
    m_cPowerActionComboBox.EnableWindow (!m_cDisablePower.GetCheck ());
}
