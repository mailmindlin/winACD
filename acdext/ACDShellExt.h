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

// ACDShellExt.h : Declaration of the CACDShellExt

#pragma once
#include "resource.h"       // main symbols

#include "ACDExt.h"

#include <shlobj.h>
#include <comdef.h>

// CACDShellExt

class ATL_NO_VTABLE CACDShellExt : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CACDShellExt, &CLSID_ACDShellExt>,
    public IDispatchImpl<IACDShellExt, &IID_IACDShellExt, &LIBID_ACDExtLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
    public IShellExtInit,
    public IShellPropSheetExt
{
public:
    CACDShellExt()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_ACDSHELLEXT)


BEGIN_COM_MAP(CACDShellExt)
    COM_INTERFACE_ENTRY(IACDShellExt)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
END_COM_MAP()


    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
	return S_OK;
    }
	
    void FinalRelease() 
    {
    }

public:
    /// IShellExtInit
    STDMETHOD (Initialize) (LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    /// IShellPropSheetExt
    STDMETHOD (AddPages) (LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHOD (ReplacePage) (UINT, LPFNADDPROPSHEETPAGE, LPARAM)
    {
	return E_NOTIMPL;
    }
};

OBJECT_ENTRY_AUTO(__uuidof(ACDShellExt), CACDShellExt)
