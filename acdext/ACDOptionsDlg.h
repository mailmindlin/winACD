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

#pragma once

#include "ACDVirtualCP.h"

/**
 * CACDOptionsDialog dialog
 */
class CACDOptionsDialog : public CDialog
{
    DECLARE_DYNAMIC (CACDOptionsDialog)

private:

    /**
     * Member controls:
     */

    CComboBox	m_cPowerActionComboBox;
    CHotKeyCtrl m_cBrightnessIncreaseHK;
    CHotKeyCtrl m_cBrightnessDecreaseHK;
    CButton	m_cDisableBrightness;
    CButton	m_cDisablePower;
    
public:
    /** Constructor. */
    CACDOptionsDialog (CWnd* pParent = NULL);
    /** Destructor. */
    virtual ~CACDOptionsDialog ();

    /** Dialog Data */
    enum {
	IDD = IDD_ACD_OPTIONS_DIALOG
    };

protected:
    /** DDX/DDV support */
    virtual void DoDataExchange (CDataExchange* pDX);
    /** Dialog initialization. Configures the dlg controls. */
    virtual BOOL OnInitDialog ();
    /** On [OK] clicked. */
    virtual void OnOK ();

    DECLARE_MESSAGE_MAP ()
public:
    afx_msg void OnBnClickedDisablePower ();
};
