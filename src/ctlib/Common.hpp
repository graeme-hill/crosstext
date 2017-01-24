#pragma once

#include <vector>
#include <string>

#define FLOAT_ERROR_MARGIN 0.01f

namespace ct
{
	struct Size
	{
		int width;
		int height;
	};

	struct Rect
	{
		int x;
		int y;
		int width;
		int height;

		inline int endX()
		{
			return x + width - 1;
		}

		inline int endY()
		{
			return y + height - 1;
		}
	};

	struct Color
	{
		uint32_t rgba;

		float redf()
		{
			return (rgba >> 24) / 255.0f;
		}

		float greenf()
		{
			return ((rgba & 0x00ff0000) >> 16) / 255.0f;
		}

		float bluef()
		{
			return ((rgba & 0x0000ff00) >> 8) / 255.0f;
		}

		float alphaf()
		{
			return (rgba & 0x000000ff) / 255.0f;
		}
	};

	enum class FontWeight
	{
		Thin = 100,
		ExtraLight = 200,
		Light = 300,
		SemiLight = 350,
		Normal = 400,
		Medium = 500,
		SemiBold = 600,
		Bold = 700,
		ExtraBold = 800,
		Black = 900,
		ExtraBlack = 950
	};

	enum class FontStyle
	{
		Normal,
		Oblique,
		Italic
	};

	enum class FontStretch
	{
		Undefined,
		UltraCondensed,
		ExtraCondensed,
		Condensed,
		SemiCondensed,
		Normal,
		Medium,
		SemiExpanded,
		Expanded,
		ExtraExpanded,
		UltraExpanded
	};

	struct Range
	{
		int start;
		int length;
	};

	struct Brush
	{
		Color color;
	};

	struct FontOptions
	{
		std::wstring family;
		FontWeight weight;
		FontStyle style;
		FontStretch stretch;
		float size;
		std::wstring locale;
		Brush foreground;
	};

	struct FontRange
	{
		FontOptions fontOptions;
		Range range;
	};
}