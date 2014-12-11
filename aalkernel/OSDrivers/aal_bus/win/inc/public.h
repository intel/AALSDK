/*++
Copyright (c) 1990-2000    Microsoft Corporation All Rights Reserved

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/


//
// Define a WMI GUID for bus.
//
DEFINE_GUID(AAL_BUS_WMI_STD_DATA_GUID ,
   0x1e11cdac, 0xe0bd, 0x47d8, 0xa1, 0x23, 0xbc, 0x13, 0xf9, 0xed, 0xd8, 0x35);
// {1E11CDAC-E0BD-47D8-A123-BC13F9EDD835}

//
// Define a WMI GUID to get toaster device info.
//

DEFINE_GUID (TOASTER_WMI_STD_DATA_GUID,
        0xBBA21300L, 0x6DD3, 0x11d2, 0xB8, 0x44, 0x00, 0xC0, 0x4F, 0xAD, 0x51, 0x71);

//
// Define a WMI GUID to represent device arrival notification WMIEvent class.
//
DEFINE_GUID(AAL_BUS_NOTIFY_DEVICE_ARRIVAL_EVENT,
   0x57a20f13, 0xd503, 0x4d54, 0x8e, 0x68, 0xbb, 0xa0, 0x2, 0x1c, 0x41, 0x70);
// {57A20F13-D503-4D54-8E68-BBA0021C4170}


//
// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//

#ifndef __PUBLIC_H
#define __PUBLIC_H

#define BUS_HARDWARE_IDS L"{B85B7C50-6A01-11d2-B841-00C04FAD5171}\\aalbus\0"
#define BUS_HARDWARE_IDS_LENGTH sizeof (BUS_HARDWARE_IDS)

#define BUSENUM_COMPATIBLE_IDS L"{B85B7C50-6A01-11d2-B841-00C04FAD5171}\\aalCompatibleBus\0"
#define BUSENUM_COMPATIBLE_IDS_LENGTH sizeof(BUSENUM_COMPATIBLE_IDS)


#define FILE_DEVICE_BUSENUM         FILE_DEVICE_BUS_EXTENDER

#define BUSENUM_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_BUSENUM, _index_, METHOD_BUFFERED, FILE_READ_DATA)

#define IOCTL_BUSENUM_PLUGIN_HARDWARE               BUSENUM_IOCTL (0x0)
#define IOCTL_BUSENUM_UNPLUG_HARDWARE               BUSENUM_IOCTL (0x1)
#define IOCTL_BUSENUM_EJECT_HARDWARE                BUSENUM_IOCTL (0x2)
#define IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE     BUSENUM_IOCTL (0x3)

//
//  Data structure used in PlugIn and UnPlug ioctls  THIS IS RM SERVER Interface
//

typedef struct _BUSENUM_PLUGIN_HARDWARE
{
    //
    // sizeof (struct _BUSENUM_HARDWARE)
    //
    IN ULONG Size;

    //
    // Unique serial number of the device to be enumerated.
    // Enumeration will be failed if another device on the
    // bus has the same serail number.
    //

    IN ULONG SerialNo;

    //
    // An array of (zero terminated wide character strings). The array itself
    //  also null terminated (ie, MULTI_SZ)
    //
    #pragma warning(disable:4200)  // nonstandard extension used

    IN  WCHAR   HardwareIDs[];

    #pragma warning(default:4200)

} BUSENUM_PLUGIN_HARDWARE, *PBUSENUM_PLUGIN_HARDWARE;

typedef struct _BUSENUM_UNPLUG_HARDWARE
{
    //
    // sizeof (struct _REMOVE_HARDWARE)
    //

    IN ULONG Size;

    //
    // Serial number of the device to be plugged out
    //

    ULONG   SerialNo;

    ULONG Reserved[2];

} BUSENUM_UNPLUG_HARDWARE, *PBUSENUM_UNPLUG_HARDWARE;

typedef struct _BUSENUM_EJECT_HARDWARE
{
    //
    // sizeof (struct _EJECT_HARDWARE)
    //

    IN ULONG Size;

    //
    // Serial number of the device to be ejected
    //

    ULONG   SerialNo;

    ULONG Reserved[2];

} BUSENUM_EJECT_HARDWARE, *PBUSENUM_EJECT_HARDWARE;

#endif

