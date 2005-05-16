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

#include "ACDHidDev.h"

namespace ACDUtil
{
    /**
     * Registry functions:
     */

    /** Get the HotKey code for increase/decrease brightness */
    DWORD GetHotKeyPref (BOOL bIncreaseBrightness);

    /** Set the HotKey code for increase/decrease brightness */
    void SetHotKeyPref (BOOL bIncreaseBrightness, DWORD wHotKey);

    /** Return the disabled button mask (see ACDHidDevice::Flags). */
    UCHAR GetDisabledButtonsPref ();

    /** Set the disabled buttons mask. */
    void SetDisabledButtonsPref (UCHAR bMask);

    /** Get the action to execute when the power button is pressed. */
    ACDPowerButtonAction GetPowerButtonActionPref ();

    /** Set the action to execute when the power button is pressed. */
    void SetPowerButtonActionPref (ACDPowerButtonAction iAction);
};
