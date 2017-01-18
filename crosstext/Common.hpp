#pragma once

#include <vector>
#include <string>

#define FLOAT_ERROR_MARGIN 0.01f

namespace ct
{
	class Size
	{
	public:
		Size(int width, int height) : 
			_width(width), _height(height)
		{ }

		Size() : Size(0, 0)
		{ }

		int width() const { return _width; }
		int height() const { return _height; }

	private:
		int _width;
		int _height;
	};

	class Rect
	{
	public:
		Rect() :
			Rect(0, 0, 0, 0)
		{ }

		Rect(int x, int y, Size size) :
			Rect(x, y, size.width(), size.height())
		{ }

		Rect(int x, int y, int width, int height)
			: _x(x), _y(y), _width(width), _height(height)
		{ }

		int x() const { return _x; }
		int y() const { return _y; }
		int width() const { return _width; }
		int height() const { return _height; }
		Size size() const { return Size(_width, _height); }
		int endX() const { return _x + _width - 1; }
		int endY() const { return _y + _height - 1; }

	private:
		int _x;
		int _y;
		int _width;
		int _height;
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

	class Range
	{
	public:
		Range(unsigned int start, unsigned int length) :
			_start(start), _length(length)
		{ }
		Range() :
			Range(0, 0)
		{ }

		unsigned int start() const { return _start; }
		unsigned int length() const { return _length; }

	private:
		unsigned int _start;
		unsigned int _length;
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

	class FontOptions
	{
	public:
		FontOptions(
			std::wstring family,
			FontWeight weight,
			FontStyle style,
			FontStretch stretch,
			float size,
			std::wstring locale,
			Brush foreground)
			:
			_family(family),
			_weight(weight),
			_style(style),
			_stretch(stretch),
			_size(size),
			_locale(locale),
			_foreground(foreground)
		{ }

		std::wstring &family() { return _family; }
		FontWeight weight() { return _weight; }
		FontStyle style() { return _style; }
		FontStretch stretch() { return _stretch; }
		float size() { return _size; }
		std::wstring &locale() { return _locale; }
		Brush foreground() { return _foreground; }

	private:
		std::wstring _family;
		FontWeight _weight;
		FontStyle _style;
		FontStretch _stretch;
		float _size;
		std::wstring _locale;
		Brush _foreground;
	};

	class FontRange
	{
	public:
		FontRange(FontOptions fontOptions, Range range) :
			_fontOptions(fontOptions), _range(range)
		{ }
		
		FontOptions fontOptions() const { return _fontOptions; }
		Range range() const { return _range; }

	private:
		FontOptions _fontOptions;
		Range _range;
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