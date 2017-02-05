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

	struct LineMetrics
	{
		int height;
		int baseline;
	};

	struct TextBlockMetrics
	{
		Size size;
		std::vector<LineMetrics> lines;
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

	enum class AntialiasMode
	{
		None,
		Grayscale,
		SubPixel
	};

	struct Range
	{
		unsigned int start;
		unsigned int length;

		unsigned int last() const
		{
			return start + length - 1;
		}
	};

	struct Brush
	{
		Color color;
	};
}
