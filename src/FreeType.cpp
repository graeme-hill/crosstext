#include "FreeType.hpp"
#include <iostream>

namespace ct
{
	// FreeTypeOptions

	FreeTypeOptions::FreeTypeOptions() :
		FreeTypeOptions(
			{ DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE },
			DEFAULT_TEXTURE_COUT)
	{ }

	FreeTypeOptions::FreeTypeOptions(Size textureSize, int textureCount) :
		_textureSize(textureSize), _textureCount(textureCount)
	{ }

	// FreeTypeSysContext

	FreeTypeSysContext::FreeTypeSysContext(FreeTypeOptions options) :
		_options(options)
	{
		FT_Init_FreeType(&_library);
	}

	FreeTypeSysContext::~FreeTypeSysContext()
	{
		FT_Done_FreeType(_library);
	}

	// FreeTypeImageData

	FreeTypeImageData::FreeTypeImageData(
		FreeTypeSysContext &context, Size size) :
		_context(context), _size(size)
	{ }

	FreeTypeImageData::FreeTypeImageData(FreeTypeImageData &&other) :
		_context(other._context), _size(other._size)
	{ }

	void FreeTypeImageData::savePng(std::string path)
	{
		std::cout << "save png to " << path << std::endl;
	}

	// FreeTypeFont

	FreeTypeFont::FreeTypeFont(std::string path, FreeTypeSysContext &context) :
		_face(nullptr)
	{
		auto error = FT_New_Face(context.library(), path.c_str(), 0, &_face);
		if (error)
		{
			_face = nullptr;
		}
	}

	FreeTypeFont::FreeTypeFont(FreeTypeFont &&other) :
		_face(other._face)
	{
		other._face = nullptr;
	}

	FreeTypeFont::~FreeTypeFont()
	{
		if (_face)
		{
			FT_Done_Face(_face);
		}
	}

	// FreeTypeMetricBuilder

	FreeTypeMetricBuilder::FreeTypeMetricBuilder(FreeTypeSysContext &context) :
		_context(context), _penX(0), _penY(0), _currentSize{ 0, 0 }
	{
		_baselines.push_back(10);
	}

	void FreeTypeMetricBuilder::next(wchar_t ch, FreeTypeFont *font, float size)
	{
		std::cout << "next" << std::endl;
		_penX += 10;
		_currentSize.width += 10;
		_currentSize.height = 10;
	}

	TextBlockMetrics FreeTypeMetricBuilder::result()
	{
		return { _currentSize, std::move(_baselines) };
	}

	// FreeTypeCharRenderer

	FreeTypeCharRenderer::FreeTypeCharRenderer(
		FreeTypeSysContext &context,
		FreeTypeImageData &imageData,
		Rect rect) :
		_context(context),
		_imageData(imageData),
		_rect(rect)
	{ }

	void FreeTypeCharRenderer::next(
		wchar_t ch, FreeTypeFont *font, float size, Brush foreground)
	{
		std::cout << "render char" << std::endl;
	}
}
