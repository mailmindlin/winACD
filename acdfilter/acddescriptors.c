/* Copyright (C) 2005 Laurent Morichetti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/**
 * acddescriptors.c: Apple HID Monitor Controls USB descriptors.
 */
#if !defined _ACD_FILTER_C
# error "Must be included by acdfilter.c"
#endif /* !defined _ACD_FILTER_C */

static UCHAR ACD_9216_HidReportDescriptor [] = {
	0x05, 0x80,     // USAGE_PAGE (Monitor)
	0x09, 0x01,     // USAGE (Monitor Control)
	0xA1, 0x01,     // COLLECTION (Application)
	0x15, 0x00,     // LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x00,	/*   LOGICAL_MAXIMUM (255)		*/
	0x75, 0x08,		/*   REPORT_SIZE (8)			*/
	0x85, 0x02,		/*   REPORT_ID (2)			*/
	0x96, 0x00, 0x01,	/*   REPORT_COUNT (256)			*/
	0x09, 0x02,		/*   USAGE (EDID Information)		*/
	0xB2, 0x02, 0x01,	/*   FEATURE (Data,Var,Abs,Buf)		*/
	0x05, 0x82,		/*   USAGE_PAGE (VESA Virtual Controls)	*/
	0x95, 0x01,		/*   REPORT_COUNT (1)			*/
	0x85, 0x10,		/*   REPORT_ID (16)			*/
	0x09, 0x10,		/*   USAGE (Brightness)			*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x25, 0x04,		/*   LOGICAL_MAXIMUM (4)		*/
	0x85, 0xD6,		/*   REPORT_ID (214)			*/
	0x09, 0xD6,		/*   USAGE (214)			*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x85, 0xE4,		/*   REPORT_ID (230)			*/
	0x09, 0xE4,		/*   USAGE (230)			*/
	0x81, 0x02,		/*   INPUT (Data,Var,Abs)		*/
	0xC0		/* END_COLLECTION			*/
};

/**
 * New configuration descriptor (since we changed the HID report size)
 */
static ACD_CONFIGURATION_DESCRIPTOR ACD_9216_ConfigurationDescriptor = {
	/* USB Configuration descriptor */
	.Configuration0 = {
		.bLength = sizeof (USB_CONFIGURATION_DESCRIPTOR),	/* bLength		*/
		.bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType	*/
		.wTotalLength = sizeof (ACD_CONFIGURATION_DESCRIPTOR),	/* wTotalLength		*/
		.bNumInterfaces = 1,					/* bNumInterfaces	*/
		.bConfigurationValue = 1,					/* bConfigurationValue	*/
		.iConfiguration = 0,                    // iConfiguration
		.bmAttributes = 0xE0,                   // bmAttributes
		.MaxPower = 1                           // MaxPower
	},
	/* USB Interface descriptor */
	.Interface0 = {
		.bLength = sizeof (USB_INTERFACE_DESCRIPTOR),	/* bLength		*/
		.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bInterfaceNumber = 0,					/* bInterfaceNumber	*/
		.bAlternateSetting = 0,					/* bAlternateSetting	*/
		.bNumEndpoints = 1,					/* bNumEndPoints	*/
		.bInterfaceClass = 3,					/* bInterfaceClass	*/
		.bInterfaceSubClass = 0,					/* bInterfaceSubClass	*/
		.bInterfaceProtocol = 0,					/* bInterfaceProtocol	*/
		.iInterface = 0					/* iInterface		*/
	},
	/* HID Descriptor */
	.Hid0 = { 
		.bLength = sizeof (HID_DESCRIPTOR),		/* bLength		*/
		.bDescriptorType = HID_HID_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bcdHID = 0x111,					/* bcdHID		*/
		.bCountry = 0,					/* bCountry		*/
		.bNumDescriptors = 1,					/* bNumDescriptors	*/
		{{
			.bReportType = HID_REPORT_DESCRIPTOR_TYPE,		/* bReportType		*/
			.wReportLength = sizeof(ACD_9216_HidReportDescriptor) /* wReportLength	*/
		}}
	},
	/* Endpoint 0x81 - Interrupt Input */
	.EndPoint1 = {
		.bLength = sizeof (USB_ENDPOINT_DESCRIPTOR),        // bLength		*/
		.bDescriptorType = USB_ENDPOINT_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bEndpointAddress = 0x81,					/* bEndPointAddress	*/
		.bmAttributes = USB_ENDPOINT_TYPE_INTERRUPT,		// bmAttributes
		.wMaxPacketSize = 8,                                // wMaxPacketSize
		.bInterval = 16                                     // bInterval
	}		
};

/**
 * New HID report descriptor (clear enclosure). We only have to change the
 * report count sizes (should not include the report_id byte), everything else
 * looks ok.
 */
static UCHAR
ACD_Clear_HidReportDescriptor [] = {
	0x05, 0x80,		/* USAGE_PAGE (Monitor)			*/
	0x09, 0x01,		/* USAGE (Monitor Control)		*/
	0xA1, 0x01,		/* COLLECTION (Application)		*/
	0x15, 0x00,		/*   LOGICAL_MINIMUM (0)		*/
	0x26, 0xFF, 0x00,	/*   LOGICAL_MAXIMUM (255)		*/
	0x75, 0x08,		/*   REPORT_SIZE (8)			*/
	0x85, 0x02,		/*   REPORT_ID (2)			*/
	0x96, 0x00, 0x01,	/*   REPORT_COUNT (256)			*/
	0x09, 0x02,		/*   USAGE (EDID Information)		*/
	0xB2, 0x02, 0x01,	/*   FEATURE (Data,Var,Abs,Buf)		*/
	0x05, 0x82,		/*   USAGE_PAGE (VESA Virtual Controls)	*/
	0x95, 0x01,		/*   REPORT_COUNT (1)			*/
	0x85, 0x10,		/*   REPORT_ID (16)			*/
	0x09, 0x10,		/*   USAGE (Brightness)			*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x25, 0x04,		/*   LOGICAL_MAXIMUM (4)		*/
	0x85, 0xD6,		/*   REPORT_ID (214)			*/
	0x09, 0xD6,		/*   USAGE (214)			*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x25, 0x01,		/*   LOGICAL_MAXIMUM (1)		*/
	0x85, 0xE6,		/*   REPORT_ID (230)			*/
	0x09, 0xE6,		/*   USAGE (230)			*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x25, 0x03,		/*   LOGICAL_MAXIMUM (3)		*/
	0x85, 0xE7,		/*   REPORT_ID (231)			*/
	0x09, 0xE7,		/*   USAGE (231)		<- (*)	*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x26, 0xFF, 0x00,	/*   LOGICAL_MAXIMUM (255)		*/
	0x85, 0xE4,		/*   REPORT_ID (228)			*/
	0x09, 0xE4,		/*   USAGE (228)		<- (*)	*/
	0x81, 0x02,		/*   INPUT (Data,Var,Abs)		*/
	0xC0		/* END_COLLECTION			*/
};

/**
 * New configuration descriptor (since we changed the HID report size)
 */
static ACD_CONFIGURATION_DESCRIPTOR
ACD_Clear_ConfigurationDescriptor = {
	/* USB Configuration descriptor */
	.Configuration0 = {
		.bLength = sizeof (USB_CONFIGURATION_DESCRIPTOR),	/* bLength		*/
		.bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType	*/
		.wTotalLength = sizeof (ACD_CONFIGURATION_DESCRIPTOR),	/* wTotalLength		*/
		.bNumInterfaces = 1,					/* bNumInterfaces	*/
		.bConfigurationValue = 1,					/* bConfigurationValue	*/
		.iConfiguration = 0,					/* iConfiguration	*/
		.bmAttributes = 0xE0,					/* bmAttributes		*/
		.MaxPower = 1					/* MaxPower		*/
	},
	/* USB Interface descriptor */
	.Interface0 = {
		.bLength = sizeof (USB_INTERFACE_DESCRIPTOR),	/* bLength		*/
		.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bInterfaceNumber = 0,					/* bInterfaceNumber	*/
		.bAlternateSetting = 0,					/* bAlternateSetting	*/
		.bNumEndpoints = 1,					/* bNumEndPoints	*/
		.bInterfaceClass = 3,					/* bInterfaceClass	*/
		.bInterfaceSubClass = 0,					/* bInterfaceSubClass	*/
		.bInterfaceProtocol = 0,					/* bInterfaceProtocol	*/
		.iInterface = 0					/* iInterface		*/
	},
	/* HID Descriptor */
	.Hid0 = {
		.bLength = sizeof (HID_DESCRIPTOR),		/* bLength		*/
		.bDescriptorType = HID_HID_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bcdHID = 0x111,					/* bcdHID		*/
		.bCountry = 0,					/* bCountry		*/
		.bNumDescriptors = 1,					/* bNumDescriptors	*/
		{{
			.bReportType = HID_REPORT_DESCRIPTOR_TYPE,
			.wReportLength = sizeof (ACD_Clear_HidReportDescriptor)
		}}
	},
	/* Endpoint 0x81 - Interrupt Input */
	.EndPoint1 = {
		.bLength = sizeof (USB_ENDPOINT_DESCRIPTOR),
		.bDescriptorType = USB_ENDPOINT_DESCRIPTOR_TYPE,
		.bEndpointAddress = 0x81,
		.bmAttributes = USB_ENDPOINT_TYPE_INTERRUPT,
		.wMaxPacketSize = 8,
		.bInterval = 16
	}
};

/**
 * New HID report descriptor (aluminum). We changed the report count sizes,
 * added the missing usage tags and declared the hidden 0xe1, 0xe3 and 0xe8
 * features.
 */
static UCHAR
ACD_Aluminum_HidReportDescriptor [] = {
	0x05, 0x80,		/* USAGE_PAGE (Monitor)			*/
	0x09, 0x01,		/* USAGE (Monitor Control)		*/
	0xA1, 0x01,		/* COLLECTION (Application)		*/
	0x15, 0x00,		/*   LOGICAL_MINIMUM (0)		*/
	0x26, 0xFF, 0x00,	/*   LOGICAL_MAXIMUM (255)		*/
	0x75, 0x08,		/*   REPORT_SIZE (8)			*/
	0x85, 0x02,		/*   REPORT_ID (2)			*/
	0x96, 0x00, 0x01,	/*   REPORT_COUNT (256)			*/
	0x09, 0x02,		/*   USAGE (EDID Information)		*/
	0xB2, 0x02, 0x01,	/*   FEATURE (Data,Var,Abs,Buf)		*/
	0x05, 0x82,		/*   USAGE_PAGE (VESA Virtual Controls)	*/
	0x95, 0x01,		/*   REPORT_COUNT (1)			*/
	0x85, 0x10,		/*   REPORT_ID (16)			*/
	0x09, 0x10,		/*   USAGE (Brightness)			*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x85, 0xE3,		/*   REPORT_ID (227)		<- (x)	*/
	0x09, 0xE3,		/*   USAGE (227)		<- (x)	*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)	<- (x)	*/

	0x25, 0x01,		/*   LOGICAL_MAXIMUM (1)	<- (x)	*/
	0x85, 0xE1,		/*   REPORT_ID (0x225)		<- (x)	*/
	0x09, 0xE1,		/*   USAGE (225)		<- (x)	*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)	<- (x)	*/
	0x85, 0xE8,		/*   REPORT_ID (232)		<- (x)	*/
	0x09, 0xE8,		/*   USAGE (232)		<- (x)	*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)	<- (x)	*/

	0x25, 0x04,		/*   LOGICAL_MAXIMUM (4)		*/
	0x85, 0xD6,		/*   REPORT_ID (214)			*/
	0x09, 0xD6,		/*   USAGE (214)			*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/

	0x25, 0x07,		/*   LOGICAL_MAXIMUM (7)		*/
	0x85, 0xE7,		/*   REPORT_ID (231)			*/
	0x09, 0xE7,		/*   USAGE (231)		<- (*)	*/
	0xB1, 0x02,		/*   FEATURE (Data,Var,Abs)		*/
	0x26, 0xFF, 0x00,	/*   LOGICAL_MAXIMUM (255)		*/
	0x85, 0xE4,		/*   REPORT_ID (228)			*/
	0x09, 0xE4,		/*   USAGE (228)		<- (*)	*/
	0x81, 0x02,		/*   INPUT (Data,Var,Abs)		*/
	0xC0		/* END_COLLECTION			*/
};
			/* (*) missing USAGE tags		*/
			/* (x) added reports			*/

/**
 * New configuration descriptor (since we changed the HID report size)
 */
static ACD_CONFIGURATION_DESCRIPTOR
ACD_Aluminum_ConfigurationDescriptor = {
	/* USB Configuration descriptor */
	.Configuration0 = {
		.bLength = sizeof (USB_CONFIGURATION_DESCRIPTOR),	/* bLength		*/
		.bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType	*/
		.wTotalLength = sizeof (ACD_CONFIGURATION_DESCRIPTOR),	/* wTotalLength		*/
		.bNumInterfaces = 1,					/* bNumInterfaces	*/
		.bConfigurationValue = 1,					/* bConfigurationValue	*/
		.iConfiguration = 0,					/* iConfiguration	*/
		.bmAttributes = 0xE0,					/* bmAttributes		*/
		.MaxPower = 1					/* MaxPower		*/
	},
	/* USB Interface descriptor */
	.Interface0 = {
		.bLength = sizeof (USB_INTERFACE_DESCRIPTOR),	/* bLength		*/
		.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bInterfaceNumber = 0,					/* bInterfaceNumber	*/
		.bAlternateSetting = 0,					/* bAlternateSetting	*/
		.bNumEndpoints = 1,					/* bNumEndPoints	*/
		.bInterfaceClass = 3,					/* bInterfaceClass	*/
		.bInterfaceSubClass = 0,					/* bInterfaceSubClass	*/
		.bInterfaceProtocol = 0,					/* bInterfaceProtocol	*/
		.iInterface = 0					/* iInterface		*/
	},
	/* HID Descriptor */
	.Hid0 = {
		.bLength = sizeof (HID_DESCRIPTOR),		/* bLength		*/
		.bDescriptorType = HID_HID_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bcdHID = 0x111,					/* bcdHID		*/
		.bCountry = 0,					/* bCountry		*/
		.bNumDescriptors = 1,					/* bNumDescriptors	*/
		{{
			.bReportType = HID_REPORT_DESCRIPTOR_TYPE,		/* bReportType		*/
			.wReportLength = sizeof (ACD_Aluminum_HidReportDescriptor) /* wReportLength	*/
		}}
	},
	/* Endpoint 0x81 - Interrupt Input */
	.EndPoint1 = {
		.bLength = sizeof(USB_ENDPOINT_DESCRIPTOR),        // bLength		*/
		.bDescriptorType = USB_ENDPOINT_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
		.bEndpointAddress = 0x81,					/* bEndPointAddress	*/
		.bmAttributes = USB_ENDPOINT_TYPE_INTERRUPT,		// bmAttributes
		.wMaxPacketSize = 8,                                // wMaxPacketSize
		.bInterval = 16                                     // bInterval
	}
};
