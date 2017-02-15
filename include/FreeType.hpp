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
			std::cout << "onChar" << std::endl;
			_imageData.write(std::vector<unsigned char>(), Rect());
		}

	private:
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

	class FreeType
	{
		using SysContext = FreeTypeSysContext;

		using MetricBuilder = FreeTypeMetricBuilder;

		template<typename TImageData>
		using CharRenderer = FreeTypeCharRenderer<TImageData>;

		using Font = FreeTypeFont;
	};

	// typedefs to avoid ugly template syntax

	// template <typename TImageData>
	// using FreeTypeTextManager = TextManager<
	// 	FreeTypeSysContext, TImageData, FreeTypeFont>;
}
