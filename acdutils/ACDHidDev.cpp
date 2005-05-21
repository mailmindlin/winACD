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

#include "ACDHidDev.h"
#include "ACDEdid.h"

#include <assert.h>

CUSBMonitorHidDevice::CUSBMonitorHidDevice (
    HANDLE hDevice,
    PHIDP_PREPARSED_DATA pPpd
    )
: m_hDevice (hDevice), m_pPpd (pPpd)
{
    NTSTATUS status;
    BOOL ret;

    if (pPpd == NULL) {
	ret = HidD_GetPreparsedData (hDevice, &m_pPpd);
	assert (ret);
    }

    ret = HidD_GetAttributes (hDevice, &m_Attributes);
    assert (ret);

    status = HidP_GetCaps (m_pPpd, &m_Caps);
    assert (status == HIDP_STATUS_SUCCESS);

    // TODO: initialize the inputs data.
    m_pInputValueCaps = NULL;
    m_pInputReportBuffer = NULL;

    // TODO: initialize the outputs data.
    m_pOutputValueCaps = NULL;
    m_pOutputReportBuffer = NULL;

    // TODO: initialize the features data
    m_pFeatureValueCaps = NULL;
    m_pFeatureReportBuffer = new CHAR [m_Caps.FeatureReportByteLength];
    assert (m_pFeatureReportBuffer != NULL);
}



CUSBMonitorHidDevice::~CUSBMonitorHidDevice ()
{
    // no need to check for nullness. delete (NULL) is a no-op 
    delete m_pInputValueCaps;
    delete m_pInputReportBuffer;
    delete m_pOutputValueCaps;
    delete m_pOutputReportBuffer;
    delete m_pFeatureValueCaps;
    delete m_pFeatureReportBuffer;

    HidD_FreePreparsedData (m_pPpd);

    // finally close the device
    CloseHandle (m_hDevice);
}

BOOL
CUSBMonitorHidDevice::SetFeatureValue (
    IN UsagePage Page,
    IN Usage Feature,
    IN ULONG lValue) const
{
    // we must zero the report buffer before using it.
    memset (m_pFeatureReportBuffer, '\0', m_Caps.FeatureReportByteLength);

    // prepare the report
    NTSTATUS status = HidP_SetUsageValue (
	HidP_Feature,	/* report type */
	Page,		/* usage page */
	0,		/* link collection */
	Feature,	/* usage */
	lValue,		/* usage value */
	m_pPpd,		/* preparsed data */
	m_pFeatureReportBuffer, /* report */
	(ULONG) m_Caps.FeatureReportByteLength /* report length */
	);

    if (status != HIDP_STATUS_SUCCESS)
	return FALSE;

    // send the report to the device
    return HidD_SetFeature (
	m_hDevice,
	m_pFeatureReportBuffer,
	(ULONG) m_Caps.FeatureReportByteLength
	);
}
 
BOOL
CUSBMonitorHidDevice::GetFeatureValue (
    IN UsagePage Page,
    IN Usage Feature,
    OUT PULONG pValue) const
{
    // we must zero the report buffer before using it.
    memset (m_pFeatureReportBuffer, '\0', m_Caps.FeatureReportByteLength);

    // FIXME_laurentm: should use value caps to insert the report id.
    m_pFeatureReportBuffer [0] = Feature;

    // get the report from the device.
    if (!HidD_GetFeature (
	m_hDevice,
	m_pFeatureReportBuffer,
	(ULONG) m_Caps.FeatureReportByteLength
	))
	return FALSE;

    // extract the usage value from the report.
    return HidP_GetUsageValue (
	HidP_Feature,	/* report type */
	Page,		/* usage page */
	0,		/* link collection */
	Feature,	/* usage */
	pValue,		/* usage value ptr */
	m_pPpd,		/* preparsed data */
	m_pFeatureReportBuffer, /* report */
	(ULONG) m_Caps.FeatureReportByteLength /* report length */
	) == HIDP_STATUS_SUCCESS;
}

BOOL
CUSBMonitorHidDevice::GetFeatureValueArray (
    IN UsagePage Page,
    IN Usage Feature,
    OUT PCHAR pValue,
    IN USHORT wLength
    ) const
{
    // we must zero the report buffer before using it.
    memset (m_pFeatureReportBuffer, '\0', m_Caps.FeatureReportByteLength);

    // FIXME_laurentm: should use value caps to insert the report id.
    m_pFeatureReportBuffer [0] = Feature;

    // get the report from the device.
    if (!HidD_GetFeature (
	m_hDevice,
	m_pFeatureReportBuffer,
	(ULONG) m_Caps.FeatureReportByteLength
	))
	return FALSE;

    // extract the usage value array from the report.
    return HidP_GetUsageValueArray (
	HidP_Feature,	/* report type */
	Page,		/* usage page */
	0,		/* link collection */
	Feature,	/* usage */
	pValue,		/* usage value ptr */
	wLength,		/* usage value byte length */
	m_pPpd,		/* preparsed data */
	m_pFeatureReportBuffer, /* report */
	(ULONG) m_Caps.FeatureReportByteLength /* report length */
	) == HIDP_STATUS_SUCCESS;
}

UCHAR
CACDHidDevice::GetFlags ()
{
    ULONG ulManagedPanel, ulManagedPower, ulButtonMask;

    GetFeatureValue (
	    USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
	    (CUSBMonitorHidDevice::Usage) ACD_USAGE_MANAGED_PANEL,
	    &ulManagedPanel
	    );
    GetFeatureValue (
	    USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
	    (CUSBMonitorHidDevice::Usage) ACD_USAGE_BUTTONS_MASK,
	    &ulButtonMask
	    );
    GetFeatureValue (
	    USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
	    (CUSBMonitorHidDevice::Usage) ACD_USAGE_MANAGED_POWER,
	    &ulManagedPower
	    );

    return (ulManagedPanel == 0 ? 0 : ACD_FLAGS_MANAGED_PANEL)
	| (ulManagedPower == 0 ? 0 : ACD_FLAGS_MANAGED_POWER)
	| (UCHAR) (ulButtonMask <<  ACD_FLAGS_BUTTONS_SHIFT
	   & ACD_FLAGS_BUTTONS_MASK_IN_PLACE);
}

void
CACDHidDevice::SetFlags (UCHAR bFlags)
{
    SetFeatureValue (
	USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
	(CUSBMonitorHidDevice::Usage) ACD_USAGE_MANAGED_PANEL,
	(ULONG) (bFlags & ACD_FLAGS_MANAGED_PANEL_MASK_IN_PLACE) != 0
	);
    SetFeatureValue (
	USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
	(CUSBMonitorHidDevice::Usage) ACD_USAGE_MANAGED_POWER,
	(ULONG) (bFlags & ACD_FLAGS_MANAGED_POWER_MASK_IN_PLACE) != 0
	);
    SetFeatureValue (
	USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
	(CUSBMonitorHidDevice::Usage) ACD_USAGE_BUTTONS_MASK,
	(ULONG) (bFlags & ACD_FLAGS_BUTTONS_MASK_IN_PLACE)
	    >> ACD_FLAGS_BUTTONS_SHIFT
	);
}