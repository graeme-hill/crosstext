#include "FreeType.hpp"
#include <iostream>

namespace ct
{
	// FreeTypeRenderOptions

	FreeTypeRenderOptions::FreeTypeRenderOptions() :
		FreeTypeRenderOptions(
			{ DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE },
			DEFAULT_TEXTURE_COUT)
	{ }

	FreeTypeRenderOptions::FreeTypeRenderOptions(Size textureSize, int textureCount) :
		_textureSize(textureSize), _textureCount(textureCount)
	{ }

	// FreeTypeRenderer

	FreeTypeRenderer::FreeTypeRenderer(FreeTypeRenderOptions options) :
		_options(options)
	{
		FT_Init_FreeType(&_library);
	}

	FreeTypeRenderer::~FreeTypeRenderer()
	{
		FT_Done_FreeType(_library);
	}

	// FreeTypeImageData

	FreeTypeImageData::FreeTypeImageData(
		FreeTypeRenderer &renderer, Size size) :
		_renderer(renderer), _size(size)
	{ }

	FreeTypeImageData::FreeTypeImageData(FreeTypeImageData &&other) :
		_renderer(other._renderer), _size(other._size)
	{ }

	void FreeTypeImageData::savePng(std::string path)
	{
		std::cout << "save png to " << path << std::endl;
	}

	// FreeTypeBuilder

	FreeTypeBuilder::FreeTypeBuilder(
		FreeTypeRenderer &renderer,
		std::wstring text,
		TextOptions options) :
		_renderer(renderer),
		_text(text),
		_options(options)
	{ }

	FreeTypeBuilder::~FreeTypeBuilder()
	{ }

	Size FreeTypeBuilder::size() const
	{
		return { 100, 100 };
	}

	void FreeTypeBuilder::render(FreeTypeImageData &imageData, Rect rect)
	{
		std::cout << "render" << std::endl;
	}

	// FreeTypeFont

	FreeTypeFont::FreeTypeFont(std::string path, FreeTypeRenderer &renderer) :
		_face(nullptr)
	{
		auto error = FT_New_Face(renderer.library(), path.c_str(), 0, &_face);
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
}
