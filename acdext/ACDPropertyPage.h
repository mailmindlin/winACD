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
 * CACDPropertyPage dialog
 */
class CACDPropertyPage : public CPropertyPage
{
    DECLARE_DYNAMIC (CACDPropertyPage)

private:

    /** Array of virtual control panels currently connected */
    CACDVirtualCP::CVirtualCPArray m_VirtualControlPanels;
    /** Current virtual control panel */
    CACDVirtualCP* m_pCurrentVirtualCP;

    /**
     * Member controls:
     */

    CListBox	m_cVirtualControlList;	    //!< Current virtual control panel.
    CSliderCtrl	m_cBrightnessSlider;	    //!< Current brightness slider.
    CEdit	m_cBrightnessEdit;	    //!< Current brightness edit.

public:

    /** Default constructor. */
    CACDPropertyPage();
    /** Destructor. */
    virtual ~CACDPropertyPage();

    /** Dialog Data */
    enum {
	IDD = IDD_ACD_PROPERTY_PAGE,
	ACD_SETTINGS_TIMER = 1
    };

    /** Eanble/Disable the Apply now buton */
    void SetModified (BOOL bChanged = TRUE);

protected:

    /** Update the dlg controls (when the current VCP has changed). */
    void UpdateControls ();
    /** Update the window text for the brightness edit control. */
    void UpdateBrightnessEdit ();

    /** DDX/DDV support */
    virtual void DoDataExchange (CDataExchange* pDX);
    /** Dialog initialization. Configures the dlg controls. */
    virtual BOOL OnInitDialog ();
    /** Apply the settings (invoked the users presses OK) */
    virtual BOOL OnApply ();
    /** Reset the settings (invoked the users presses Cancel) */
    virtual void OnReset ();
    /** Finish the PropertyPage objects when the WND has been destroyed. */
    virtual void PostNcDestroy ();

    DECLARE_MESSAGE_MAP()
public:
    /** VCP selection change (when we have multiple VCPs). */
    afx_msg void OnLbnSelchangeVirtualControlList ();
    /** Check brightness settings timer event */
    void OnTimer (UINT nIDEvent);
    /** On Horizontal scroll (should only be the Brightness slider. */
    afx_msg void OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    /** On [Properties] clicked. */
    afx_msg void OnBnClickedPropertiesButton ();
    /** On [Options...] clicked. */
    afx_msg void OnBnClickedOptionsButton ();
    /** When text is entered in the brightness edit */
    afx_msg void OnEnChangeBrightnessEdit ();
};
