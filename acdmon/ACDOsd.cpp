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

// ACDOsd.cpp : implementation of the CACDBrightnessOsd class
//

#include "stdafx.h"
#include "ACDMon.h"

#include "ACDOsd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <assert.h>
#include <setupapi.h>


IMPLEMENT_DYNAMIC (CACDBrightnessOsd, CFrameWnd)

BEGIN_MESSAGE_MAP (CACDBrightnessOsd, CFrameWnd)
    ON_WM_CREATE ()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND ()
    ON_WM_TIMER ()
    ON_MESSAGE (WM_HOTKEY, OnHotKey)
    ON_MESSAGE (ACD_WM_RELOAD, OnReload)
END_MESSAGE_MAP ()


CACDBrightnessOsd::CACDBrightnessOsd (int unused)
: m_pOtherLayeredWnd (NULL), m_Brightness (0), m_Timer (0), m_pDevice (0)
{
    unused;
}

CACDBrightnessOsd::CACDBrightnessOsd ()
: m_Brightness (0), m_Timer (0)
{
    m_pOtherLayeredWnd = new CACDBrightnessOsd (0);
    assert (m_pOtherLayeredWnd);
    m_pOtherLayeredWnd->Create (NULL, NULL);

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

	m_pDevice = new CACDHidDevice (hidDevice);
	m_Brightness = m_pDevice->GetBrightness () / 16;
	break;
    }

}

CACDBrightnessOsd::~CACDBrightnessOsd ()
{
    if (m_pOtherLayeredWnd)
	delete m_pOtherLayeredWnd;
    if (m_pDevice)
	delete m_pDevice;
}


int CACDBrightnessOsd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate (lpCreateStruct) == -1)
	return -1;

    SetWindowRgn (
	CreateRoundRectRgn (
	    0, 0, ACD_OSD_CX, ACD_OSD_CY, ACD_OSD_ROUND, ACD_OSD_ROUND
	), TRUE);

    // We start completely transparent.
    m_Opacity = 0;
    SetLayeredWindowAttributes (0, (255 * m_Opacity) / 100, LWA_ALPHA);

    // Fill the window backing store (while invisible)
    CFrameWnd::ShowWindow (SW_SHOWNOACTIVATE);
    CFrameWnd::UpdateWindow ();
    CFrameWnd::ShowWindow (SW_HIDE);
    CFrameWnd::UpdateWindow ();

    return 0;
}

BOOL CACDBrightnessOsd::OnEraseBkgnd (CDC* pDC)
{
    return TRUE;
}

void CACDBrightnessOsd::DrawSteps (CDC& dc, int from, int to, BOOL on)
{
    if (m_pOtherLayeredWnd) {
	// Foreground ON:white OFF:grey.
	for (int i = from; i <= to; ++i)
	    dc.FillSolidRect (15 * i + 32, 245, 12, 15,
		on ? RGB (255, 255, 255) : RGB (39, 39, 39));
    }
    else if (on) {
	// Background ON:bitmap.
        CDC dcMemory;
	dcMemory.CreateCompatibleDC (&dc);
        CBitmap* pOldBitmap = dcMemory.SelectObject (&m_bStep);

        for (int i = from; i <= to; ++i)
	    dc.BitBlt (15 * i + 32, 245, 15, 20,
		&dcMemory, 0, 0, SRCCOPY);

        dcMemory.SelectObject (pOldBitmap);
    }
    else {
	// Background OFF:clear and black rect.
        for (int i = from; i <= to; ++i) {
	    dc.FillSolidRect (15 * i + 44, 245, 3, 20, RGB (102, 102, 102));
	    dc.FillSolidRect (15 * i + 32, 260, 12, 5, RGB (102, 102, 102));
	    dc.FillSolidRect (15 * i + 32, 245, 12, 15, RGB (0, 0, 0));
	}
    }
}

void CACDBrightnessOsd::OnPaint ()
{
    CPaintDC dc (this);

    // Create an in-memory DC compatible with the
    // display DC we're using to paint
    CDC dcMemory;
    dcMemory.CreateCompatibleDC (&dc);
        
    // Select the bitmap into the in-memory DC
    CBitmap* pNewBitmap = m_pOtherLayeredWnd
	? &m_bForeground : &m_bBackground;
    CBitmap* pOldBitmap = dcMemory.SelectObject (pNewBitmap);
        
    // Copy the bits from the in-memory DC into the on-
    // screen DC to actually do the painting. Use the centerpoint
    // we computed for the target offset.
    dc.BitBlt (0, 0, ACD_OSD_CX, ACD_OSD_CY, &dcMemory, 0, 0, SRCCOPY);

    dcMemory.SelectObject (pOldBitmap);

    DrawSteps (dc, 0, m_Brightness, TRUE);
}

void CACDBrightnessOsd::OnTimer(UINT nIDEvent) 
{
    if (nIDEvent == ACD_OSD_TIMER_START) {
	KillTimer (m_Timer);
	m_Timer = SetTimer (ACD_OSD_TIMER_REPEAT, 20, 0);
    }

    SetOpacity (m_Opacity - ACD_OSD_ALPHA_STEP);
    if (m_Opacity == 0) {
	KillTimer (m_Timer);
	m_Timer = 0;

	ShowWindow (SW_HIDE);
   	m_pOtherLayeredWnd->ShowWindow (SW_HIDE);

	// Place the windows at the bottom of the z-order.
	SetWindowPos (&CWnd::wndBottom, 0, 0, 0, 0,
	    SWP_NOMOVE |SWP_NOSIZE |SWP_NOACTIVATE);
	m_pOtherLayeredWnd->SetWindowPos (&CWnd::wndBottom, 0, 0, 0, 0,
	    SWP_NOMOVE |SWP_NOSIZE |SWP_NOACTIVATE);
    }

    CFrameWnd::OnTimer(nIDEvent);
}

LRESULT CACDBrightnessOsd::OnHotKey (WPARAM wParam, LPARAM lParam)
{
    m_Brightness = m_pDevice->GetBrightness () / 16;

    switch (wParam) {
    case CACDMonApp::ACD_BRIGHTNESS_UP:
	SetBrightness (m_Brightness + 1);
	break;
    case CACDMonApp::ACD_BRIGHTNESS_DOWN:
	SetBrightness (m_Brightness - 1);
	break;
    }

    m_pDevice->SetBrightness (m_Brightness * 16 + m_Brightness);
    return 0;
}

LRESULT CACDBrightnessOsd::OnReload (WPARAM wParam, LPARAM lParam)
{
    if (!m_pOtherLayeredWnd)
	return 0;

    // TODO: write the reload code.
    return 0;
}

BOOL CACDBrightnessOsd::PreCreateWindow (CREATESTRUCT& cs)
{
    if (!CFrameWnd::PreCreateWindow (cs))
	return FALSE;


    if (m_pOtherLayeredWnd)
	m_bForeground.LoadBitmap (IDB_FOREGROUND);
    else {
        m_bBackground.LoadBitmap (IDB_BACKGROUND);
	m_bStep.LoadBitmap (IDB_STEP);
    }

    // Modify the Window style.
    cs.dwExStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW
	| WS_EX_TOPMOST | WS_EX_TRANSPARENT
	;
    cs.style = WS_POPUP;

    // Set the position.
    CSize ssize;
    HDC dc = ::GetDC(NULL); 
    ssize.cx = GetDeviceCaps (dc, HORZRES) + GetSystemMetrics (SM_CXBORDER);
    ssize.cy = GetDeviceCaps (dc, VERTRES) + GetSystemMetrics (SM_CYBORDER);
    ::ReleaseDC (0,dc); 

    cs.cx = ACD_OSD_CX;
    cs.cy = ACD_OSD_CY;
    cs.x = (ssize.cx - cs.cx) / 2;
    cs.y = ssize.cy - cs.cy - ACD_OSD_MARGIN;

    // Set the class.
    cs.lpszClass = AfxRegisterWndClass (0);
    return TRUE;
}


void CACDBrightnessOsd::SetOpacity (int opacity)

{
    assert (m_pOtherLayeredWnd);

    m_Opacity = opacity < 0 ? 0 : (opacity > 100 ? 100 : opacity);

    if (m_Opacity == 100)
	SetLayeredWindowAttributes (RGB (255, 0, 255), 0, LWA_COLORKEY);
    else
	SetLayeredWindowAttributes (0, 0, LWA_ALPHA);

    m_pOtherLayeredWnd->SetLayeredWindowAttributes (
	0, (255 * m_Opacity * ACD_OSD_ALPHA) / 10000, LWA_ALPHA);
}


void CACDBrightnessOsd::SetBrightness (int brightness)
{
    if (brightness < 0) brightness = 0;
    if (brightness > 15) brightness = 15;

    assert (m_pOtherLayeredWnd);
    CClientDC fdc (this);
    CClientDC bdc (m_pOtherLayeredWnd);

    if (brightness > m_Brightness) {
	m_pOtherLayeredWnd->DrawSteps (bdc, m_Brightness + 1, brightness, TRUE);
	DrawSteps (fdc, m_Brightness + 1, brightness, TRUE);
    }
    else {
	m_pOtherLayeredWnd->DrawSteps (bdc, brightness + 1, m_Brightness, FALSE);
	DrawSteps (fdc, brightness + 1, m_Brightness, FALSE);
    }
    m_pOtherLayeredWnd->m_Brightness = m_Brightness = brightness;

    if (m_Opacity != 100) {
	SetOpacity (100);

	// Place the windows at the top of the z-order.
	m_pOtherLayeredWnd->SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
	    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	SetWindowPos (&CWnd::wndTopMost, 0, 0, 0, 0,
	    SWP_NOMOVE |SWP_NOSIZE | SWP_NOACTIVATE| SWP_SHOWWINDOW);
    }

    if (m_Timer) {
	KillTimer (m_Timer);
    }

    m_Timer = SetTimer (ACD_OSD_TIMER_START, 1000, 0);
}

// CACDBrightnessOsd diagnostics

#ifdef _DEBUG

void CACDBrightnessOsd::AssertValid () const
{
    CFrameWnd::AssertValid ();
}

void CACDBrightnessOsd::Dump (CDumpContext& dc) const
{
    CFrameWnd::Dump (dc);
}

#endif //_DEBUG
