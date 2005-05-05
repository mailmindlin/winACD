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
//
#pragma once

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define ACD_WM_RELOAD WM_APP + 0

// CACDMonApp:
// See ACDMon.cpp for the implementation of this class
//

class CACDMonApp : public CWinApp
{
public:
    enum {
	ACD_BRIGHTNESS_UP	= 1,
	ACD_BRIGHTNESS_DOWN	= 2
    };

    CACDMonApp();


// Overrides
public:
    virtual BOOL InitInstance();

// Implementation

public:
    DECLARE_MESSAGE_MAP()
};

extern CACDMonApp theApp;