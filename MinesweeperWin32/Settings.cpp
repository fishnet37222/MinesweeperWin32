// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "Settings.h"
#include <string>

namespace
{
	std::wstring g_settingsRegSubPath(L"SOFTWARE\\FishNetSoft\\Minesweeper");
	std::wstring g_mainWindowPositionX(L"MainWindowPosX");
	std::wstring g_mainWindowPositionY(L"MainWindowPosY");
}

void Settings_SetMainWindowPosition(POINT position)
{
	HKEY settingsRegKey;
	RegCreateKeyEx(HKEY_CURRENT_USER, g_settingsRegSubPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &settingsRegKey, nullptr);

	RegSetKeyValue(settingsRegKey, nullptr, g_mainWindowPositionX.c_str(), REG_DWORD, &position.x, sizeof(LONG));
	RegSetKeyValue(settingsRegKey, nullptr, g_mainWindowPositionY.c_str(), REG_DWORD, &position.y, sizeof(LONG));

	RegCloseKey(settingsRegKey);
}

POINT Settings_GetMainWindowPosition()
{
	POINT position = { .x = -1, .y = -1 };

	HKEY settingsRegKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_settingsRegSubPath.c_str(), 0, KEY_READ, &settingsRegKey) == ERROR_SUCCESS)
	{
		DWORD dataSize = sizeof(LONG);
		RegGetValueW(settingsRegKey, nullptr, g_mainWindowPositionX.c_str(), RRF_RT_REG_DWORD, nullptr, &position.x, &dataSize);
		RegGetValueW(settingsRegKey, nullptr, g_mainWindowPositionY.c_str(), RRF_RT_REG_DWORD, nullptr, &position.y, &dataSize);
	}

	return position;
}
