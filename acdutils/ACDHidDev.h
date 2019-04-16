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

#include "ACDEdid.h"

#include <wtypes.h>
#include <setupapi.h>

extern "C" {
#include <hidsdi.h>
}

class CUSBMonitorHidDevice {
public:

    /** Enumerator helper struct */
    template <class DeviceClass>
    struct EnumHelperTmpl {
		/** Helper device type */
		typedef DeviceClass CDevice;

		/** Enumeration iteration variables */
		HDEVINFO	m_hDevInfo;		//!< Device information set.
		DWORD		m_dwCurrentIndex;	//!< Device information index.
		HANDLE		m_hDevice;		//!< Driver handle.

			/** USB Monitor device enumeration status */
		enum ENUMPROC_STATUS {
			ENUMPROC_STATUS_SUCCESS	=  0,
			ENUMPROC_STATUS_CONTINUE	= -1,	//!< Continue to enumerate.
			ENUMPROC_STATUS_BREAK	= -2	//!< Stop enumerating!
		};

		/** Callback invoked after a valid hid device is found. */
		ENUMPROC_STATUS Callback (
				IN CDevice* Device
		) {
			return ENUMPROC_STATUS_SUCCESS;
		}
    };

    /** Convenience type for the EnumHelper */
    typedef EnumHelperTmpl<CUSBMonitorHidDevice> EnumHelper;

public:

    /** USB Monitor usage pages. */
    enum UsagePage {
		USAGE_PAGE_MONITOR               = 0x80,
		USAGE_PAGE_ENUMERATED_VALUES     = 0x81,
		USAGE_PAGE_VESA_VIRTUAL_CONTROLS = 0x82
    };

    enum Usage {
		/** Monitor usage page values. */
		USAGE_MONITOR_CONTROL    = 0x01,		//!< Monitor control Device.
		USAGE_EDID_INFO          = 0x02,		//!< EDID Information.
		USAGE_VDIF_INFO          = 0x03,		//!< VDIF Information.
		USAGE_VESA_VERSION       = 0x04,		//!< VESA command set version.

		/** VESA Virtual control page values. */
		USAGE_BRIGHTNESS         = 0x10,		//!< Brightness control usage.
    };

private:

    /** USB device data. */
    HANDLE			m_hDevice;	//!< Handle to the USB device.
    PHIDP_PREPARSED_DATA	m_pPpd;		//!< Opaque preparsed data.
    HIDD_ATTRIBUTES		m_Attributes;	//!< Device's attributes.
    HIDP_CAPS			m_Caps;		//!< Device's capabilities.

    /** Inputs data. */
    PHIDP_VALUE_CAPS	m_pInputValueCaps;	//!< Input capabilities.
    PCHAR		m_pInputReportBuffer;	//!< Input report buffer.

    /** Outputs data. */
    PHIDP_VALUE_CAPS	m_pOutputValueCaps;	//!< Output capabilities.
    PCHAR		m_pOutputReportBuffer;	//!< Output report buffer.

    /** Features data. */
    PHIDP_VALUE_CAPS	m_pFeatureValueCaps;	//!< Feature capabilities.
    PCHAR		m_pFeatureReportBuffer;	//!< Feature report buffer.

public:

    /** Contructor. */
    CUSBMonitorHidDevice (
			HANDLE hDevice,
			PHIDP_PREPARSED_DATA pPpd = NULL
	);
    /** Destructor. */
    ~CUSBMonitorHidDevice ();

private:

    /** Copy not allowed. */
    CUSBMonitorHidDevice (const CUSBMonitorHidDevice &);
    /** Copy not allowed. */
    void operator = (const CUSBMonitorHidDevice &);

protected:

    /** Set the value of a given feature. */
    BOOL SetFeatureValue (
			IN UsagePage Page,
			IN Usage Feature,
			IN ULONG lValue
	) const;

    /** Return the value of a given feature */
    BOOL GetFeatureValue (
			IN UsagePage Page,
			IN Usage Feature,
			OUT PULONG pValue
	) const;

    /** Return the value array of a given feature. */
    BOOL GetFeatureValueArray (
			IN UsagePage Page,
			IN Usage Feature,
			OUT PCHAR pValue,
			IN USHORT wLength
	) const;

public:

    /** Invoke the give callback func for every usb hid instance */
    template <class CHelper>
    static BOOL EnumDevices (
			IN CHelper&
	);

    /** Return the HID device handle */
    HANDLE GetHandle (void) const {
		return m_hDevice;
    }

    /** Return the manufacturer's product string */
    BOOLEAN GetProductString (
			OUT PVOID pBuffer,
			IN ULONG lLength
	) const {
		return HidD_GetProductString (m_hDevice, pBuffer, lLength);
    }

    /** Return the HID device's vendor ID */
    USHORT GetDeviceVendorID (void) const {
		return m_Attributes.VendorID;
    }

    /** Return the HID device's product ID */
    USHORT GetDeviceProductID (void) const {
		return m_Attributes.ProductID;
    }

    /** Return the HID device's revision number */
    USHORT GetDeviceVersionNumber (void) const {
		return m_Attributes.VersionNumber;
    }
};



class CACDHidDevice : public CUSBMonitorHidDevice {
protected:

    /** Apple's vendor id */
    static const USHORT APPLE_VENDORID = 0x05AC;

    /** Apple HID USB Monitor proprietary commands */
    enum ACDUsage {
		ACD_USAGE_POWERSTATE		= 0xD6, //!< Monitor power state
		ACD_USAGE_MANAGED_PANEL		= 0xE1, //!< Managed panel
		ACD_USAGE_UNKNOWN_1		= 0xE3, //!< Don't know yet!
		ACD_USAGE_UNKNOWN_2		= 0xE6, //!< Don't know yet!
		ACD_USAGE_BUTTONS_MASK		= 0xE7, //!< Panel buttons mask
		ACD_USAGE_MANAGED_POWER		= 0xE8  //!< Power button override
    };

    /** Supported cinema display product ids */
    enum ProductID {
		CLEAR_CINEMA_DISPLAY_20INCH_X		= 0x9212,
		CLEAR_CINEMA_DISPLAY_22INCH		= 0x9216,
		CLEAR_STUDIO_DISPLAY_17INCH		= 0x9217,
		CLEAR_CINEMA_DISPLAY_20INCH		= 0x9219,
		CLEAR_CINEMA_HD_DISPLAY_23INCH		= 0x9218,

		ALUMINUM_CINEMA_DISPLAY_20INCH		= 0x921D,
		ALUMINUM_CINEMA_DISPLAY_20INCH_X	= 0x9222,
		ALUMINUM_CINEMA_HD_DISPLAY_23INCH	= 0x921E,
		ALUMINUM_CINEMA_HD_DISPLAY_23INCH_X	= 0x921F,
		ALUMINUM_CINEMA_HD_DISPLAY_23INCH_Y	= 0x9223,
		ALUMINUM_CINEMA_HD_DISPLAY_30INCH	= 0x9220,
		ALUMINUM_CINEMA_HD_DISPLAY_30INCH_X	= 0x9232
    };

public:

    /** ACD flags */
    enum Flags {
		ACD_FLAGS_POWER_BUTTON			= 0x01,
		ACD_FLAGS_USER_ACTION_BUTTON		= 0x02,
		ACD_FLAGS_BRIGHTNESS_BUTTON		= 0x04,
		ACD_FLAGS_BUTTONS_MASK_IN_PLACE		= 0x07,
		ACD_FLAGS_BUTTONS_SHIFT			=    0,

		ACD_FLAGS_MANAGED_PANEL			= 0x08,
		ACD_FLAGS_MANAGED_PANEL_MASK_IN_PLACE	= 0x08,
		ACD_FLAGS_MANAGED_PANEL_SHIFT		=    3,

		ACD_FLAGS_MANAGED_POWER			= 0x10,
		ACD_FLAGS_MANAGED_POWER_MASK_IN_PLACE	= 0x10,
		ACD_FLAGS_MANAGED_POWER_SHIFT		=    4
    };

    /** Device enumerator helper struct */
    template <class DeviceClass>
    struct EnumHelperTmpl : CUSBMonitorHidDevice::EnumHelperTmpl<DeviceClass> {
		ENUMPROC_STATUS Callback (
				IN CACDHidDevice* pDevice
		) {
			// return success if the device is an Apple Cinema Display
			return pDevice->IsSupportedCinemaDisplay ()
					? ENUMPROC_STATUS_SUCCESS
					: ENUMPROC_STATUS_CONTINUE;
		}
    };
    typedef EnumHelperTmpl <CACDHidDevice> EnumHelper;

public:

    /** Contructor */
    CACDHidDevice (
			HANDLE hDevice,
			PHIDP_PREPARSED_DATA pPpd = NULL
	) : CUSBMonitorHidDevice (hDevice, pPpd)
    { }

    /** return TRUE if this is a clear cinema display */
    BOOL IsClearCinemaDisplay () const {
		switch (GetDeviceProductID ()) {
			/* Clear enclosure models: */
			case CLEAR_CINEMA_DISPLAY_22INCH:
			case CLEAR_STUDIO_DISPLAY_17INCH:
			case CLEAR_CINEMA_DISPLAY_20INCH:
			case CLEAR_CINEMA_DISPLAY_20INCH_X:
			case CLEAR_CINEMA_HD_DISPLAY_23INCH:
				return TRUE;
			default:
				return FALSE;
		}
    }

    /** return TRUE if this is an aluminum cinema display */
    BOOL IsAluminumCinemaDisplay() const {
		switch (GetDeviceProductID()) {
			/* All aluminum models: */
			case ALUMINUM_CINEMA_DISPLAY_20INCH:
			case ALUMINUM_CINEMA_DISPLAY_20INCH_X:
			case ALUMINUM_CINEMA_HD_DISPLAY_23INCH:
			case ALUMINUM_CINEMA_HD_DISPLAY_23INCH_X:
			case ALUMINUM_CINEMA_HD_DISPLAY_23INCH_Y:
			case ALUMINUM_CINEMA_HD_DISPLAY_30INCH:
			case ALUMINUM_CINEMA_HD_DISPLAY_30INCH_X:
				return TRUE;
			default:
				return FALSE;
		}
    }

    /** Return TRUE if this HidDevice is a supported Cinema Display model */
    BOOL IsSupportedCinemaDisplay() const {
		if (GetDeviceVendorID() != APPLE_VENDORID)
			return FALSE;

		return IsAluminumCinemaDisplay()
				|| IsClearCinemaDisplay();
    }

    /** Return the display's brightness */
    BOOL GetBrightness(PUCHAR pbBrightness) const {
		ULONG nBrightness;
		BOOL bRet = GetFeatureValue(
			USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
			USAGE_BRIGHTNESS,
			&nBrightness
			);
		if (bRet)
			*pbBrightness = (UCHAR) nBrightness;
		return bRet;
    }

    /** Set the display's brightness. */
    BOOL SetBrightness(
			UCHAR bBrightness
	) const {
		return SetFeatureValue (
			USAGE_PAGE_VESA_VIRTUAL_CONTROLS,
			USAGE_BRIGHTNESS,
			(ULONG) bBrightness
		);
    }

    /** Extract the diplay's EDID data */
    BOOL GetEDID(
			PEDID_STRUCT pEdid
	) const {
		return GetFeatureValueArray (
			USAGE_PAGE_MONITOR,
			USAGE_EDID_INFO,
			(PCHAR)pEdid,
			sizeof (EDID_STRUCT)
		);
    }

    /** Set the ACD flags */
    void SetFlags (UCHAR flags);

    /** Get the ACD flags */
    UCHAR GetFlags ();

};


template <class CHelper>
BOOL CUSBMonitorHidDevice::EnumDevices (
		IN CHelper& helper
) {
    // get the HIDClass guid.
    GUID hidGuid;
    HidD_GetHidGuid (&hidGuid);

    // get the hid PnP node.
    helper.m_hDevInfo = SetupDiGetClassDevs (
	&hidGuid,	/* class guid */
	NULL,		/* enumerator */
	NULL,		/* top-level parent wnd */
	DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
	);

    if (helper.m_hDevInfo == INVALID_HANDLE_VALUE)
		return FALSE;

    SP_DEVICE_INTERFACE_DATA interfaceData;
    interfaceData.cbSize = sizeof (interfaceData);

    for (helper.m_dwCurrentIndex = 0; SetupDiEnumDeviceInterfaces (
	helper.m_hDevInfo,
	NULL,
	&hidGuid,
	helper.m_dwCurrentIndex,
	&interfaceData
	); ++helper.m_dwCurrentIndex) {

		//
		// Get the interface detail data.
		//

		DWORD idSize;
		PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData;

		// allocate the interface detail data struct.
		SetupDiGetDeviceInterfaceDetail (
				helper.m_hDevInfo,
				&interfaceData,
				NULL, 0, &idSize, NULL);
		pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(idSize);
		if (pInterfaceDetailData == NULL) {
			// can't do much after this kind of error
			SetupDiDestroyDeviceInfoList(helper.m_hDevInfo);
			return FALSE;
		}

		// retrieve the detail data.
		pInterfaceDetailData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail (
			helper.m_hDevInfo,	/* device info set */
			&interfaceData,		/* device interface data */
			pInterfaceDetailData,	/* device interface detail data */
			idSize,			/* device interface detail data size */
			NULL,			/* required size */
			NULL			/* device info data */
			)) {
			free (pInterfaceDetailData);
			continue;
		}

		//
		// Open the driver.
		//

		HANDLE hDevice = CreateFile (
			pInterfaceDetailData->DevicePath,	/* filename */
			GENERIC_READ | GENERIC_WRITE,	/* desired access */
			FILE_SHARE_READ | FILE_SHARE_WRITE,	/* share mode */
			NULL,				/* security attributes */
			OPEN_EXISTING,			/* creation disposition */
			0,					/* flags and attributes */
			NULL				/* template file */
			);

		free (pInterfaceDetailData);

		if (hDevice == INVALID_HANDLE_VALUE)
			continue;

		//
		// Check the collection USAGE_PAGE (must be monitor)
		// 

		PHIDP_PREPARSED_DATA ppData;
		if (!HidD_GetPreparsedData(hDevice, &ppData)) {
			CloseHandle (hDevice);
			continue;
		}

		HIDP_CAPS caps;
		NTSTATUS hidStatus = HidP_GetCaps(ppData, &caps);
		if (hidStatus != HIDP_STATUS_SUCCESS
				|| caps.UsagePage != USAGE_PAGE_MONITOR) {
			HidD_FreePreparsedData(ppData);
			CloseHandle(hDevice);
			continue;
		}

		//
		// we have a usb monitor hid device!
		//

		CHelper::CDevice *pDevice = new CHelper::CDevice(hDevice, ppData);
		if (pDevice == NULL) {
			HidD_FreePreparsedData (ppData);
			CloseHandle (hDevice);
			SetupDiDestroyDeviceInfoList (helper.m_hDevInfo);
			return FALSE;
		}

		// invoke the helper callback proc
		helper.m_hDevice = hDevice;
		CHelper::ENUMPROC_STATUS procStatus = helper.Callback (pDevice);

		if (procStatus != CHelper::ENUMPROC_STATUS_SUCCESS) {
			delete pDevice;
			if (procStatus == CHelper::ENUMPROC_STATUS_BREAK)
				break;
		}
    }
    SetupDiDestroyDeviceInfoList (helper.m_hDevInfo);

    return TRUE;
}


/**
 * Bezel button codes.
 */
enum BezelButtonCode {
    ACD_BUTTON_BRIGHTNESS_UP	= 0x03,
    ACD_BUTTON_BRIGHTNESS_DOWN	= 0x04,
    ACD_BUTTON_USER_ACTION	= 0x05,
    ACD_BUTTON_POWER		= 0x0A
};

/**
 * Power button action.
 */
enum ACDPowerButtonAction {
    ACD_POWER_BUTTON_DO_NOTHING = 0,
    ACD_POWER_BUTTON_STAND_BY,
    ACD_POWER_BUTTON_HIBERNATE,
    ACD_POWER_BUTTON_SHUT_DOWN
};