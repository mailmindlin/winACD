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

#include "ACDHidDev.h"

class CACDVirtualCP
{
public:

    /** The VCP array type. TODO: replace with MFC CArray */
    typedef CArray <CACDVirtualCP*> CVirtualCPArray;

private:

    /**
     * Enumerator helper struct (used by FindKnownVirtualCPs)
     */
    struct EnumHelper : CACDHidDevice::EnumHelperTmpl <CACDVirtualCP>
    {
	/** Convenience type to access the base class */
	typedef CACDHidDevice::EnumHelperTmpl <CACDVirtualCP> CBase;

	/** Helper member: the CPs array we are supposed to fill */
	CVirtualCPArray& m_Array;

	/** Constructor */
	EnumHelper (CVirtualCPArray& Array) : m_Array (Array) { }

	/* The enumerator callback */
	ENUMPROC_STATUS Callback (
	    IN CACDVirtualCP* pCP
	    );
    };

private:
    //
    // Device:
    //

    CACDHidDevice m_Device;	//!< The HID device.
    LPCSTR m_lpDeviceName;	//!< The USB device's name.
    LPCSTR m_lpDeviceID;	//!< Hardware instance id.

    //
    // Settings:
    //

    UCHAR m_bInitialBrightness;	//!< Initial brightness

    /** Current brightness. Might be different from the actual
     *  device's brightness (i.e. physical button pressed or another
     *  control program running). If we detect a change, take the new
     *  value as the Initial brightness (same effect as Apply).
     */
    UCHAR m_bBrightness;

    BOOL m_bModified;		//!< TRUE is the settings have been modified.

private:

    /** Initialize the current instance */
    void InitializeFeatures (void);

public:

    /** Constructor */
    CACDVirtualCP (
	HANDLE hDevice,
	PHIDP_PREPARSED_DATA pPpd = NULL
	) : m_Device (hDevice, pPpd),
	    m_lpDeviceName (NULL),
	    m_lpDeviceID (NULL),
	    m_bModified (FALSE)
    { }

    /** Destructor */
    ~CACDVirtualCP (void);

public:

    /** Find supported ACD HID monitor control panels. */
    static BOOL FindKnownVirtualCPs (CVirtualCPArray& VirtualCPArray)
    {
	EnumHelper helper (VirtualCPArray);
	return CACDHidDevice::EnumDevices (helper);
    }

    /** Return the HID Device. */
    CACDHidDevice& GetDevice ()
    {
	return m_Device;
    }

    /** Return the device's name. */
    LPCSTR GetDeviceName () const
    {
	return m_lpDeviceName;
    }

    /** Return the device's hardware instance ID. */
    LPCSTR GetDeviceID () const
    {
	return m_lpDeviceID;
    }

    /**
     * Return TRUE if the current hardware's brightness is the same
     * as the last we set with SetBrightness ().
     */
    BOOL CheckBrightness () const
    {
	return m_bBrightness == m_Device.GetBrightness ();
    }

    /** Return the current brightness (cached in m_bBrightness). */
    UCHAR GetBrightness ()
    {
	return m_bBrightness;
    }

    /** Set the current brightness. */
    void SetBrightness (UCHAR bBrightness)
    {
	m_bBrightness = bBrightness;
	m_Device.SetBrightness (bBrightness);
	m_bModified = TRUE;
    }

    /** Return TRUE if the settings have be modified, FALSE otherwise. */
    BOOL Modified () const
    {
	return m_bModified;
    }

    /** Apply the current settings. */
    void Apply ();

    /** Revert to the initial or last applied settings. */
    void Reset ();
};
