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

#include "ACDUtil.h"

DWORD
ACDUtil::GetHotKeyPref (BOOL bIncreaseBrightness)
{
    LPTSTR lpValueName = bIncreaseBrightness
	? "IncreaseBrightnessHotKey" 
	: "DecreaseBrightnessHotKey";

    HKEY hKey;
    DWORD type, sz, value = 0;
    LONG lRet;

    // default setting for the system hotkeys
    value = MAKELONG (bIncreaseBrightness ? VK_ADD : VK_SUBTRACT,
	HOTKEYF_CONTROL | HOTKEYF_ALT);

    lRet = RegOpenKeyEx (HKEY_CURRENT_USER, "SOFTWARE\\WinACD\\Preferences",
	0, KEY_READ, &hKey);

    if (lRet == ERROR_SUCCESS) {
	sz = sizeof (value);
	RegQueryValueEx (hKey, lpValueName, 0,
	    &type, (LPBYTE) &value, &sz);

	RegCloseKey (hKey);
    }

    return value;
}

void
ACDUtil::SetHotKeyPref (BOOL bIncreaseBrightness, DWORD wHotKey)
{
    LPTSTR lpValueName = bIncreaseBrightness
	? "IncreaseBrightnessHotKey" 
	: "DecreaseBrightnessHotKey";

    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKeyEx (
	HKEY_CURRENT_USER, "SOFTWARE\\WinACD\\Preferences", 0, 0,
	REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 0, &hKey, 0);

    if (lRet == ERROR_SUCCESS) {
        RegSetValueEx (hKey, lpValueName, 0,
	    REG_DWORD, (LPBYTE) &wHotKey, sizeof (wHotKey));

	RegCloseKey (hKey);
    }
}

UCHAR
ACDUtil::GetDisabledButtonsPref ()
{
    HKEY hKey;
    DWORD type, sz, value = 0;
    LONG lRet;

    lRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "SOFTWARE\\WinACD\\Preferences",
	0, KEY_READ, &hKey);

    if (lRet == ERROR_SUCCESS) {
	sz = sizeof (value);
	RegQueryValueEx (hKey, "DisabledButtons", 0,
	    &type, (LPBYTE) &value, &sz);

	RegCloseKey (hKey);
    }

    return (UCHAR) value;
}

void
ACDUtil::SetDisabledButtonsPref (UCHAR bMask)
{
    HKEY hKey;
    DWORD value = bMask;
    LONG lRet;

    lRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "SOFTWARE\\WinACD\\Preferences",
	0, KEY_SET_VALUE, &hKey);

    if (lRet == ERROR_SUCCESS) {
        RegSetValueEx (hKey, "DisabledButtons", 0,
	    REG_DWORD, (LPBYTE) &value, sizeof (value));

	RegCloseKey (hKey);
    }
}

ACDPowerButtonAction
ACDUtil::GetPowerButtonActionPref ()
{
    HKEY hKey;
    DWORD type, sz, value = 0;
    LONG lRet;

    lRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "SOFTWARE\\WinACD\\Preferences",
	0, KEY_READ, &hKey);

    if (lRet == ERROR_SUCCESS) {
	sz = sizeof (value);
	RegQueryValueEx (hKey, "PowerButtonAction", 0,
	    &type, (LPBYTE) &value, &sz);

	RegCloseKey (hKey);
    }

    return (ACDPowerButtonAction) value;
}

void
ACDUtil::SetPowerButtonActionPref (ACDPowerButtonAction iAction)
{
    HKEY hKey;
    DWORD value = iAction;
    LONG lRet;

    lRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "SOFTWARE\\WinACD\\Preferences",
	0, KEY_SET_VALUE, &hKey);

    if (lRet == ERROR_SUCCESS) {
        RegSetValueEx (hKey, "PowerButtonAction", 0,
	    REG_DWORD, (LPBYTE) &value, sizeof (value));

	RegCloseKey (hKey);
    }
}

BOOL
ACDUtil::GetForceShutdownPref ()
{
    HKEY hKey;
    DWORD type, sz, value = 0;
    LONG lRet;

    lRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "SOFTWARE\\WinACD\\Preferences",
	0, KEY_READ, &hKey);

    if (lRet == ERROR_SUCCESS) {
	sz = sizeof (value);
	RegQueryValueEx (hKey, "ForceShutdown", 0,
	    &type, (LPBYTE) &value, &sz);

	RegCloseKey (hKey);
    }

    return (ACDPowerButtonAction) value;
}

void
ACDUtil::SetForceShutdownPref (BOOL bForce)
{
    HKEY hKey;
    DWORD value = bForce;
    LONG lRet;

    lRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "SOFTWARE\\WinACD\\Preferences",
	0, KEY_SET_VALUE, &hKey);

    if (lRet == ERROR_SUCCESS) {
        RegSetValueEx (hKey, "ForceShutdown", 0,
	    REG_DWORD, (LPBYTE) &value, sizeof (value));

	RegCloseKey (hKey);
    }
}
