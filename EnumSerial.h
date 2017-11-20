/************************************************************************* 
* Serial port enumeration routines
*
* The EnumSerialPort function will populate an array of SSerInfo structs,
* each of which contains information about one serial port present in
* the system. Note that this code must be linked with setupapi.lib,
* which is included with the Win32 SDK.
*
* by Zach Gorman <gormanjz@hotmail.com>
*
* Copyright (c) 2002 Archetype Auction Software, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following condition is
* met: Redistributions of source code must retain the above copyright
* notice, this condition and the following disclaimer.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL ARCHETYPE AUCTION SOFTWARE OR ITS
* AFFILIATES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************/
#ifndef ENUM_SERIAL_H
#define ENUM_SERIAL_H

#include <afxtempl.h>

// The following define is from ntddser.h in the DDK. It is also
// needed for serial port enumeration.
#ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, \
	0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif

#ifndef GUID_CLASS_USBPORT
DEFINE_GUID(GUID_CLASS_USBPORT, 0xa5dcbf10L, 0x6530, 0x11d2, 0x90, 0x1f, \
	0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed);
#endif

// Struct used when enumerating the available serial ports
// Holds information about an individual serial port.
struct SSerInfo {
    SSerInfo() : bUsbDevice(FALSE) {}
    CString strDevPath;          // Device path for use with CreateFile()
    CString strPortName;         // Simple name (i.e. COM1)
    CString strFriendlyName;     // Full name to be displayed to a user
    BOOL bUsbDevice;             // Provided through a USB connection?
    CString strPortDesc;         // friendly name without the COMx
	CString strHardwareID;		 // hardware ID of the device // lct.sw
};

// Routine for enumerating the available serial ports. Throws a CString on
// failure, describing the error that occurred. If bIgnoreBusyPorts is TRUE,
// ports that can't be opened for read/write access are not included.
void EnumSerialPorts(CArray<SSerInfo,SSerInfo&> &asi, BOOL bIgnoreBusyPorts=TRUE);

void EnumSerialPorts(GUID* guid, CArray<SSerInfo, SSerInfo&> &asi, BOOL bIgnoreBusyPorts);

#endif // ENUM_SERIAL_H 