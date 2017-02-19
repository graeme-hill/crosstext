#pragma once

#include "CrossText.hpp"
#include <string>
#include <time.h>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define DEFAULT_TEXTURE_SIZE 4096
#define DEFAULT_TEXTURE_COUT 1

namespace ct
{
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
		int _penX;
		int _penY;
		int _currentWidth;
		std::vector<LineMetrics> _lines;
		Size _maxSize;
	};

	template <typename TImageData>
	class FreeTypeCharRenderer
	{
	public:
		FreeTypeCharRenderer(
			FreeTypeSysContext &context,
			TImageData &imageData,
			Rect rect) :
			_penX(0),
			_penY(0),
			_context(context),
			_imageData(imageData),
			_rect(rect)
		{ }

		FreeTypeCharRenderer(const FreeTypeCharRenderer &) = delete;

		FreeTypeCharRenderer(FreeTypeCharRenderer &&) = delete;

		void onStyleChange(FreeTypeFont *font, float size, Brush foreground)
		{ }

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

			int effectivePenY = _penY - face->glyph->bitmap_top;
			int effectivePenX = _penX + face->glyph->bitmap_left;

			uint8_t r = foreground.color.redByte();
			uint8_t g = foreground.color.greenByte();
			uint8_t b = foreground.color.blueByte();
			uint8_t a = foreground.color.alphaByte();

			auto bitmap = face->glyph->bitmap;
			for (unsigned int y = 0; y < bitmap.rows; y++)
			{
				for (unsigned int x = 0; x < bitmap.width; x++)
				{
					int ftalpha = bitmap.buffer[y * bitmap.width + x];
					float ftalphaf = static_cast<float>(ftalpha) / 255.0f;
					int finalAlpha = static_cast<int>(
						ftalphaf * static_cast<float>(a));
					_imageData.setPixel(x, y, r, g, b, finalAlpha);
				}
			}

			std::cout << "onChar" << std::endl;
			_imageData.write(std::vector<uint8_t>(), Rect());
		}

	private:
		int _penX;
		int _penY;
		FreeTypeSysContext &_context;
		TImageData &_imageData;
		Rect _rect;
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
}
