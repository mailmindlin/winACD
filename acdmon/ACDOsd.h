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

// ACDOsd.h : interface of the CMainFrame class
//


#pragma once

#include "ACDHidDev.h"

class CACDBrightnessOsd : public CFrameWnd
{
private:
    static const int ACD_OSD_CX		= 300;
    static const int ACD_OSD_CY		= 300;
    static const int ACD_OSD_MARGIN	= 150;
    static const int ACD_OSD_ROUND	= 60;
    static const int ACD_OSD_ALPHA	= 70;
    static const int ACD_OSD_ALPHA_STEP	= 5;

    enum {
	ACD_OSD_TIMER_START	= 1,
	ACD_OSD_TIMER_REPEAT	= 2
    };

private:
    CACDBrightnessOsd* m_pOtherLayeredWnd;
    CBitmap m_bBackground;
    CBitmap m_bForeground;
    CBitmap m_bStep;

    int		m_Opacity;
    UINT_PTR	m_Timer;
    int		m_Brightness;

    CACDHidDevice *m_pDevice;

    CACDBrightnessOsd (int);
public:
    CACDBrightnessOsd ();
    virtual ~CACDBrightnessOsd ();

protected: 
    DECLARE_DYNAMIC (CACDBrightnessOsd)

private:
    void DrawSteps (CDC& dc, int from, int to, BOOL on);
    void SetOpacity (int opacity);

public:
    void SetBrightness (int brightness);

    virtual BOOL PreCreateWindow (CREATESTRUCT& cs);

#ifdef _DEBUG
    virtual void AssertValid () const;
    virtual void Dump (CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd (CDC* pDC);
    afx_msg void OnPaint ();
    afx_msg void OnTimer (UINT nIDEvent);
    afx_msg LRESULT OnHotKey (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnReload (WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP ()
};


