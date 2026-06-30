// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "MainWindow.h"
#include <optional>
#include "resource.h"
#include <array>

namespace
{
	std::optional<WNDCLASS> g_mainWindowClass;
	HWND g_handle = nullptr;
	HWND g_btnNewGame = nullptr;
	DWORD g_windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
	std::array<HICON, 4> g_smileIcons;
	constexpr auto OUTER_CONTROL_SPACING = 10;
	constexpr auto INNER_CONTROL_SPACING = 5;

	LRESULT CALLBACK WindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
	{
		switch (uMsg)
		{
			case WM_DESTROY:
			{
				for (const auto i : g_smileIcons)
				{
					DestroyIcon(i);
				}

				PostQuitMessage(0);

				return 0;
			}

			case WM_CREATE:
			{
				g_handle = hwnd;

				g_smileIcons[0] = LoadIcon(g_mainWindowClass->hInstance, MAKEINTRESOURCE(IDI_SMILE1));
				g_smileIcons[1] = LoadIcon(g_mainWindowClass->hInstance, MAKEINTRESOURCE(IDI_SMILE2));
				g_smileIcons[2] = LoadIcon(g_mainWindowClass->hInstance, MAKEINTRESOURCE(IDI_SMILE3));
				g_smileIcons[3] = LoadIcon(g_mainWindowClass->hInstance, MAKEINTRESOURCE(IDI_SMILE4));

				g_btnNewGame = CreateWindowEx(
					0,
					L"BUTTON",
					nullptr,
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_ICON,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					hwnd,
					nullptr,
					g_mainWindowClass->hInstance,
					nullptr
				);
				SendMessage(g_btnNewGame, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(g_smileIcons[0]));
				ICONINFO iconInfo;
				GetIconInfo(g_smileIcons[0], &iconInfo);
				BITMAP bmp;
				GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);
				SetWindowPos(g_btnNewGame, nullptr, 0, 0, bmp.bmWidth + 12, bmp.bmHeight + 12, SWP_NOZORDER | SWP_NOMOVE);

				RECT rcBtnNewGame;
				GetWindowRect(g_btnNewGame, &rcBtnNewGame);
				OffsetRect(&rcBtnNewGame, -rcBtnNewGame.left, -rcBtnNewGame.top);

				RECT rcDesired{};
				rcDesired.right = OUTER_CONTROL_SPACING + rcBtnNewGame.right + OUTER_CONTROL_SPACING;
				rcDesired.bottom = INNER_CONTROL_SPACING + rcBtnNewGame.bottom + OUTER_CONTROL_SPACING;
				AdjustWindowRect(&rcDesired, g_windowStyle, TRUE);
				OffsetRect(&rcDesired, -rcDesired.left, -rcDesired.top);
				SetWindowPos(hwnd, nullptr, 0, 0, rcDesired.right, rcDesired.bottom, SWP_NOZORDER | SWP_NOMOVE);

				RECT rcWindow;
				GetWindowRect(hwnd, &rcWindow);
				OffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);

				RECT rcWorkingArea;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkingArea, 0);

				SetWindowPos(
					hwnd,
					nullptr,
					rcWorkingArea.left + (rcWorkingArea.right - rcWorkingArea.left - rcWindow.right) / 2,
					rcWorkingArea.top + (rcWorkingArea.bottom - rcWorkingArea.top - rcWindow.bottom) / 2,
					0, 0,
					SWP_NOZORDER | SWP_NOSIZE
				);

				return 0;
			}

			case WM_SIZE:
			{
				RECT rcClient;
				GetClientRect(hwnd, &rcClient);

				RECT rcBtnNewGame;
				GetWindowRect(g_btnNewGame, &rcBtnNewGame);
				OffsetRect(&rcBtnNewGame, -rcBtnNewGame.left, -rcBtnNewGame.top);

				SetWindowPos(g_btnNewGame, nullptr, (rcClient.right - rcBtnNewGame.right) / 2, INNER_CONTROL_SPACING, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

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
					switch (LOWORD(wParam))
					{
						case ID_GAME_EXIT:
						{
							DestroyWindow(hwnd);

							return 0;
						}

						default: break;
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
