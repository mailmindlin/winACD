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

#include "ACDEdid.h"

const PCHAR EDID_EstablishedTimingString [] = {
    "800x600@60Hz", /* bit 0, byte 0 */
    "800x600@56Hz",
    "640x480@75Hz",
    "640x480@72Hz",
    "640x480@67Hz",
    "640x480@60Hz",
    "720x400@88Hz",
    "720x400@70Hz",  /* bit 7, byte 0 */
    "1280x1024@75Hz" /* bit 0, byte 1 */
    "1024x768@75Hz", 
    "1024x768@70Hz", 
    "1024x768@60Hz", 
    "1024x768@87Hz(Interlaced)",
    "832x624@75Hz", 
    "800x600@5Hz", 
    "800x600@72Hz"  /* bit 7, byte 1 */
};

