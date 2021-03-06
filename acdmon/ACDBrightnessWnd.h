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

/**
 * CACDBrightnessWnd frame
 */
class CACDBrightnessWnd : public CFrameWnd
{
    DECLARE_DYNAMIC (CACDBrightnessWnd)

    /** Wnd parameters */
    static const int ACD_OSD_CX		= 300;
    static const int ACD_OSD_CY		= 300;
    static const int ACD_OSD_MARGIN	= 150;
    static const int ACD_OSD_ROUND	= 60;
    static const int ACD_OSD_ALPHA	= 70;
    static const int ACD_OSD_ALPHA_STEP	= 5;

    /** Step parameters */
    static const int ACD_OSD_STEP_LEFT	= 32;
    static const int ACD_OSD_STEP_TOP	= 245;
    static const int ACD_OSD_STEP_CX	= 15;
    static const int ACD_OSD_STEP_CY	= 20;

    /** HotKeys */
    enum {
	ACD_BRIGHTNESS_UP_HOTKEY,
	ACD_BRIGHTNESS_DOWN_HOTKEY
    };

    /** Timers */
    enum {
	ACD_FADE_WND_TIMER_START	= 1,
	ACD_FADE_WND_TIMER_STEP		= 2,
	ACD_BRIGHTNESS_REFRESH_TIMER	= 3
    };

public:

    /** OSD type */
    enum ACD_OSD_TYPE {
	ACD_OSD_NONE = 0,
	ACD_OSD_BRIGHTNESS,
	ACD_OSD_POWER
    };

    /** Window position (foreground/background wnd) */
    enum ACD_OSD_WND_POS {
	ACD_OSD_WND_BACKGROUND,
	ACD_OSD_WND_FOREGROUND
    };

private:

    ACD_OSD_WND_POS	m_bPos;		//!< The window pos.
    ACD_OSD_TYPE	m_bType;	//!< The OSD type.
    CACDBrightnessWnd*	m_pLowerWnd;    //!< bkgrnd wnd if type is fgrnd.

    CBitmap	m_bBackground;	//!< Background bitmap.
    CBitmap	m_bStep;	//!< Bitmap used for each individual steps.
    UINT_PTR	m_nTimer;	//!< Current active timer.
    UINT	m_nOpacity;	//!< Current wnd opacity.
    UCHAR	m_nBrightness;	//!< Current ACD(s) brightness.

public:

    /** CACDBrightnessWnd construction. */
    CACDBrightnessWnd (
	ACD_OSD_WND_POS bType,
	CACDBrightnessWnd* pLowerWnd = NULL
	);

    /** Destructor. */
    virtual ~CACDBrightnessWnd ();

    /** Call before the wnd creation by the framework. */
    virtual BOOL PreCreateWindow (CREATESTRUCT& cs);

    /** Set the window opacity (must be called on the Foreground wnd only) */
    void SetOpacity (int opacity);

    /** Update the brightness value */
    void UpdateBrightness ();

    /** Set the OSD message type (power/brightness) */
    void SetOSDType (ACD_OSD_TYPE type);

protected:

    DECLARE_MESSAGE_MAP()

public:

    /** Called by MonApp when a bezel button is clicked */
    afx_msg LRESULT OnBezelBnClicked (WPARAM wParam, LPARAM lParam);
    /** Initialize the HotKeys (called by init and the MonApp notify thread) */
    afx_msg LRESULT OnInitHotKeys (WPARAM wParam = 0, LPARAM lParam = 0);
    /** Called when a HotKey (Brightness UP/DOWN) is pressed */
    afx_msg LRESULT OnHotKey (WPARAM wParam, LPARAM lParam);
    /** Called by the frameword when the background needs to be erased */
    afx_msg BOOL OnEraseBkgnd (CDC* pDC);
    /** Called by the frameword when the window needs to be repainted */
    afx_msg void OnPaint ();
    /** Called after the window is created but before it is made visible */
    afx_msg int OnCreate (LPCREATESTRUCT lpCs);
    /** Called by the framework when the fade timer expires */
    afx_msg void OnTimer (UINT nIDEvent);
    /** Called when an HID device is inserted/removed */
    afx_msg BOOL OnDeviceChange (UINT nEventType, DWORD_PTR dwData);
};


