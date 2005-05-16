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

// ACDMon.h : main header file for the ACDMon application

#pragma once

#ifndef __AFXWIN_H__
# error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h" // main symbols

#include "ACDHidDev.h"

#define	ACD_WM_INIT_HOTKEYS (WM_APP + 0)

/**
 * CACDMonApp:
 * See ACDMon.cpp for the implementation of this class
 */
class CACDMonApp : public CWinApp
{
private:

    /** Array of ACD hid devices */
    CArray <CACDHidDevice*> m_DeviceArray;

    struct EnumHelper : CACDHidDevice::EnumHelper {
	/** the callbuf proc (call when we find a matching device */
	ENUMPROC_STATUS Callback (CACDHidDevice* pDevice);
    };

public:

    /** CMonApp construction. */
    CACDMonApp ();

    /** Destructor */
    virtual ~CACDMonApp ();

    /** Return the current brightness [0-15] */
    UINT GetBrightness ();

    /** Set the current brightness [0-15] */
    void SetBrightness (UINT nBrightness);


    // Overrides
public:

    /** CMonApp instance initialization */
    virtual BOOL InitInstance ();

protected:
    /** Registry change notification thread main proc */
    static UINT RegNotifyThreadMain (LPVOID pParam);

    DECLARE_MESSAGE_MAP ()
public:
};

extern CACDMonApp theApp;