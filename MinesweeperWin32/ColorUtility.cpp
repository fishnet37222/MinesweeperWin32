// Copyright (c) 2026 David A. Frischknecht
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "ColorUtility.h"
#include <algorithm>

namespace
{
	[[nodiscard]] BYTE AlphaBlend(const BYTE fg, const BYTE bg, const double alpha)
	{
		auto result = bg + alpha * (fg - bg);
		result = std::clamp(result, 0.0, 255.0);

		return static_cast<BYTE>(result);
	}
}

Gdiplus::Color Color_ChangeLightness(const Gdiplus::Color source, int iAlpha)
{
	if (iAlpha == 100) return source;

	iAlpha = std::clamp(iAlpha, 0, 200);
	auto alpha = (iAlpha - 100.0) / 100.0;

	BYTE bg;
	if (iAlpha > 100)
	{
		bg = 255;
		alpha = 1.0 - alpha;
	}
	else
	{
		bg = 0;
		alpha = 1.0 + alpha;
	}

	auto r = AlphaBlend(source.GetR(), bg, alpha);
	auto g = AlphaBlend(source.GetG(), bg, alpha);
	auto b = AlphaBlend(source.GetB(), bg, alpha);

	return { r, g, b };
}
