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

#include <wtypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EDID_MONOCHROME_DISPLAY_TYPE		0
#define EDID_RGB_COLOUR_DISPLAY_TYPE		1
#define EDID_RGB_NON_RGB_COLOUR_DISPLAY_TYPE	2
#define EDID_UNDEFINED_DISPLAY_TYPE		3

#define EDID_VIDEO_INPUT_ANALOG_TYPE	0
#define EDID_VIDEO_INPUT_DIGITAL_TYPE	1

#define EDID_ASPECT_RATIO_16x10	0
#define EDID_ASPECT_RATIO_4x3	1
#define EDID_ASPECT_RATIO_5x4	2
#define EDID_ASPECT_RATIO_16x9	3

extern const PCHAR EDID_EstablishedTimingString [];

#pragma pack (push, 1)
/** EXTENDED DISPLAY IDENTIFICATION DATA */
typedef struct _EDID_STRUCT {

    /* EDID Header must be: 00ffffffffffff00 */
    UCHAR EDID_HEADER [8];

    /** Vendor / Product Identification */
    struct _EDID_PRODUCT_IDENTIFICATION {
	USHORT	wManufacturerID;	    /* EISA 3-character ID	*/
	USHORT	wProductCode;		    /* Vendor assigned code	*/
	DWORD	dwSerialNumber;		    /* 32-bit serial number	*/
	UCHAR	bManufacturingWeek;	    /* Week number		*/
	UCHAR	bManufacturingYear;	    /* Year			*/
    } ProductIdentification;

    /** EDID Structure Version / Revision */
    struct _EDID_VERSION {
	UCHAR	bVersion;		    /* EDID Version Number	*/
	UCHAR	bRevision;		    /* EDID Revision Number	*/
    } Version;

    /** Basic Display Parameters / Features */
    struct _EDID_DISPLAY_PARAMETERS {
	union {
	    struct {
		UCHAR _reserved		:7;
		UCHAR Type		:1;
	    };
	    /** Digital Video Input Definition */
	    struct {
		UCHAR DFP1xCompatible	:1;
		UCHAR _unused		:6;
		UCHAR Type		:1; /* Must be 1 */
	    } Digital;
	    /** Analog Video Input Definition */
	    struct {
		UCHAR SerrationVSync	:1;
		UCHAR SyncOnGreen	:1;
		UCHAR CompositeSync	:1;
		UCHAR SeparateSyncs	:1;
		UCHAR BlankToBack	:1;
		UCHAR VideoLevel	:2;
		UCHAR Type		:1; /* Must be 0 */
	    } Analog;
	} VideoInputDefinition;
	UCHAR	bMaxHorizontalImageSize;    /* Max. Horizontal Image Size	*/ 
	UCHAR	bMaxVerticalImageSize;	    /* Max. Vertical Image Size		*/
	UCHAR	bGamma;			    /* Display Transfer Characteristic	*/
	struct _EDID_DISPLAY_FEATURES {	    /* DPMS / Feature Support		*/
	    UCHAR DefaultGTFSupported	:1;
	    UCHAR PreferedTimingMode	:1;
	    UCHAR sRGB			:1;
	    UCHAR DisplayType		:2;
	    UCHAR DmpsActiveOff		:1;
	    UCHAR DpmsSuspend		:1;
	    UCHAR DpmsStandby		:1;
	} Features;
    } DisplayParameters;

    /** Color Characteristics */
    struct _EDID_COLOR_PARAMETERS {
	UCHAR	bParam [10];
    } ColorParameters;

    /** Established Timings and Manufacturer's reserved timing */
    struct _EDID_TIMINGS {
	USHORT	EstablishedTimings;
	UCHAR	ManufacturerTimings;
    } Timings;

    /** Standard Timing Identification */
    struct _EDID_STANDARD_TIMING_IDS {
	UCHAR	bHorizentalResolution;
	UCHAR	bVerticalFrequency	:6;
	UCHAR	AspectRatio		:2;
    } StandardTimingID [8];

    /** Detailed Timing Descriptions / Descriptor blocks */
    UCHAR   DetailedTiming [4][18];

    UCHAR   bExtensionFlag; /* Number of 128-byte EDID extension blocks */
    UCHAR   bChecksum;	    /* Checksum (1-byte sum of all bytes == 0) */

    UCHAR   Extension [128];
} EDID_STRUCT, *PEDID_STRUCT;
#pragma pack (pop)

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
