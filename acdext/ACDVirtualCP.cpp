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
#include "ACDVirtualCP.h"

#include <setupapi.h>
#include <cfgmgr32.h>

CACDVirtualCP::EnumHelper::ENUMPROC_STATUS
CACDVirtualCP::EnumHelper::Callback (IN CACDVirtualCP* pVCP)
{
    ENUMPROC_STATUS status = CBase::Callback (&pVCP->m_Device);

    if (status != ENUMPROC_STATUS_SUCCESS)
	return status;

    //
    // Find out the HID parents's name and hardware instance id.
    //

    SP_DEVINFO_DATA devInfo;
    devInfo.cbSize = sizeof (devInfo);
    CHAR buffer [128];

    if (!SetupDiEnumDeviceInfo (m_hDevInfo, m_dwCurrentIndex, &devInfo))
	return ENUMPROC_STATUS_CONTINUE;

    DEVINST devInst;
    CM_Get_Parent (&devInst, devInfo.DevInst, 0);

    _snprintf (buffer, sizeof (buffer), " (HID%d)", m_dwCurrentIndex);
    buffer [sizeof (buffer) - 1] = '\0';

    ULONG length = 0;
    CM_Get_DevNode_Registry_Property (
	devInst,	    /* devInst */
	CM_DRP_DEVICEDESC,  /* property */
	NULL,		    /* registry data type */
	NULL,		    /* buffer */
	&length,	    /* length */
	0		    /* flags */
	);
    
    length += (ULONG) strlen (buffer);
    LPSTR name = (LPSTR) malloc (length);
    if (!name)
	return ENUMPROC_STATUS_CONTINUE;

    if (CM_Get_DevNode_Registry_Property (
	devInst,	    /* devInst */
	CM_DRP_DEVICEDESC,  /* property */
	NULL,		    /* registry data type */
	name,		    /* buffer */
	&length,	    /* length */
	0		    /* flags */
	) != CR_SUCCESS)
	    return ENUMPROC_STATUS_CONTINUE;

    strcat (name, buffer);
    pVCP->m_lpDeviceName = name;

    if (CM_Get_Device_ID (
	devInfo.DevInst,    /* devInst */
	buffer,		    /* buffer */
	sizeof (buffer),    /* length */
	0		    /* flags */
	) != CR_SUCCESS)
	    return ENUMPROC_STATUS_CONTINUE;
    pVCP->m_lpDeviceID = strdup (buffer);

    pVCP->InitializeFeatures ();

    m_Array.Add (pVCP);

    return ENUMPROC_STATUS_SUCCESS;
}

CACDVirtualCP::~CACDVirtualCP (void)
{
    delete m_lpDeviceName;
    delete m_lpDeviceID;
}

void
CACDVirtualCP::InitializeFeatures (void)
{
    m_bBrightness = m_Device.GetBrightness ();
    m_bInitialBrightness = m_bBrightness;
}

void
CACDVirtualCP::Reset (void)
{
    if (CheckBrightness ())
	SetBrightness (m_bInitialBrightness);
    else
	InitializeFeatures ();

    m_bModified = FALSE;
}

void
CACDVirtualCP::Apply (void)
{
    if (CheckBrightness ())
	m_bInitialBrightness = m_bBrightness;
    else
	InitializeFeatures ();

    m_bModified = FALSE;
}