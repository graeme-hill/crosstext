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

	enum class AntialiasMode
	{
		None,
		Grayscale,
		SubPixel
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

	struct Font
	{
		std::wstring family;
		FontWeight weight;
		FontStyle style;
		FontStretch stretch;
		float size;
		std::wstring locale;
		Brush foreground;

		Font withFamily(std::wstring newFamily)
		{
			Font newFont(*this);
			newFont.family = newFamily;
			return newFont;
		}

		Font withWeight(FontWeight newWeight)
		{
			Font newFont(*this);
			newFont.weight = newWeight;
			return newFont;
		}

		Font withStyle(FontStyle newStyle)
		{
			Font newFont(*this);
			newFont.style = newStyle;
			return newFont;
		}

		Font withStretch(FontStretch newStretch)
		{
			Font newFont(*this);
			newFont.stretch = newStretch;
			return newFont;
		}

		Font withSize(float newSize)
		{
			Font newFont(*this);
			newFont.size = newSize;
			return newFont;
		}

		Font withLocale(std::wstring newLocale)
		{
			Font newFont(*this);
			newFont.locale = newLocale;
			return newFont;
		}

		Font withForeground(Brush newForeground)
		{
			Font newFont(*this);
			newFont.foreground = newForeground;
			return newFont;
		}
	};

	struct FontRange
	{
		Font fontOptions;
		Range range;
	};

	struct TextOptions
	{
		Font baseFont;
		AntialiasMode antialiasMode;
		std::vector<FontRange> fontRanges;
		Color background;

		inline static TextOptions fromFont(Font base)
		{
			return{ base, AntialiasMode::Grayscale, {}, 0x00000000 };
		}

		TextOptions withFont(Font newBaseFont)
		{
			TextOptions opts(*this);
			opts.baseFont = newBaseFont;
			return opts;
		}

		TextOptions withAntialiasMode(AntialiasMode newAntialiasMode)
		{
			TextOptions opts(*this);
			opts.antialiasMode = newAntialiasMode;
			return opts;
		}

		TextOptions withFontRanges(std::vector<FontRange> newRanges)
		{
			TextOptions opts(*this);
			opts.fontRanges = newRanges;
			return opts;
		}

		TextOptions withBackground(Color newBackground)
		{
			TextOptions opts(*this);
			opts.background = newBackground;
			return opts;
		}
	};
}