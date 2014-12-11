//****************************************************************************
//                               INTEL CONFIDENTIAL
//
//        Copyright (c) 2012 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all  documents related to
// the  source  code  ("Material")  are  owned  by  Intel  Corporation  or its
// suppliers  or  licensors.    Title  to  the  Material  remains  with  Intel
// Corporation or  its suppliers  and licensors.  The Material  contains trade
// secrets  and  proprietary  and  confidential  information  of  Intel or its
// suppliers and licensors.  The Material is protected  by worldwide copyright
// and trade secret laws and treaty provisions. No part of the Material may be
// used,   copied,   reproduced,   modified,   published,   uploaded,  posted,
// transmitted,  distributed,  or  disclosed  in any way without Intel's prior
// express written permission.
//
// No license under any patent,  copyright, trade secret or other intellectual
// property  right  is  granted  to  or  conferred  upon  you by disclosure or
// delivery  of  the  Materials, either expressly, by implication, inducement,
// estoppel or otherwise.  Any license under such intellectual property rights
// must be express and approved by Intel in writing.
//****************************************************************************
//****************************************************************************
//        FILE: aal_pciedriver.h
//     CREATED: 04/30/2014
//      AUTHOR: Joseph Grecco, Intel Corporation
//
// PURPOSE: Public definitions for the AAL Windows PCIe Shell Driver.
//
// HISTORY:
// WHEN:          WHO:     WHAT:
//****************************************************************************


//
// Define an Interface Guid for AAL PCIe Shell device class.
// This GUID is used to register (IoRegisterDeviceInterface)
// an instance of an interface so that user application
// can control the shell device.
//
// {3A704F1B-0DBA-408D-B4D5-9D3D7A350CF3}
DEFINE_GUID(GUID_DEVINTERFACE_AALPCIE, 
0x3a704f1b, 0xdba, 0x408d, 0xb4, 0xd5, 0x9d, 0x3d, 0x7a, 0x35, 0xc, 0xf3);

//
// Define a Setup Class GUID for PCIe Shell Class. This is same
// as the CLASS guid in the INF files.
//
DEFINE_GUID (GUID_DEVCLASS_AALPCIE,
        0x0FDCAAAD,0x6273, 0x463D, 0xB5, 0x9C, 0xE3, 0xB7, 0x67, 0x61, 0xA5, 0x4D);


//
// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//
#ifndef _AAL_PCIEDRIVER_H_
#define _AAL_PCIEDRIVER_H_
// DMA Register Locations (byte offset from BAR 0)

#define MAX_TRANSFER_SIZE	32768					// RRW - match application allocation

#define SPL2_DIRECT                    0x0C00
#define SPL2_DIRECT_FUNCTION(f)     (f+SPL2_DIRECT)

// IOCTL code
#define IOCTL_SPL2_DIRECT_CREATE_WORKSPACE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                                     SPL2_DIRECT_FUNCTION(0x000), \
                                                     METHOD_BUFFERED,       \
                                                     FILE_ANY_ACCESS)

#define IOCTL_SPL2_DIRECT_FREE_WORKSPACE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                                   SPL2_DIRECT_FUNCTION(0x001), \
                                                   METHOD_BUFFERED,       \
                                                   FILE_ANY_ACCESS)


#define IOCTL_SPL2_DIRECT_WAIT_MESSAGE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                                   SPL2_DIRECT_FUNCTION(0x002), \
                                                   METHOD_BUFFERED,       \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SPL2_DIRECT_GET_MESSAGE CTL_CODE (FILE_DEVICE_UNKNOWN,   \
                                                   SPL2_DIRECT_FUNCTION(0x003), \
                                                   METHOD_BUFFERED,       \
                                                   FILE_ANY_ACCESS)


#endif // _AAL_PCIEDRIVER_H_

