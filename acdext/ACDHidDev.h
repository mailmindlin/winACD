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

extern "C" {
#include <hidsdi.h>
}

class CACDHidDevice
{
private:

    static const UCHAR ACD_USAGE_PAGE_MONITOR	    = 0x80;
    static const UCHAR ACD_USAGE_PAGE_VESA_CONTROLS = 0x82;

    /// Apple's vendor id
    static const USHORT APPLE_VENDORID = 0x05AC;

    /// ACD product id
    enum ProductID {
	ALUMINIUM_CINEMA_DISPLAY_20INCH		= 0x921D,
	ALUMINIUM_CINEMA_HD_DISPLAY_23INCH	= 0x921E,
	ALUMINIUM_CINEMA_HD_DISPLAY_30INCH	= 0x9220
    };

    /// ACD hid usage opcodes.
    enum Usage {
	ACD_USAGE_EDID		    = 0x02,
	ACD_USAGE_BRIGHTNESS	    = 0x10,
	ACD_USAGE_POWERSTATE	    = 0xD6,
	ACD_USAGE_MANAGE_BUTTONS    = 0xE1,
	ACD_USAGE_UNKNOWN	    = 0xE3,
	ACD_USAGE_BUTTONMASK	    = 0xE7,
	ACD_USAGE_MANAGE_POWER	    = 0xE8
    };

public:

    /// ACD physical panel buttons.
    enum Button {
	ACD_BUTTON_POWER	= 0x1,
	ACD_BUTTON_BRIGHTNESS	= 0x2
    };

private:

    HANDLE			m_Device;	//!< Handle to the ACD device.
    PHIDP_PREPARSED_DATA	m_Ppd;		//!< Opaque preparsed data.
    HIDP_CAPS			m_Caps;		//!< Device's capabilities.
    HIDD_ATTRIBUTES		m_Attributes;	//!< Device's attributes.

    /// Feature value capabilities.
    PHIDP_VALUE_CAPS	m_FeatureValueCaps;
    /// Feature report buffer (of m_Caps.FeatureReportByteLength bytes)
    PCHAR		m_FeatureReportBuffer;

protected:

    /// Return the given feature's value.
    ULONG GetFeatureValue (Usage feature) const;
    /// Set the given feature value.
    void SetFeatureValue (Usage feature, UCHAR value);

public:

    /// Return true is 'device' is an Apple Cinema Display hid device.
    static BOOL IsACDHidDevice (HANDLE device);

public:

    CACDHidDevice (HANDLE device);
    ~CACDHidDevice ();

    HANDLE DeviceHandle (void) const
    {
	return m_Device;
    }

    /// Return the current brightness.
    UCHAR GetBrightness (void) const
    {
	return (UCHAR) GetFeatureValue (ACD_USAGE_BRIGHTNESS);
    }

    /// Set the display's brightness.
    void SetBrightness (UCHAR brightness)
    {
	SetFeatureValue (ACD_USAGE_BRIGHTNESS, brightness);
    }
};
