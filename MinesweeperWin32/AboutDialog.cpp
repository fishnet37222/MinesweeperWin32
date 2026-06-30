// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "AboutDialog.h"
#include <optional>
#include <memory>
#include <WebView2.h>
#include <wil/com.h>
#include <wrl.h>
#include "LICENSE-2.0-html.h"

namespace
{
	std::optional<WNDCLASSEX> g_aboutDialogClass;
	HWND g_handle = nullptr;
	HWND g_btnClose = nullptr;
	HWND g_lblAppName = nullptr;
	HWND g_lblCopyright = nullptr;
	HWND g_lblLicenseDeclaration = nullptr;
	constexpr auto OUTER_SPACING = 10;
	constexpr auto INNER_SPACING = 5;
	std::optional<HFONT> g_defaultFont;
	DWORD g_windowExStyle = WS_EX_DLGMODALFRAME;
	DWORD g_windowStyle = WS_CAPTION | WS_SYSMENU;
	HWND g_owner = nullptr;
	wil::com_ptr<ICoreWebView2Controller> g_webViewController;
	SIZE g_webViewSize{ .cx = 600, .cy = 300 };
	wil::com_ptr<ICoreWebView2> g_webView;

	void AutoSizeButton(HWND button)
	{
		const auto textLength = GetWindowTextLength(button);
		const auto text = std::make_unique<wchar_t[]>(textLength + 1);
		GetWindowText(button, text.get(), textLength + 1);
		const auto dc = GetDC(button);
		SelectObject(dc, g_defaultFont.value()); // NOLINT(bugprone-unchecked-optional-access)
		SIZE textSize;
		GetTextExtentPoint32(dc, text.get(), textLength, &textSize);
		ReleaseDC(button, dc);

		RECT rcDesired{};
		rcDesired.right = textSize.cx + 30;
		rcDesired.bottom = textSize.cy + 10;
		AdjustWindowRect(&rcDesired, WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, FALSE);
		OffsetRect(&rcDesired, -rcDesired.left, -rcDesired.top);

		SetWindowPos(button, nullptr, 0, 0, rcDesired.right, rcDesired.bottom, SWP_NOMOVE | SWP_NOZORDER);
	}

	void AutoSizeLabel(HWND label)
	{
		const auto textLength = GetWindowTextLength(label);
		const auto text = std::make_unique<wchar_t[]>(textLength + 1);
		GetWindowText(label, text.get(), textLength + 1);
		const auto dc = GetDC(label);
		SelectObject(dc, g_defaultFont.value()); // NOLINT(bugprone-unchecked-optional-access)
		SIZE textSize;
		GetTextExtentPoint32(dc, text.get(), textLength, &textSize);
		ReleaseDC(label, dc);

		RECT rcDesired;
		rcDesired.right = textSize.cx;
		rcDesired.bottom = textSize.cy;

		SetWindowPos(label, nullptr, 0, 0, rcDesired.right, rcDesired.bottom, SWP_NOMOVE | SWP_NOZORDER);
	}

	void CreateWebView()
	{
		// Create WebView2 environment
		std::ignore = CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
			Microsoft::WRL::Callback<
			ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[](const HRESULT result,
					ICoreWebView2Environment* env) -> HRESULT
				{
					if (FAILED(result))
					{
						return result;
					}

					// Create WebView2 controller
					std::ignore = env->CreateCoreWebView2Controller(
						g_handle,
						Microsoft::WRL::Callback<
						ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
							[](const HRESULT hresult,
								ICoreWebView2Controller* controller) ->
							HRESULT
							{
								if (FAILED(hresult))
								{
									return hresult;
								}

								if (controller != nullptr)
								{
									g_webViewController = controller;
									std::ignore = g_webViewController
										->get_CoreWebView2(
											&g_webView);

									// Add NavigationStarting event handler to intercept navigation
									std::ignore = g_webView->
										add_NavigationStarting(
											Microsoft::WRL::Callback
											<ICoreWebView2NavigationStartingEventHandler>(
												[]([[maybe_unused
												]] ICoreWebView2*
													sender,
													ICoreWebView2NavigationStartingEventArgs
													* args) -> HRESULT
												{
													wil::unique_cotaskmem_string
														uri;
													std::ignore =
														args
														->
														get_Uri(
															&uri);

													// Allow data: URIs to load in the WebView
													if (
														uri &&
														wcsncmp(
															uri.
															get(),
															L"data:",
															5) !=
														0)
													{
														// Cancel navigation in WebView
														std::ignore
															= args
															->
															put_Cancel(
																TRUE);

														// Open in default browser
														ShellExecute(
															nullptr,
															L"open",
															uri.
															get(),
															nullptr,
															nullptr,
															SW_SHOWNORMAL);
													}

													return S_OK;
												}).Get(),
													nullptr);

									// Trigger WM_SIZE to position the WebView properly
									RECT rcClient;
									GetClientRect(
										g_handle, &rcClient);
									SendMessage(
										g_handle, WM_SIZE, 0,
										MAKELPARAM(rcClient.right,
											rcClient.bottom));

									std::ignore = g_webView->
										NavigateToString(
											g_licenseHtml);
								}

								return S_OK;
							}).Get());

					return S_OK;
				}).Get());
	}

	LRESULT CALLBACK WindowProc(HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
	{
		switch (uMsg)
		{
			case WM_CREATE:
			{
				g_handle = hwnd;

				g_lblAppName = CreateWindowEx(
					0,
					L"STATIC",
					L"Minesweeper 1.0.0",
					WS_CHILD | WS_VISIBLE,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					hwnd,
					nullptr,
					g_aboutDialogClass->hInstance, // NOLINT(bugprone-unchecked-optional-access)
					nullptr
				);
				SendMessage(g_lblAppName, WM_SETFONT, reinterpret_cast<WPARAM>(g_defaultFont.value()), TRUE); // NOLINT(bugprone-unchecked-optional-access)
				AutoSizeLabel(g_lblAppName);

				RECT rcLblAppName;
				GetWindowRect(g_lblAppName, &rcLblAppName);
				OffsetRect(&rcLblAppName, -rcLblAppName.left, -rcLblAppName.top);

				g_lblCopyright = CreateWindowEx(
					0,
					L"STATIC",
					L"Copyright \u{A9} 2024 FishNetSoft",
					WS_CHILD | WS_VISIBLE,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					hwnd,
					nullptr,
					g_aboutDialogClass->hInstance, // NOLINT(bugprone-unchecked-optional-access)
					nullptr
				);
				SendMessage(g_lblCopyright, WM_SETFONT, reinterpret_cast<WPARAM>(g_defaultFont.value()), TRUE); // NOLINT(bugprone-unchecked-optional-access)
				AutoSizeLabel(g_lblCopyright);

				RECT rcLblCopyright;
				GetWindowRect(g_lblCopyright, &rcLblCopyright);
				OffsetRect(&rcLblCopyright, -rcLblCopyright.left, -rcLblCopyright.top);

				g_lblLicenseDeclaration = CreateWindowEx(
					0,
					L"STATIC",
					L"Licensed under the Apache License, Version 2.0",
					WS_CHILD | WS_VISIBLE,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					hwnd,
					nullptr,
					g_aboutDialogClass->hInstance, // NOLINT(bugprone-unchecked-optional-access)
					nullptr
				);
				SendMessage(g_lblLicenseDeclaration, WM_SETFONT, reinterpret_cast<WPARAM>(g_defaultFont.value()), TRUE); // NOLINT(bugprone-unchecked-optional-access)
				AutoSizeLabel(g_lblLicenseDeclaration);

				RECT rcLblLicenseDeclaration;
				GetWindowRect(g_lblLicenseDeclaration, &rcLblLicenseDeclaration);
				OffsetRect(&rcLblLicenseDeclaration, -rcLblLicenseDeclaration.left, -rcLblLicenseDeclaration.top);

				CreateWebView();

				g_btnClose = CreateWindowEx(
					0,
					L"BUTTON",
					L"Close",
					WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					hwnd,
					nullptr,
					g_aboutDialogClass->hInstance, // NOLINT(bugprone-unchecked-optional-access)
					nullptr
				);
				SendMessage(g_btnClose, WM_SETFONT, reinterpret_cast<WPARAM>(g_defaultFont.value()), TRUE); // NOLINT(bugprone-unchecked-optional-access)
				AutoSizeButton(g_btnClose);

				RECT rcBtnClose;
				GetWindowRect(g_btnClose, &rcBtnClose);
				OffsetRect(&rcBtnClose, -rcBtnClose.left, -rcBtnClose.top);

				RECT rcDesired{};
				rcDesired.right = OUTER_SPACING + std::max({
									  rcLblAppName.right, rcLblCopyright.right,
									  rcLblLicenseDeclaration.right, rcBtnClose.right,
									  g_webViewSize.cx
					})
					+ OUTER_SPACING;
				rcDesired.bottom =
					OUTER_SPACING + rcLblAppName.bottom + INNER_SPACING +
					rcLblCopyright.bottom + INNER_SPACING + rcLblLicenseDeclaration.
					bottom + INNER_SPACING + g_webViewSize.cy + INNER_SPACING +
					rcBtnClose.bottom + OUTER_SPACING;
				AdjustWindowRect(&rcDesired, g_windowStyle, FALSE);
				OffsetRect(&rcDesired, -rcDesired.left, -rcDesired.top);
				SetWindowPos(hwnd, nullptr, 0, 0, rcDesired.right, rcDesired.bottom,
					SWP_NOMOVE | SWP_NOZORDER);

				RECT rcOwner;
				GetWindowRect(g_owner, &rcOwner);

				RECT rcAboutDialog;
				GetWindowRect(hwnd, &rcAboutDialog);
				OffsetRect(&rcAboutDialog, -rcAboutDialog.left, -rcAboutDialog.top);

				SetWindowPos(hwnd, nullptr,
					rcOwner.left + (
						rcOwner.right - rcOwner.left - rcAboutDialog.right) / 2,
					rcOwner.top + (
						rcOwner.bottom - rcOwner.top - rcAboutDialog.bottom) / 2,
					0, 0, SWP_NOSIZE | SWP_NOZORDER);

				return 0;
			}

			case WM_SIZE:
			{
				RECT rcClient;
				GetClientRect(hwnd, &rcClient);

				RECT rcLblAppName;
				GetWindowRect(g_lblAppName, &rcLblAppName);
				OffsetRect(&rcLblAppName, -rcLblAppName.left, -rcLblAppName.top);

				SetWindowPos(g_lblAppName, nullptr, (rcClient.right - rcLblAppName.right) / 2,
					OUTER_SPACING, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				RECT rcLblCopyright;
				GetWindowRect(g_lblCopyright, &rcLblCopyright);
				OffsetRect(&rcLblCopyright, -rcLblCopyright.left, -rcLblCopyright.top);

				SetWindowPos(g_lblCopyright, nullptr,
					(rcClient.right - rcLblCopyright.right) / 2,
					OUTER_SPACING + rcLblAppName.bottom + INNER_SPACING, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER);

				RECT rcLblLicenseDeclaration;
				GetWindowRect(g_lblLicenseDeclaration, &rcLblLicenseDeclaration);
				OffsetRect(&rcLblLicenseDeclaration, -rcLblLicenseDeclaration.left,
					-rcLblLicenseDeclaration.top);

				SetWindowPos(g_lblLicenseDeclaration, nullptr,
					(rcClient.right - rcLblLicenseDeclaration.right) / 2,
					OUTER_SPACING + rcLblAppName.bottom + INNER_SPACING +
					rcLblCopyright.bottom + INNER_SPACING, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER);

				RECT rcBtnClose;
				GetWindowRect(g_btnClose, &rcBtnClose);
				OffsetRect(&rcBtnClose, -rcBtnClose.left, -rcBtnClose.top);

				SetWindowPos(g_btnClose, nullptr,
					rcClient.right - rcBtnClose.right - OUTER_SPACING,
					rcClient.bottom - rcBtnClose.bottom - OUTER_SPACING, 0, 0,
					SWP_NOSIZE | SWP_NOZORDER);

				// Set bounds for WebView (only if it's been created)
				if (g_webViewController)
				{
					const RECT bounds{
						.left = OUTER_SPACING,
						.top = OUTER_SPACING + rcLblAppName.bottom + INNER_SPACING +
							   rcLblCopyright.bottom + INNER_SPACING + rcLblLicenseDeclaration
							   .bottom + INNER_SPACING,
						.right = rcClient.right - OUTER_SPACING,
						.bottom = rcClient.bottom - rcBtnClose.bottom - OUTER_SPACING -
								  INNER_SPACING
					};
					std::ignore = g_webViewController->put_Bounds(bounds);
				}

				return 0;
			}

			case WM_COMMAND:
			{
				if (lParam > 0)
				{
					if (const auto handle = reinterpret_cast<HWND>(lParam);
						handle == g_btnClose)
					{
						DestroyWindow(hwnd);
						return 0;
					}
				}

				return 0;
			}

			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				const auto dc = BeginPaint(hwnd, &ps);

				FillRect(dc, &ps.rcPaint, g_aboutDialogClass->hbrBackground); // NOLINT(bugprone-unchecked-optional-access)

				EndPaint(hwnd, &ps);

				return 0;
			}

			case WM_DESTROY:
			{
				PostQuitMessage(0);

				return 0;
			}

			default: break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void AboutDialog_CreateAndShow(HINSTANCE appInstance, HWND owner)
{
	if (!g_aboutDialogClass)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = L"AboutDialog";
		wc.hInstance = appInstance;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);

		RegisterClassEx(&wc);
		g_aboutDialogClass = wc;
	}

	g_owner = owner;

	if (!g_defaultFont)
	{
		NONCLIENTMETRICS ncm = {};
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		g_defaultFont = CreateFontIndirect(&ncm.lfMessageFont);
	}

	if (owner)
	{
		EnableWindow(owner, FALSE);
	}

	const auto handle = CreateWindowEx(
		g_windowExStyle,
		g_aboutDialogClass->lpszClassName,
		L"About Minesweeper",
		g_windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		owner,
		nullptr,
		appInstance,
		nullptr
	);

	if (!handle)
	{
		const auto error = GetLastError();
		const auto errorMessage = std::make_unique<wchar_t[]>(256);
		FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorMessage.get(),
			256,
			nullptr
		);
		OutputDebugString(errorMessage.get());
		OutputDebugString(L"\n");
	}

	ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		if (!IsDialogMessage(handle, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	g_defaultFont.reset();

	if (owner)
	{
		EnableWindow(owner, TRUE);
		SetForegroundWindow(owner);
	}
}
