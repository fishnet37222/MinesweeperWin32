// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <Windows.h>

[[nodiscard]] HWND SevenSegmentDisplay_Create(HWND parent, HINSTANCE appInstance);
void SevenSegmentDisplay_SetValue(HWND instance, int value);
[[nodiscard]] int SevenSegmentDisplay_GetValue(HWND instance);
