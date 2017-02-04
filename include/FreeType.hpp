#pragma once

#include "Common.hpp"
#include <string>
#include <time.h>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define DEFAULT_TEXTURE_SIZE 4096
#define DEFAULT_TEXTURE_COUT 1

namespace ct
{
	class FreeTypeOptions
	{
	public:
		FreeTypeOptions();
		FreeTypeOptions(Size textureSize, int textureCount);
		Size textureSize() const { return _textureSize; }
		int textureCount() const { return _textureCount; }

	private:
		Size _textureSize;
		int _textureCount;
	};

	class FreeTypeSysContext
	{
	public:
		FreeTypeSysContext(FreeTypeOptions options);
		~FreeTypeSysContext();
		Size textureSize() const { return _options.textureSize(); }
		FT_Library library() { return _library; }

	private:
		FreeTypeOptions _options;
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

	private:
		FT_Face _face;
	};

	class FreeTypeMetricBuilder
	{
	public:
		FreeTypeMetricBuilder(FreeTypeSysContext &context);
		FreeTypeMetricBuilder(const FreeTypeMetricBuilder &) = delete;
		FreeTypeMetricBuilder(FreeTypeMetricBuilder &&) = delete;
		void next(wchar_t ch, FreeTypeFont *font, float size);
		TextBlockMetrics result();

	private:
		int _penX;
		int _penY;
		Size _currentSize;
		std::vector<int> _baselines;
	};

	class FreeTypeCharRenderer
	{
	public:
		FreeTypeCharRenderer(
			FreeTypeSysContext &context,
			FreeTypeImageData &imageData,
			Rect rect);
		FreeTypeCharRenderer(const FreeTypeCharRenderer &) = delete;
		FreeTypeCharRenderer(FreeTypeCharRenderer &&) = delete;
		void next(wchar_t ch, FreeTypeFont *font, float size, Brush foreground);
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
}
