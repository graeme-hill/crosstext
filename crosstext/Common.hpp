#pragma once

#include <vector>
#include <string>

#define FLOAT_ERROR_MARGIN 0.01f

namespace ct
{
	inline bool equalish(float a, float b)
	{
		auto diff = abs(a - b);
		return diff < FLOAT_ERROR_MARGIN;
	}

	inline bool greaterish(float a, float b)
	{
		return (a + FLOAT_ERROR_MARGIN) > b;
	}

	inline bool lessish(float a, float b)
	{
		return (a - FLOAT_ERROR_MARGIN) < b;
	}

	inline bool betweenish(float value, float start, float end)
	{
		return start - FLOAT_ERROR_MARGIN < value && end + FLOAT_ERROR_MARGIN > value;
	}

	class Size
	{
	public:
		Size(float width, float height) : 
			_width(width), _height(height)
		{ }

		Size() : Size(0.0f, 0.0f)
		{ }

		float width() const { return _width; }
		float height() const { return _height; }

	private:
		float _width;
		float _height;
	};

	class Rect
	{
	public:
		Rect() :
			Rect(0.0f, 0.0f, 0.0f, 0.0f)
		{ }

		Rect(float x, float y, Size size) :
			Rect(x, y, size.width(), size.height())
		{ }

		Rect(float x, float y, float width, float height)
			: _x(x), _y(y), _width(width), _height(height)
		{ }

		float x() const { return _x; }
		float y() const { return _y; }
		float width() const { return _width; }
		float height() const { return _height; }
		Size size() const { return Size(_width, _height); }
		float endX() const { return _x + _width; }
		float endY() const { return _y + _height; }

	private:
		float _x;
		float _y;
		float _width;
		float _height;
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

	class FontOptions
	{
	public:
		FontOptions(
			std::wstring family,
			FontWeight weight,
			FontStyle style,
			FontStretch stretch,
			float size,
			std::wstring locale)
			:
			_family(family),
			_weight(weight),
			_style(style),
			_stretch(stretch),
			_size(size),
			_locale(locale)
		{ }

		std::wstring &family() { return _family; }
		FontWeight weight() { return _weight; }
		FontStyle style() { return _style; }
		FontStretch stretch() { return _stretch; }
		float size() { return _size; }
		std::wstring &locale() { return _locale; }

	private:
		std::wstring _family;
		FontWeight _weight;
		FontStyle _style;
		FontStretch _stretch;
		float _size;
		std::wstring _locale;
	};

	class Color
	{
	public:
		Color(uint32_t rgba) : _rgba(rgba)
		{ }

		float redf()
		{
			return (_rgba >> 24) / 255.0f;
		}

		float greenf()
		{
			return ((_rgba & 0x00ff0000) >> 16) / 255.0f;
		}

		float bluef()
		{
			return ((_rgba & 0x0000ff00) >> 8) / 255.0f;
		}

		float alphaf()
		{
			return (_rgba & 0x000000ff) / 255.0f;
		}

	private:
		uint32_t _rgba;
	};

	class Brush
	{
	public:
		Brush(Color color) : _color(color)
		{ }

		Color &color() { return _color; }

	private:
		Color _color;
	};

	class Text
	{
	public:
		Text(std::wstring string) : /*, FontOptions fontOptions*/
			_string(string) //, _fontOptions(fontOptions)
		{ }
		std::wstring &string() { return _string; }
		//FontOptions &fontOptions() { return _fontOptions; }

	private:
		std::wstring _string;
		//FontOptions _fontOptions;
	};
}