// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "SevenSegmentDisplay.h"
#include <optional>
#include <unordered_map>
#include <gdiplus.h>
#include <memory>
#include "ColorUtility.h"
#include <vector>
#include <string>

namespace
{
#define SEGMENT_LIST TOP,TOP_LEFT,TOP_RIGHT,MIDDLE,BOTTOM_LEFT,BOTTOM_RIGHT,BOTTOM

	enum Segment : uint8_t { SEGMENT_LIST };

	std::unordered_map<wchar_t, std::vector<Segment>> g_digitSegments
	{
		{L'0', {TOP, TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, BOTTOM}},
		{L'1', {TOP_RIGHT, BOTTOM_RIGHT}},
		{L'2', {TOP, TOP_RIGHT, BOTTOM_LEFT, BOTTOM, MIDDLE}},
		{L'3', {TOP, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM, MIDDLE}},
		{L'4', {TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, MIDDLE}},
		{L'5', {TOP, TOP_LEFT, BOTTOM_RIGHT, BOTTOM, MIDDLE}},
		{L'6', {TOP, TOP_LEFT, BOTTOM_LEFT, BOTTOM_RIGHT, BOTTOM, MIDDLE}},
		{L'7', {TOP, TOP_RIGHT, BOTTOM_RIGHT}},
		{L'8', {SEGMENT_LIST}},
		{L'9', {TOP, TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM, MIDDLE}},
		{L' ', {}}
	};

	class Properties final
	{
	public:
		int m_value{ 0 };
		int m_digitSpacing{ 5 };
		int m_segmentThickness{ 3 };
		SIZE m_digitSize{ .cx = 20, .cy = 40 };
		bool m_leadingZerosVisible{ false };
		int m_digitCount{ 3 };
		Gdiplus::Color m_backgroundColor{ 0, 0, 0 };
		Gdiplus::Color m_foregroundColor{ 255, 255, 0 };
	};

	std::optional<WNDCLASS> g_windowClass;
	std::unordered_map<HWND, Properties> g_properties;

	void AutoSizeWindow(HWND window)
	{
		const auto& properties = g_properties[window];

		const auto width = properties.m_digitSpacing * (properties.m_digitCount + 1) + properties.m_digitSize.cx * properties.m_digitCount + 5;
		const auto height = properties.m_digitSize.cy + properties.m_digitSpacing * 2 + 5;
		SetWindowPos(window, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
	}

	LRESULT CALLBACK WindowProc(HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
	{
		switch (uMsg)
		{
			case WM_CREATE:
			{
				g_properties[hwnd] = Properties();

				AutoSizeWindow(hwnd);

				return 0;
			}

			case WM_DESTROY:
			{
				g_properties.erase(hwnd);

				return 0;
			}

			case WM_PAINT:
			{
				RECT rcClient;
				GetClientRect(hwnd, &rcClient);

				const auto backBitmap = std::make_unique<Gdiplus::Bitmap>(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, PixelFormat32bppRGB);
				{
					const auto backGraphics = std::make_unique<Gdiplus::Graphics>(backBitmap.get());
					backGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

					const auto&
						[
							m_value,
							m_digitSpacing,
							m_segmentThickness,
							m_digitSize,
							m_leadingZerosVisible,
							m_digitCount,
							m_backgroundColor,
							m_foregroundColor
						] = g_properties[hwnd];

					const auto bgPen = std::make_unique<Gdiplus::Pen>(m_backgroundColor);
					const auto bgBrush = std::make_unique<Gdiplus::SolidBrush>(m_backgroundColor);
					backGraphics->FillRectangle(bgBrush.get(), 0, 0, rcClient.right, rcClient.bottom);
					backGraphics->DrawRectangle(bgPen.get(), 0, 0, rcClient.right, rcClient.bottom);

					Gdiplus::Color parentBackground;
					LOGBRUSH parentBg;
					GetObject(reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1), sizeof(LOGBRUSH), &parentBg);
					parentBackground.SetFromCOLORREF(parentBg.lbColor);

					const auto highlightPen = std::make_unique<Gdiplus::Pen>(Color_ChangeLightness(parentBackground, 150));
					const auto shadowPen = std::make_unique<Gdiplus::Pen>(Color_ChangeLightness(parentBackground, 50));

					for (auto i = 0; i < 2; i++)
					{
						backGraphics->DrawLine(shadowPen.get(), i, i, rcClient.right - i, i);
						backGraphics->DrawLine(shadowPen.get(), i, i, i, rcClient.bottom - i);

						backGraphics->DrawLine(highlightPen.get(), i, rcClient.bottom - i, rcClient.right - i, rcClient.bottom - i);
						backGraphics->DrawLine(highlightPen.get(), rcClient.right - i, i, rcClient.right - i, rcClient.bottom - i);
					}

					const auto litColor = m_foregroundColor;
					const auto unlitColor = Color_ChangeLightness(litColor, 20);
					const auto halfDigitHeight = m_digitSize.cy / 2;

					auto valueText = std::to_wstring(m_value);
					const auto padChar = m_leadingZerosVisible ? L'0' : L' ';
					if (std::cmp_less(valueText.length(), m_digitCount))
					{
						valueText.insert(valueText.begin(), m_digitCount - valueText.length(), padChar);
					}
					else
					{
						if (std::cmp_greater(valueText.length(), m_digitCount))
						{
							valueText = valueText.substr(valueText.length() - m_digitCount);
						}
					}

					for (auto digitIndex = 0; digitIndex < m_digitCount; digitIndex++)
					{
						const auto& digit = valueText[digitIndex];
						const auto& digitSegments = g_digitSegments[digit];
						const auto digitOriginX = m_digitSpacing * (digitIndex + 1) + m_digitSize.cx * digitIndex + 2;
						const auto digitOriginY = m_digitSpacing + 2;

						for (const auto& segment : { SEGMENT_LIST })
						{
							std::vector<Gdiplus::Point> segmentVertices;

							switch (segment)
							{
								case TOP:
								{
									segmentVertices.emplace_back(digitOriginX + 1, digitOriginY);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - 1, digitOriginY);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness - 1, digitOriginY + m_segmentThickness);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness + 1, digitOriginY + m_segmentThickness);

									break;
								}

								case TOP_LEFT:
								{
									segmentVertices.emplace_back(digitOriginX, digitOriginY + 1);
									segmentVertices.emplace_back(digitOriginX, digitOriginY + halfDigitHeight - 1);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness, digitOriginY + halfDigitHeight - m_segmentThickness - 1);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness, digitOriginY + m_segmentThickness + 1);

									break;
								}

								case TOP_RIGHT:
								{
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx, digitOriginY + 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx, digitOriginY + halfDigitHeight - 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness, digitOriginY + halfDigitHeight - m_segmentThickness - 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness, digitOriginY + m_segmentThickness + 1);

									break;
								}

								case MIDDLE:
								{
									segmentVertices.emplace_back(digitOriginX + 1, digitOriginY + halfDigitHeight);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness - 1, digitOriginY + halfDigitHeight - m_segmentThickness + 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness + 1, digitOriginY + halfDigitHeight - m_segmentThickness + 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - 1, digitOriginY + halfDigitHeight);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness + 1, digitOriginY + halfDigitHeight + m_segmentThickness - 1);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness - 1, digitOriginY + halfDigitHeight + m_segmentThickness - 1);

									break;
								}

								case BOTTOM_LEFT:
								{
									segmentVertices.emplace_back(digitOriginX, digitOriginY + halfDigitHeight + 1);
									segmentVertices.emplace_back(digitOriginX, digitOriginY + m_digitSize.cy - 1);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness, digitOriginY + m_digitSize.cy - m_segmentThickness - 1);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness, digitOriginY + halfDigitHeight + m_segmentThickness + 1);

									break;
								}

								case BOTTOM_RIGHT:
								{
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx, digitOriginY + halfDigitHeight + 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx, digitOriginY + m_digitSize.cy - 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness, digitOriginY + m_digitSize.cy - m_segmentThickness - 1);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness, digitOriginY + halfDigitHeight + m_segmentThickness + 1);

									break;
								}

								case BOTTOM:
								{
									segmentVertices.emplace_back(digitOriginX + 1, digitOriginY + m_digitSize.cy);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - 1, digitOriginY + m_digitSize.cy);
									segmentVertices.emplace_back(digitOriginX + m_digitSize.cx - m_segmentThickness - 1, digitOriginY + m_digitSize.cy - m_segmentThickness);
									segmentVertices.emplace_back(digitOriginX + m_segmentThickness + 1, digitOriginY + m_digitSize.cy - m_segmentThickness);

									break;
								}
							}

							const auto segmentColor = std::ranges::find(digitSegments, segment) != digitSegments.end() ? litColor : unlitColor;
							const auto segmentPen = std::make_unique<Gdiplus::Pen>(segmentColor);
							const auto segmentBrush = std::make_unique<Gdiplus::SolidBrush>(segmentColor);

							if (segmentVertices.empty()) continue;
							backGraphics->DrawPolygon(segmentPen.get(), segmentVertices.data(), static_cast<int>(segmentVertices.size()));
							backGraphics->FillPolygon(segmentBrush.get(), segmentVertices.data(), static_cast<int>(segmentVertices.size()));
						}
					}
				}

				PAINTSTRUCT ps;
				const auto dc = BeginPaint(hwnd, &ps);
				const auto graphics = std::make_unique<Gdiplus::Graphics>(dc);

				graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

				graphics->DrawImage(backBitmap.get(), 0, 0);

				EndPaint(hwnd, &ps);

				return 0;
			}

			default: break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

HWND SevenSegmentDisplay_Create(HWND parent, HINSTANCE appInstance)
{
	if (!g_windowClass)
	{
		WNDCLASS wc = {};
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = L"SevenSegmentDisplay";
		wc.hInstance = appInstance;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

		RegisterClass(&wc);
		g_windowClass = wc;
	}

	const auto handle = CreateWindowEx(
		0,
		g_windowClass->lpszClassName,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		parent,
		nullptr,
		appInstance,
		nullptr
	);

	return handle;
}

void SevenSegmentDisplay_SetValue(HWND instance, int value)
{
	if (g_properties[instance].m_value == value)
	{
		return;
	}

	auto& properties = g_properties[instance];
	properties.m_value = value;
	InvalidateRect(instance, nullptr, TRUE);
}

int SevenSegmentDisplay_GetValue(HWND instance)
{
	return g_properties[instance].m_value;
}
