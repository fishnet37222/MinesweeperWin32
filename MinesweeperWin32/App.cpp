// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "MainWindow.h"
#include "resource.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI wWinMain(const HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] PWSTR lpCmdLine, const int nShowCmd)
{
	// Initialize GDI+
	const Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Gdiplus::Ok)
	{
		MessageBoxW(nullptr, L"Failed to initialize GDI+.", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	const auto hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	const auto mainWindow = MainWindow_Create(hInstance);

	if (!mainWindow)
	{
		MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	ShowWindow(mainWindow, nShowCmd);
	UpdateWindow(mainWindow);

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(mainWindow, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}
