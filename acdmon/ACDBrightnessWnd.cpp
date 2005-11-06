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

// CACDBrightnessWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ACDMon.h"
#include "ACDBrightnessWnd.h"
#include "ACDUtil.h"

#include <dbt.h>

// CBrightnessWnd

IMPLEMENT_DYNAMIC (CACDBrightnessWnd, CFrameWnd)

CACDBrightnessWnd::CACDBrightnessWnd (
    ACD_OSD_WND_POS bPos,
    CACDBrightnessWnd *pLowerWnd
    ) : m_bPos (bPos), m_bType (ACD_OSD_NONE), m_pLowerWnd (pLowerWnd),
	m_nOpacity (0), m_nTimer (0), m_nBrightness (0)
{
}

CACDBrightnessWnd::~CACDBrightnessWnd ()
{
}

BOOL
CACDBrightnessWnd::PreCreateWindow (CREATESTRUCT& cs)
{
    if (!CFrameWnd::PreCreateWindow (cs))
	return FALSE;

#if 0
    if (m_bPos == ACD_OSD_WND_FOREGROUND)
	m_bBackground.LoadBitmap (IDB_FOREGROUND);
    else {
        m_bBackground.LoadBitmap (IDB_BACKGROUND);
	m_bStep.LoadBitmap (IDB_STEP);
    }
    m_bType = ACD_OSD_BRIGHTNESS;
#else
    if (m_bPos == ACD_OSD_WND_BACKGROUND)
	m_bStep.LoadBitmap (IDB_STEP);
#endif

    // Modify the Window style.
    cs.dwExStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW
	| WS_EX_TOPMOST | WS_EX_TRANSPARENT
	;
    cs.style = WS_POPUP;

    // Set the position.
    CSize ssize;
    HDC dc = ::GetDC (NULL); 
    ssize.cx = GetDeviceCaps (dc, HORZRES) + GetSystemMetrics (SM_CXBORDER);
    ssize.cy = GetDeviceCaps (dc, VERTRES) + GetSystemMetrics (SM_CYBORDER);
    ::ReleaseDC (0, dc); 

    cs.cx = ACD_OSD_CX;
    cs.cy = ACD_OSD_CY;
    cs.x = (ssize.cx - cs.cx) / 2;
    cs.y = ssize.cy - cs.cy - ACD_OSD_MARGIN;

    // Set the class.
    cs.lpszClass = AfxRegisterWndClass (0);
    return TRUE;
}

void
CACDBrightnessWnd::SetOpacity (int opacity)
{
    ASSERT (m_bPos == ACD_OSD_WND_FOREGROUND);

    // range is [0-100]
    m_nOpacity = max (0, min (opacity, 100));

    // the foreground window is invisible until opacity reached 100%
    if (m_nOpacity == 100)
	SetLayeredWindowAttributes (RGB (255, 0, 255), 0, LWA_COLORKEY);
    else
	SetLayeredWindowAttributes (0, 0, LWA_ALPHA);

    m_pLowerWnd->SetLayeredWindowAttributes (
	0, (255 * m_nOpacity * ACD_OSD_ALPHA) / (100 * 100), LWA_ALPHA);
}

void
CACDBrightnessWnd::SetOSDType (ACD_OSD_TYPE type)
{
    ASSERT (m_bPos == ACD_OSD_WND_FOREGROUND);

    if (type != 0 && type != m_bType) {
	m_bBackground.DeleteObject ();
	m_pLowerWnd->m_bBackground.DeleteObject ();
    }

    if (type == m_bType)
	return;

    if (type == ACD_OSD_BRIGHTNESS) {
	m_bBackground.LoadBitmap (IDB_FOREGROUND);
	m_pLowerWnd->m_bBackground.LoadBitmap (IDB_BACKGROUND);
    }
    else if (type == ACD_OSD_POWER) {
	m_bBackground.LoadBitmap (IDB_PWR_FOREGROUND);
	m_pLowerWnd->m_bBackground.LoadBitmap (IDB_PWR_BACKGROUND);
    }

    m_pLowerWnd->m_bType = m_bType = type;

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = ACD_OSD_CX;
    rect.bottom = ACD_OSD_CY;

    m_pLowerWnd->InvalidateRect (&rect, TRUE);
    InvalidateRect (&rect, TRUE);
}

void
CACDBrightnessWnd::UpdateBrightness ()
{
    BOOL bRet;

    bRet = theApp.GetBrightness (&m_nBrightness);
    if (m_nBrightness == m_pLowerWnd->m_nBrightness || !bRet)
	return;

    m_pLowerWnd->m_nBrightness = m_nBrightness;

    RECT rect;
    rect.left = ACD_OSD_STEP_LEFT;
    rect.top = ACD_OSD_STEP_TOP;
    rect.right = ACD_OSD_STEP_LEFT + 16 * ACD_OSD_STEP_CX;
    rect.bottom = ACD_OSD_STEP_TOP + ACD_OSD_STEP_CY;

    m_pLowerWnd->InvalidateRect (&rect, TRUE);
    InvalidateRect (&rect, TRUE);
}

static const UINT
UWM_BEZEL_BN_CLICKED = ::RegisterWindowMessage (
    _T ("ACD_WM_BEZEL_BN_CLICKED"));

BEGIN_MESSAGE_MAP (CACDBrightnessWnd, CFrameWnd)
    ON_WM_TIMER ()
    ON_WM_CREATE ()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND ()
    ON_WM_DEVICECHANGE ()
    ON_MESSAGE (WM_HOTKEY, OnHotKey)
    ON_MESSAGE (ACD_WM_INIT_HOTKEYS, OnInitHotKeys)
    ON_REGISTERED_MESSAGE (UWM_BEZEL_BN_CLICKED, OnBezelBnClicked)
END_MESSAGE_MAP ()

// CBrightnessWnd message handlers

BOOL
CACDBrightnessWnd::OnEraseBkgnd (CDC* pDC)
{
    // Create an in-memory DC compatible with the
    // display DC we're using to paint
    CDC dcMemory;
    dcMemory.CreateCompatibleDC (pDC);

    // Select the bitmap into the in-memory DC
    CBitmap* pOldBitmap = dcMemory.SelectObject (&m_bBackground);
        
    // Copy the bits from the in-memory DC into the on-
    // screen DC to actually do the painting. Use the centerpoint
    // we computed for the target offset.
    pDC->BitBlt (0, 0, ACD_OSD_CX, ACD_OSD_CY, &dcMemory, 0, 0, SRCCOPY);

    dcMemory.SelectObject (pOldBitmap);
    return TRUE;
}

void
CACDBrightnessWnd::OnPaint ()
{
    CPaintDC dc (this);

    if (m_bType != ACD_OSD_BRIGHTNESS)
	return;

    if (m_bPos == ACD_OSD_WND_FOREGROUND) {
	for (UINT i = 0; i <= m_nBrightness; ++i)
	    dc.FillSolidRect (ACD_OSD_STEP_LEFT + ACD_OSD_STEP_CX * i,
		ACD_OSD_STEP_TOP, 12, 15, RGB (255, 255, 255));
    }
    else {
        CDC dcMemory;

	dcMemory.CreateCompatibleDC (&dc);
        CBitmap* pOldBitmap = dcMemory.SelectObject (&m_bStep);

        for (UINT i = 0; i <= m_nBrightness; ++i)
	    dc.BitBlt (ACD_OSD_STEP_LEFT + ACD_OSD_STEP_CX * i,
		ACD_OSD_STEP_TOP, 15, 20, &dcMemory, 0, 0, SRCCOPY);

        dcMemory.SelectObject (pOldBitmap);
    }
}

int
CACDBrightnessWnd::OnCreate (LPCREATESTRUCT lpCs)
{
    if (CFrameWnd::OnCreate (lpCs) == -1)
	return -1;

    // create the rounded corners.
    SetWindowRgn (CreateRoundRectRgn (
	0, 0, ACD_OSD_CX, ACD_OSD_CY, ACD_OSD_ROUND, ACD_OSD_ROUND
	), TRUE);

    // we start completely transparent.
    SetLayeredWindowAttributes (0, 0, LWA_ALPHA);
    m_nOpacity = 0;

    return 0;
}

LRESULT
CACDBrightnessWnd::OnHotKey (WPARAM wParam, LPARAM lParam)
{
    ASSERT (m_bPos == ACD_OSD_WND_FOREGROUND);

    int to, from = m_nBrightness;

    if (!theApp.GetBrightness (&m_nBrightness))
	return -1;

    to = m_nBrightness;
    switch (wParam) {
    case ACD_BRIGHTNESS_UP_HOTKEY:
	++to;
	break;
    case ACD_BRIGHTNESS_DOWN_HOTKEY:
	--to;
	break;
    default:
	break;
    }

    m_nBrightness = max (0, min (to, 15));
    if (m_nBrightness < from) {
	to = from;
	from = m_nBrightness;
    }
    else 
	to = m_nBrightness;

    if (from != to) {
	m_pLowerWnd->m_nBrightness = m_nBrightness;
	theApp.SetBrightness (m_nBrightness);
    }

    if (ACDUtil::GetDisableOSDPref ())
	return 0;

    if (from != to) {
	RECT rect;
	rect.left = ACD_OSD_STEP_LEFT + ACD_OSD_STEP_CX * from;
	rect.top = ACD_OSD_STEP_TOP;
	rect.right = rect.left + (to - from + 1) * ACD_OSD_STEP_CX;
	rect.bottom = rect.top + ACD_OSD_STEP_CY;

	m_pLowerWnd->InvalidateRect (&rect, TRUE);
	InvalidateRect (&rect, TRUE);
    }

    if (m_nTimer)
	KillTimer (m_nTimer);

    SetOSDType (ACD_OSD_BRIGHTNESS);

    if (m_nOpacity < 100) {
	// place the windows at the top of the z-order.
	m_pLowerWnd->SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
	    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
	    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE| SWP_SHOWWINDOW);

	SetOpacity (100);
    }

    m_nTimer = SetTimer (ACD_FADE_WND_TIMER_START, 1000, 0);

    return 0;
}

void
CACDBrightnessWnd::OnTimer (UINT nIDEvent) 
{
    ASSERT (m_bPos == ACD_OSD_WND_FOREGROUND);

    if (nIDEvent == ACD_FADE_WND_TIMER_START) {
	KillTimer (ACD_FADE_WND_TIMER_START);
	m_nTimer = SetTimer (ACD_FADE_WND_TIMER_STEP, 20, 0);
    }
    else if (nIDEvent == ACD_FADE_WND_TIMER_STEP) {
	if (m_nOpacity == 100)
	    SetWindowPos (&CWnd::wndBottom, 0, 0, 0, 0,
		SWP_NOMOVE |SWP_NOSIZE |SWP_NOACTIVATE);

	SetOpacity ((int) m_nOpacity - ACD_OSD_ALPHA_STEP);

	if (m_nOpacity > 0)
	    return;
	
	KillTimer (ACD_FADE_WND_TIMER_STEP);
	m_nTimer = 0;

	ShowWindow (SW_HIDE);
   	m_pLowerWnd->ShowWindow (SW_HIDE);

	// place the windows at the bottom of the z-order.
	m_pLowerWnd->SetWindowPos (&CWnd::wndBottom, 0, 0, 0, 0,
	    SWP_NOMOVE |SWP_NOSIZE |SWP_NOACTIVATE);
    }
    else if (nIDEvent == ACD_BRIGHTNESS_REFRESH_TIMER) {
	UpdateBrightness ();
    }
    else 
	CFrameWnd::OnTimer (nIDEvent);
}

LRESULT
CACDBrightnessWnd::OnInitHotKeys (WPARAM wParam, LPARAM lParam)
{
    ASSERT (m_bPos == ACD_OSD_WND_FOREGROUND);

    DWORD dwIncreaseHotKey = ACDUtil::GetHotKeyPref (TRUE);
    WORD wVirtualKeyCode, wModifiers;

    wVirtualKeyCode = LOWORD (dwIncreaseHotKey);
    wModifiers = 0;
    
    if (HIWORD (dwIncreaseHotKey) & HOTKEYF_SHIFT)
	wModifiers |= MOD_SHIFT;
    if (HIWORD (dwIncreaseHotKey) & HOTKEYF_CONTROL)
	wModifiers |= MOD_CONTROL;
    if (HIWORD (dwIncreaseHotKey) & HOTKEYF_ALT)
	wModifiers |= MOD_ALT;

    if (dwIncreaseHotKey == 0)
	::UnregisterHotKey (m_hWnd, ACD_BRIGHTNESS_UP_HOTKEY);
    else
        ::RegisterHotKey (m_hWnd, ACD_BRIGHTNESS_UP_HOTKEY,
  	    wModifiers, wVirtualKeyCode);

    DWORD dwDecreaseHotKey = ACDUtil::GetHotKeyPref (FALSE);
    wVirtualKeyCode = LOWORD (dwDecreaseHotKey);
    wModifiers = 0;
    
    if (HIWORD (dwDecreaseHotKey) & HOTKEYF_SHIFT)
	wModifiers |= MOD_SHIFT;
    if (HIWORD (dwDecreaseHotKey) & HOTKEYF_CONTROL)
	wModifiers |= MOD_CONTROL;
    if (HIWORD (dwDecreaseHotKey) & HOTKEYF_ALT)
	wModifiers |= MOD_ALT;

    if (dwIncreaseHotKey == 0)
	::UnregisterHotKey (m_hWnd, ACD_BRIGHTNESS_DOWN_HOTKEY);
    else
	::RegisterHotKey (m_hWnd, ACD_BRIGHTNESS_DOWN_HOTKEY,
	    wModifiers, wVirtualKeyCode);

    return 0;
}

BOOL
CACDBrightnessWnd::OnDeviceChange (UINT nEventType, DWORD_PTR dwData)
{
    PDEV_BROADCAST_HDR pdb = (PDEV_BROADCAST_HDR) dwData;

    switch (nEventType) {
    case DBT_DEVICEARRIVAL:
    case DBT_DEVICEREMOVECOMPLETE:
	if (pdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
	    if (theApp.OnDeviceChange (nEventType,
		((PDEV_BROADCAST_DEVICEINTERFACE)pdb)->dbcc_name))
		return TRUE;
    default:
	return CFrameWnd::OnDeviceChange (nEventType, dwData);
    }
}

static UINT
RunDisplayProperties (LPVOID pParam)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory (&si, sizeof (si));
    si.cb = sizeof (si);

     if (CreateProcess (NULL,
	    "rundll32.exe shell32.dll,Control_RunDLL desk.cpl,,3",
	    NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi
	    )) {
	WaitForSingleObject (pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
    }
    return 0;
}

LRESULT
CACDBrightnessWnd::OnBezelBnClicked (WPARAM wParam, LPARAM lParam)
{
    if (m_bPos != ACD_OSD_WND_FOREGROUND)
	return 0;

    DWORD dwCurrentSessionId;
    if (!ProcessIdToSessionId (GetCurrentProcessId (), &dwCurrentSessionId))
	return -1;

    /* we are not the current session, ignore the event */
    if (dwCurrentSessionId != WTSGetActiveConsoleSessionId ())
	return 0;

    switch (wParam) {
	case 0:
	    if (m_nTimer == ACD_BRIGHTNESS_REFRESH_TIMER) {
		UpdateBrightness ();
		KillTimer (ACD_BRIGHTNESS_REFRESH_TIMER);
		m_nTimer = SetTimer (ACD_FADE_WND_TIMER_START, 1000, 0);
	    }
	    break;

	case ACD_BUTTON_POWER:
	    if (ACDUtil::GetDisableOSDPref ())
		break;

	    if (m_nTimer)
		KillTimer (m_nTimer);

	    SetOSDType (ACD_OSD_POWER);

	    if (m_nOpacity < 100) {
		// place the windows at the top of the z-order.
		m_pLowerWnd->SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
		    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
		    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		SetOpacity (100);
	    }

	    m_nTimer = SetTimer (ACD_FADE_WND_TIMER_START, 500, 0);
	    break;

	case ACD_BUTTON_BRIGHTNESS_UP:
	case ACD_BUTTON_BRIGHTNESS_DOWN:
	    UpdateBrightness ();

	    if (ACDUtil::GetDisableOSDPref ())
		break;

	    if (m_nTimer)
		KillTimer (m_nTimer);

	    SetOSDType (ACD_OSD_BRIGHTNESS);

	    if (m_nOpacity < 100) {
		// place the windows at the top of the z-order.
		m_pLowerWnd->SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
		    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
		    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		SetOpacity (100);
	    }

	    m_nTimer = SetTimer (ACD_BRIGHTNESS_REFRESH_TIMER, 50, 0);
	    break;

	case ACD_BUTTON_USER_ACTION:
	    AfxBeginThread (RunDisplayProperties, 0);
	    break;

	default:
	    break;
    }
    return 0;
}
