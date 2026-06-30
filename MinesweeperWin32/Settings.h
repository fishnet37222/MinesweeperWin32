// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <Windows.h>

void Settings_SetMainWindowPosition(POINT position);
[[nodiscard]] POINT Settings_GetMainWindowPosition();
