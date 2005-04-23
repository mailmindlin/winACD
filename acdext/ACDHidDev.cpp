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

#include "StdAfx.h"
#include "ACDHidDev.h"

#include <assert.h>

BOOL
CACDHidDevice::IsACDHidDevice (HANDLE device)
{
    PHIDP_PREPARSED_DATA ppData;

    if (!HidD_GetPreparsedData (device, &ppData))
	return FALSE;

    HIDD_ATTRIBUTES attributes;
    if (!HidD_GetAttributes (device, &attributes)) {
	HidD_FreePreparsedData (ppData);
	return FALSE;
    }

    HIDP_CAPS capabilities;
    if (!HidP_GetCaps (ppData, &capabilities)) {
	HidD_FreePreparsedData (ppData);
	return FALSE;
    }

    BOOL isAcd;
    if (capabilities.UsagePage != ACD_USAGE_PAGE_MONITOR
	|| attributes.VendorID != APPLE_VENDORID)
	isAcd = FALSE;
    else switch (attributes.ProductID) {
	// currently supported models
    case ALUMINIUM_CINEMA_DISPLAY_20INCH:
    case ALUMINIUM_CINEMA_HD_DISPLAY_23INCH:
    case ALUMINIUM_CINEMA_HD_DISPLAY_30INCH:
	isAcd = TRUE;
	break;
	// everything else
    default:
	isAcd = FALSE;
	break;
    }

    HidD_FreePreparsedData (ppData);
    return isAcd;
}


CACDHidDevice::CACDHidDevice (HANDLE device)
: m_Device (device)
{
    BOOL status;

    status = HidD_GetPreparsedData (device, &m_Ppd);
    assert (status);

    status = HidD_GetAttributes (device, &m_Attributes);
    assert (status);

    status = HidP_GetCaps (m_Ppd, &m_Caps);
    assert (status);

    m_FeatureReportBuffer = new CHAR [m_Caps.FeatureReportByteLength];
    assert (m_FeatureReportBuffer != NULL);
}

CACDHidDevice::~CACDHidDevice ()
{
    delete[] m_FeatureReportBuffer;
    HidD_FreePreparsedData (m_Ppd);
}


void
CACDHidDevice::SetFeatureValue (Usage feature, UCHAR value)
{
    // we must zero the report buffer before using it.
    memset (m_FeatureReportBuffer, '\0', m_Caps.FeatureReportByteLength);

    // prepare the report
    NTSTATUS status;
    status = HidP_SetUsageValue (
	HidP_Feature, ACD_USAGE_PAGE_VESA_CONTROLS, 0,
	feature, value, m_Ppd, m_FeatureReportBuffer,
	(ULONG) m_Caps.FeatureReportByteLength
	);
    assert (status == HIDP_STATUS_SUCCESS);

    // send the report to the device
    BOOL result;
    result = HidD_SetFeature (
	m_Device, m_FeatureReportBuffer,
	(ULONG) m_Caps.FeatureReportByteLength
	);
    assert (result);
}
 
ULONG
CACDHidDevice::GetFeatureValue (Usage feature)
{
    ULONG value = 0;

    // we must zero the report buffer before using it.
    memset (m_FeatureReportBuffer, '\0', m_Caps.FeatureReportByteLength);
    m_FeatureReportBuffer [0] = feature;

    // get the report from the device.
    BOOL result;
    result = HidD_GetFeature(
	m_Device, m_FeatureReportBuffer,
	(ULONG) m_Caps.FeatureReportByteLength
	);
    assert (result);

    // get the usage value from the report.
    NTSTATUS status;
    status = HidP_GetUsageValue(
	HidP_Feature, ACD_USAGE_PAGE_VESA_CONTROLS, 0,
	feature, &value,  m_Ppd, m_FeatureReportBuffer,
	(ULONG) m_Caps.FeatureReportByteLength
	);

    assert (status == HIDP_STATUS_SUCCESS);

    return value;
}
