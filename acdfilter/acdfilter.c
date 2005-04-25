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
 * acdfilter.c: Apple HID Monitor Controls WDM filter driver.
 */

#include <wdm.h>
#include <usb.h>
#include <usbioctl.h>
#include <hidport.h>

#include "acdfilter.h"

/**
 * New HID report descriptor.
 *
 * TODO: fixup the bitfield usages (report size=1, report count=x and add
 *	 constant padding)
 */
UCHAR
ACD_HidReportDescriptor [] = {
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
	0xC0			/* END_COLLECTION			*/
};
				/* (*) missing USAGE tags		*/
				/* (x) added reports			*/

/**
 * New configuration descriptor (since we changed the HID report size)
 */
ACD_CONFIGURATION_DESCRIPTOR
ACD_ConfigurationDescriptor = {
    { /* USB Configuration descriptor */
	sizeof (USB_CONFIGURATION_DESCRIPTOR),	/* bLength		*/
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType	*/
	sizeof (ACD_CONFIGURATION_DESCRIPTOR),	/* wTotalLength		*/
	1,					/* bNumInterfaces	*/
	1,					/* bConfigurationValue	*/
	0,					/* iConfiguration	*/
	0xE0,					/* bmAttributes		*/
	1					/* MaxPower		*/
    },
    { /* USB Interface descriptor */
	sizeof (USB_INTERFACE_DESCRIPTOR),	/* bLength		*/
	USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
	0,					/* bInterfaceNumber	*/
	0,					/* bAlternateSetting	*/
	1,					/* bNumEndPoints	*/
	3,					/* bInterfaceClass	*/
	0,					/* bInterfaceSubClass	*/
	0,					/* bInterfaceProtocol	*/
	0					/* iInterface		*/
    },
    { /* HID Descriptor */
	sizeof (HID_DESCRIPTOR),		/* bLength		*/
	HID_HID_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
	0x111,					/* bcdHID		*/
	0,					/* bCountry		*/
	1,					/* bNumDescriptors	*/
	{
	    HID_REPORT_DESCRIPTOR_TYPE,		/* bReportType		*/
	    sizeof (ACD_HidReportDescriptor)	/* wReportLength	*/
	}
    },
    { /* Endpoint 0x81 - Interrupt Input */
	sizeof (USB_ENDPOINT_DESCRIPTOR),	/* bLength		*/
	USB_ENDPOINT_DESCRIPTOR_TYPE,		/* bDescriptorType	*/
	0x81,					/* bEndPointAddress	*/
	USB_ENDPOINT_TYPE_INTERRUPT,		/* bmAttributes		*/
	8,					/* wMaxPacketSize	*/
	16					/* bInterval		*/
    }		
};

/**
 * DriverEntry: Driver initialization entry point.
 */
NTSTATUS
DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    ULONG i;

    PAGED_CODE ();
    UNREFERENCED_PARAMETER (RegistryPath);

    ACD_dbgPrint (("DriverEntry"));

    /* initialize our function pointers. */
    DriverObject->DriverExtension->AddDevice = ACD_AddDevice;
    DriverObject->DriverUnload = ACD_DriverUnload;

    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
	DriverObject->MajorFunction [i] = ACD_DispatchAny;
    }
    /* the WDM requires us to have specific PnP & Power dispatch routines */
    DriverObject->MajorFunction [IRP_MJ_POWER] = ACD_DispatchPower;
    DriverObject->MajorFunction [IRP_MJ_PNP] = ACD_DispatchPnP;

    /* we need to filter the hid report descriptor */
    DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] =
	ACD_DispatchIoctl;

    /* that's it! */
    return STATUS_SUCCESS;
}

/**
 * ACD_AddDevice: Create, initialize and attach a new filter device object.
 */
NTSTATUS
ACD_AddDevice (IN PDRIVER_OBJECT DriverObject,
	       IN PDEVICE_OBJECT PhysicalDeviceObject)
{
    PDEVICE_OBJECT filterDeviceObject = NULL;
    PDEVICE_OBJECT lowerDeviceObject;
    PDEVICE_EXTENSION deviceExt;
    NTSTATUS status;

    ACD_dbgPrint (("AddDevice"));

    /* create the filter device object to attach to the stack */
    status = IoCreateDevice (
	DriverObject,
	sizeof (DEVICE_EXTENSION),
	NULL,
	FILE_DEVICE_UNKNOWN,
	FILE_AUTOGENERATED_DEVICE_NAME,
	FALSE,
	&filterDeviceObject
	);

    if (!NT_SUCCESS (status))
	return status;

    /* initialize the filter device extension */
    deviceExt = (PDEVICE_EXTENSION) filterDeviceObject->DeviceExtension;
    RtlZeroMemory (deviceExt, sizeof (DEVICE_EXTENSION));
    deviceExt->physicalDeviceObject = PhysicalDeviceObject;

    /* attach our new filter device to the device stack */
    IoInitializeRemoveLock (&deviceExt->removeLock, 0, 0, 255);
    lowerDeviceObject = IoAttachDeviceToDeviceStack (
	filterDeviceObject, PhysicalDeviceObject
	);
    ASSERT (lowerDeviceObject != NULL);
    deviceExt->lowerDeviceObject = lowerDeviceObject;

    /* maintain IO/POWER related flags */
    filterDeviceObject->Flags |= lowerDeviceObject->Flags
	& (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE | DO_POWER_INRUSH);

    /* maintain the lower device's characteristics */
    filterDeviceObject->DeviceType = lowerDeviceObject->DeviceType;
    filterDeviceObject->Characteristics = lowerDeviceObject->Characteristics;
    filterDeviceObject->AlignmentRequirement =
	lowerDeviceObject->AlignmentRequirement;

    /* we can now receive IRPs */
    filterDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}

VOID
ACD_DriverUnload (IN PDRIVER_OBJECT DriverObject)
{
    ACD_dbgPrint (("DriverUnload"));
}

NTSTATUS
ACD_CompleteRequest (IN PIRP Irp, IN NTSTATUS Status, IN ULONG Info)
{
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = Info;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return Status;
}

NTSTATUS
ACD_DispatchAny (IN PDEVICE_OBJECT FilterDeviceObject, IN PIRP Irp)
{
    PDEVICE_EXTENSION deviceExt;
    NTSTATUS status;

    deviceExt = (PDEVICE_EXTENSION) FilterDeviceObject->DeviceExtension;
    status = IoAcquireRemoveLock (&deviceExt->removeLock, Irp);
    if (!NT_SUCCESS (status))
	return ACD_CompleteRequest (Irp, status, 0);

    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (deviceExt->lowerDeviceObject, Irp);
    IoReleaseRemoveLock (&deviceExt->removeLock, Irp);

    return status;
}

NTSTATUS
ACD_DispatchIoctl (IN PDEVICE_OBJECT FilterDeviceObject, IN PIRP Irp)
{
    BOOLEAN completeRequestHere = FALSE;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExt;
    ULONG cCode;
    NTSTATUS status;

    deviceExt = (PDEVICE_EXTENSION) FilterDeviceObject->DeviceExtension;
    status = IoAcquireRemoveLock (&deviceExt->removeLock, Irp);
    if (!NT_SUCCESS (status))
	return ACD_CompleteRequest (Irp, status, 0);

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    cCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

    if (cCode == IOCTL_INTERNAL_USB_SUBMIT_URB) {
	PURB urb = (PURB) irpStack->Parameters.Others.Argument1;
	struct _URB_CONTROL_DESCRIPTOR_REQUEST* desc =
	    (struct _URB_CONTROL_DESCRIPTOR_REQUEST *) urb;
	USHORT function = urb->UrbHeader.Function;

	if (function == URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE
	    && desc->DescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE) {
	    /* return our own device configuration descriptor */
	    status = ACD_FillControlDescriptorRequest (
		urb, (PUCHAR) &ACD_ConfigurationDescriptor,
		sizeof (ACD_ConfigurationDescriptor)
		);
	    completeRequestHere = TRUE;
	}
	else if (function == URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE
		 && desc->DescriptorType == HID_REPORT_DESCRIPTOR_TYPE) {
	    /* return our own interface report descriptor */
	    status = ACD_FillControlDescriptorRequest (
		urb, (PUCHAR) ACD_HidReportDescriptor,
		sizeof (ACD_HidReportDescriptor)
		);
	    completeRequestHere = TRUE;
	}
    }

    if (completeRequestHere) {
	Irp->IoStatus.Status = status;
	IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }
    else {
	IoSkipCurrentIrpStackLocation (Irp);
	status = IoCallDriver (deviceExt->lowerDeviceObject, Irp);
    }
    IoReleaseRemoveLock (&deviceExt->removeLock, Irp);
    return status;
}

NTSTATUS
ACD_DispatchPower (IN PDEVICE_OBJECT FilterDeviceObject, IN PIRP Irp)
{
    PDEVICE_EXTENSION deviceExt;
    NTSTATUS status;

    PoStartNextPowerIrp (Irp);

    deviceExt = (PDEVICE_EXTENSION) FilterDeviceObject->DeviceExtension;
    status = IoAcquireRemoveLock (&deviceExt->removeLock, Irp);
    if (!NT_SUCCESS (status))
	return ACD_CompleteRequest (Irp, status, 0);

    IoSkipCurrentIrpStackLocation (Irp);
    status = PoCallDriver (deviceExt->lowerDeviceObject, Irp);
	
    IoReleaseRemoveLock (&deviceExt->removeLock, Irp);
    return status;
}

NTSTATUS
ACD_DispatchPnP (IN PDEVICE_OBJECT FilterDeviceObject, IN PIRP Irp)
{
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION deviceExt;
    ULONG minor;
    NTSTATUS status;

    deviceExt = (PDEVICE_EXTENSION) FilterDeviceObject->DeviceExtension;
    status = IoAcquireRemoveLock (&deviceExt->removeLock, Irp);
    if (!NT_SUCCESS (status))
	return ACD_CompleteRequest (Irp, status, 0);

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    minor = irpStack->MinorFunction;

    /* handle START_DEVICE here since we want to emit a RESET after
     * completion of the lower stack.
     */
    if (minor == IRP_MN_START_DEVICE) {
	KEVENT event;

	KeInitializeEvent (&event, NotificationEvent, FALSE);

	IoCopyCurrentIrpStackLocationToNext (Irp);
	IoSetCompletionRoutine (
	    Irp, ACD_IoCallDriverCompletion, &event, TRUE, TRUE, TRUE
	    );

	status = IoCallDriver (deviceExt->lowerDeviceObject, Irp);

	/* wait for the call to complete (we'll get notified by
	 * the completion routine.
	 */
	KeWaitForSingleObject (
	    &event, Executive, KernelMode, FALSE, NULL
	    );

	status = Irp->IoStatus.Status;

	/* for some reason the device is not functioning properly
	 * after START_DEVICE. Reseting the port seems to wake the
	 * HID monitor controls up.
	 */
	if (NT_SUCCESS (status)) {
	    status = ACD_ResetPort (deviceExt->lowerDeviceObject);
	}

	/* we're done with this IRP, we can complete it here */
	IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }
    else {
	IoSkipCurrentIrpStackLocation (Irp);
	status = IoCallDriver (deviceExt->lowerDeviceObject, Irp);
    }

    if (minor == IRP_MN_REMOVE_DEVICE) {
	IoReleaseRemoveLockAndWait (&deviceExt->removeLock, Irp);
	IoDetachDevice (deviceExt->lowerDeviceObject);
	IoDeleteDevice (FilterDeviceObject);
	return status;
    }

    IoReleaseRemoveLock (&deviceExt->removeLock, Irp);
    return status;
}

NTSTATUS
ACD_IoCallDriverCompletion (IN PDEVICE_OBJECT DeviceObject,
			    IN PIRP Irp, IN PVOID Context)
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);

    KeSetEvent((PKEVENT) Context, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ACD_GetPortStatus (IN PDEVICE_OBJECT LowerDeviceObject, IN OUT PULONG Flags)
{
    PIO_STACK_LOCATION irpStack;
    KEVENT event;
    PIRP irp;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    *Flags = 0;

    KeInitializeEvent (&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest (
	IOCTL_INTERNAL_USB_GET_PORT_STATUS, LowerDeviceObject,
	NULL, 0, NULL, 0, TRUE, &event, &ioStatus
	);

    if (irp == NULL)
	return STATUS_INSUFFICIENT_RESOURCES;

    irpStack = IoGetNextIrpStackLocation (irp);
    irpStack->Parameters.Others.Argument1 = Flags;

    status = IoCallDriver (LowerDeviceObject, irp);

    if (status == STATUS_PENDING) {
	KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);
	return ioStatus.Status;
    }

    return status;
}

NTSTATUS
ACD_ResetPort (IN PDEVICE_OBJECT LowerDeviceObject)
{
    KEVENT event;
    PIRP irp;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    KeInitializeEvent (&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest (
	IOCTL_INTERNAL_USB_RESET_PORT, LowerDeviceObject,
	NULL, 0, NULL, 0, TRUE, &event, &ioStatus
	);

    if (irp == NULL)
	return STATUS_INSUFFICIENT_RESOURCES;

    status = IoCallDriver (LowerDeviceObject, irp);

    if (status == STATUS_PENDING) {
	KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);
	return ioStatus.Status;
    }

    return status;
}

NTSTATUS
ACD_FillControlDescriptorRequest (IN PURB Urb, IN PUCHAR Buffer,
				  IN ULONG BufferLength)
{
    struct _URB_CONTROL_DESCRIPTOR_REQUEST *desc =
	(struct _URB_CONTROL_DESCRIPTOR_REQUEST *) Urb;
    ULONG length = BufferLength < desc->TransferBufferLength
	? BufferLength : desc->TransferBufferLength;

    RtlCopyMemory (desc->TransferBuffer, Buffer, length);

    Urb->UrbHeader.Status = USBD_STATUS_SUCCESS;
    return STATUS_SUCCESS;
}
