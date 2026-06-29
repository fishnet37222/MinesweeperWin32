// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "MainWindow.h"
#include <optional>
#include "resource.h"

namespace
{
	std::optional<WNDCLASS> g_mainWindowClass;
	HWND g_handle = nullptr;
	DWORD g_windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX);

	LRESULT CALLBACK WindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
	{
		switch (uMsg)
		{
			case WM_DESTROY:
			{
				PostQuitMessage(0);

				return 0;
			}

			case WM_CREATE:
			{
				g_handle = hwnd;

				return 0;
			}

			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				const auto dc = BeginPaint(hwnd, &ps);

				FillRect(dc, &ps.rcPaint, g_mainWindowClass->hbrBackground);  // NOLINT(bugprone-unchecked-optional-access)

				EndPaint(hwnd, &ps);

				return 0;
			}

			case WM_COMMAND:
			{
				if (lParam == 0)
				{
					switch (LOWORD(lParam))
					{
						case ID_GAME_EXIT:
						{
							DestroyWindow(hwnd);

							return 0;
						}
					}
				}

				return 0;
			}

			default: break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

HWND MainWindow_Create(const HINSTANCE appInstance)
{
	if (!g_mainWindowClass)
	{
		WNDCLASS wc = {};
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = appInstance;
		wc.lpszClassName = L"MainWindowClass";
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);
		wc.hIcon = LoadIcon(appInstance, MAKEINTRESOURCE(IDI_APP_ICON));

		if (!RegisterClassW(&wc))
		{
			return nullptr;
		}

		g_mainWindowClass = wc;
	}

	return CreateWindowEx(
		0,
		g_mainWindowClass->lpszClassName,
		L"Minesweeper",
		g_windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		LoadMenu(appInstance, MAKEINTRESOURCE(IDR_MENU)),
		appInstance,
		nullptr
	);
}
