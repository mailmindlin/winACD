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
 * acdfilter.h: Apple HID Monitor Controls WDM filter driver.
 */


#if !defined _ACDFILTER_H_
#define _ACDFILTER_H_ 1

#include <wdm.h>

#if DBG
# define ACD_dbgPrint(p) { \
    DbgPrint ("'ACDFILTER> "); DbgPrint p; DbgPrint ("\n"); \
}
#else /* !DBG */
# define ACD_dbgPrint(p)
#endif /* !DBG */

#define ACD_LAST_DEVICE_STRING_DESCRIPTOR   1
#define ACD_LAST_DRIVER_STRING_DESCRIPTOR   \
    (ACD_LAST_DEVICE_STRING_DESCRIPTOR + 0)

typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT physicalDeviceObject;
    PDEVICE_OBJECT lowerDeviceObject;

    PUSB_STRING_DESCRIPTOR stringDescriptor [ACD_LAST_DRIVER_STRING_DESCRIPTOR + 1];

    IO_REMOVE_LOCK removeLock;

    USHORT idProduct;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _ACD_CONFIGURATION_DESCRIPTOR {

    USB_CONFIGURATION_DESCRIPTOR	Configuration0;
    USB_INTERFACE_DESCRIPTOR		Interface0;
    HID_DESCRIPTOR			Hid0;
    USB_ENDPOINT_DESCRIPTOR		EndPoint1;

} ACD_CONFIGURATION_DESCRIPTOR, *PACD_CONFIGURATION_DESCRIPTOR;

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
ACD_AddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

VOID
ACD_DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ACD_DispatchAny (
    IN PDEVICE_OBJECT FilterDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ACD_DispatchPower (
    IN PDEVICE_OBJECT FilterDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ACD_DispatchPnP (
    IN PDEVICE_OBJECT FilterDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ACD_DispatchIoctl (
    IN PDEVICE_OBJECT FilterDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ACD_IoCallDriverCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
ACD_FillControlDescriptorRequest (
    IN PURB Urb, IN PUCHAR Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
ACD_GetPortStatus (
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN OUT PULONG FLAGS
    );

NTSTATUS
ACD_ResetPort (
    IN PDEVICE_OBJECT LowerDeviceObject
    );

NTSTATUS
ACD_CacheStringDescriptors (
    IN OUT PDEVICE_EXTENSION DeviceExt
    );

NTSTATUS
ACD_GetStringDescriptor (
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN UCHAR Index,
    IN OUT PUSB_STRING_DESCRIPTOR pDescriptor,
    IN ULONG Size
    );

NTSTATUS
ACD_GetDeviceDescriptor (
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN OUT PUSB_DEVICE_DESCRIPTOR pDescriptor,
    IN ULONG Size
    );

#endif /* !defined _ACD_FILTER_H_ */