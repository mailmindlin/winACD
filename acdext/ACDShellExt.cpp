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

// ACDShellExt.cpp : Implementation of CACDShellExt

#include "stdafx.h"
#include "ACDShellExt.h"
#include "ACDPropertyPage.h"

// CACDShellExt

//
// IShellExtInit interface:
//

HRESULT
CACDShellExt::Initialize (
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
    return S_OK;
}

//
//IShellPropSheetExt interface:
//

HRESULT
CACDShellExt::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPageProc, LPARAM lParam)
{
    HPROPSHEETPAGE hpsp;

    if (lpfnAddPageProc == NULL) {
	return E_INVALIDARG;
    }

    // Initialize MFC.
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

    // Create a new CACDPropertyPage.
    CACDPropertyPage* pPsp = new CACDPropertyPage ();
    hpsp = CreatePropertySheetPage (&pPsp->m_psp);
    if (hpsp == NULL) {
	return E_FAIL;
    }

    // Add the new sheet.
    if (!lpfnAddPageProc (hpsp, lParam)) {
	DestroyPropertySheetPage (hpsp);
	return E_FAIL;
    }

    return S_OK;
}