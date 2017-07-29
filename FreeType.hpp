#pragma once

#include "CrossText.hpp"
#include <string>
#include <time.h>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define DEFAULT_TEXTURE_SIZE 4096
#define DEFAULT_TEXTURE_COUT 1

BEGIN_XT_NAMESPACE

class FreeTypeSysContext
{
public:
	FreeTypeSysContext(TextManagerOptions options);
	~FreeTypeSysContext();
	Size textureSize() const { return _options.textureSize; }
	FT_Library library() { return _library; }

private:
	TextManagerOptions _options;
	FT_Library _library;
};

class FreeTypeImageData
{
public:
	FreeTypeImageData(FreeTypeSysContext &context, Size size);
	FreeTypeImageData(const FreeTypeImageData &) = delete;
	FreeTypeImageData(FreeTypeImageData &&);
	Size size() const { return _size; }
	void savePng(std::string path);

private:
	FreeTypeSysContext &_context;
	Size _size;
};

class FreeTypeFont
{
public:
	FreeTypeFont(std::string path, FreeTypeSysContext &context);
	FreeTypeFont(const FreeTypeFont &) = delete;
	FreeTypeFont(FreeTypeFont &&other);
	~FreeTypeFont();
	bool isLoaded() { return _face != nullptr; }
	FT_Face face() { return _face; }

private:
	FT_Face _face;
};

class FreeTypeMetricBuilder
{
public:
	FreeTypeMetricBuilder(FreeTypeSysContext &context, Size maxSize);
	FreeTypeMetricBuilder(const FreeTypeMetricBuilder &) = delete;
	FreeTypeMetricBuilder(FreeTypeMetricBuilder &&) = delete;
	void onStyleChange(FreeTypeFont *font, float size, Brush foreground);
	void onChar(
		wchar_t ch, FreeTypeFont *font, float size, Brush foreground);
	TextBlockMetrics done();

private:
	FreeTypeSysContext &_context;
	TextLayout _layout;
};

template <typename TImageData>
class FreeTypeCharRenderer
{
public:
	FreeTypeCharRenderer(
		FreeTypeSysContext &context,
		TImageData &imageData,
		Rect rect,
		TextBlockMetrics &metrics) :
		_penX(rect.x),
		_context(context),
		_imageData(imageData),
		_rect(rect),
		_metrics(metrics),
		_row(0),
		_column(0)
	{ }

	FreeTypeCharRenderer(const FreeTypeCharRenderer &) = delete;

	FreeTypeCharRenderer(FreeTypeCharRenderer &&) = delete;

	void onStyleChange(FreeTypeFont *font, float size, Brush foreground)
	{
		FT_Set_Char_Size(font->face(), 0, size*64.0, 100, 100);
	}

	void onChar(
		wchar_t ch, FreeTypeFont *font, float size, Brush foreground)
	{
		auto face = font->face();
		auto glyphIndex = FT_Get_Char_Index(face, ch);
		auto error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);

		if (error)
		{
			std::cout << "ERROR: failed to load glyph" << std::endl;
		}

		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		if (error)
		{
			std::cout << "ERROR: failed to render glyph" << std::endl;
		}

		uint8_t r = foreground.color.redByte();
		uint8_t g = foreground.color.greenByte();
		uint8_t b = foreground.color.blueByte();
		uint8_t a = foreground.color.alphaByte();

		auto lineMetrics = _metrics.lines[_row];

		unsigned effectivePenY =
			lineMetrics.baseline - face->glyph->bitmap_top + _rect.y;
		unsigned effectivePenX = _penX + face->glyph->bitmap_left;

		auto bitmap = face->glyph->bitmap;

		auto maxWidth = std::min(
			bitmap.width, _imageData.size().width - effectivePenX);
		auto maxHeight = std::min(
			bitmap.rows, _imageData.size().height - effectivePenY);

		for (unsigned y = 0; y < maxHeight; y++)
		{
			for (unsigned x = 0; x < maxWidth; x++)
			{
				auto realX = x + effectivePenX;
				auto realY = y + effectivePenY;

				auto ftalpha = bitmap.buffer[y * bitmap.width + x];
				auto ftalphaf = static_cast<float>(ftalpha) / 255.0f;
				auto finalAlpha = static_cast<unsigned>(
					ftalphaf * static_cast<float>(a));
				_imageData.setPixel(realX, realY, r, g, b, finalAlpha);
			}
		}

		_penX += face->glyph->advance.x >> 6;

		_column += 1;
		if (lineMetrics.chars <= _column)
		{
			_column = 0;
			_row += 1;
			_penX = _rect.x;
		}
	}

private:
	unsigned _penX;
	FreeTypeSysContext &_context;
	TImageData &_imageData;
	Rect _rect;
	TextBlockMetrics &_metrics;
	unsigned _row;
	unsigned _column;
};

class LinuxTimer
{
public:
	LinuxTimer()
	{
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &_start);
	}

	double millis()
	{
		struct timespec end;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		long diff = end.tv_nsec - _start.tv_nsec;
		return static_cast<float>(diff) / 1000.0f;
	}

private:
	struct timespec _start;
};

template <typename TImageData>
class FreeType
{
public:
	using SysContext = FreeTypeSysContext;
	using MetricBuilder = FreeTypeMetricBuilder;
	using CharRenderer = FreeTypeCharRenderer<TImageData>;
	using Font = FreeTypeFont;
	using ImageData = TImageData;
};

END_XT_NAMESPACE
