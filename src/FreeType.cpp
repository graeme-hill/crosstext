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
		auto error = FT_Init_FreeType(&_library);
		if (error)
		{
			std::cout << "failed to init freetype" << std::endl;
		}
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
			std::cout << "failed to load font '" << path << "'" << std::endl;
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

	FreeTypeMetricBuilder::FreeTypeMetricBuilder(
		FreeTypeSysContext &context,
		Size maxSize) :
		_context(context),
		_penX(0),
		_penY(0),
		_currentWidth(0),
		_maxSize(maxSize)
	{ }

	void FreeTypeMetricBuilder::onStyleChange(
		FreeTypeFont *font, float size, Brush foreground)
	{
		std::cout << "font size: " << size << std::endl;
		FT_Set_Char_Size(font->face(), 0, size*64.0, 100, 100);
	}

	void FreeTypeMetricBuilder::onChar(
		wchar_t ch, FreeTypeFont *font, float size, Brush foreground)
	{
		auto face = font->face();
		auto glyphIndex = FT_Get_Char_Index(face, ch);
		FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		auto fontMetrics = face->size->metrics;
		auto charMetrics = face->glyph->metrics;
		auto charWidth = static_cast<int>(charMetrics.horiAdvance >> 6);
		auto fontHeight = static_cast<int>(fontMetrics.height >> 6);

		auto ax = face->glyph->advance.x >> 6;
		auto ay = face->glyph->advance.y >> 6;

		auto isFirstChar = _lines.empty();
		auto isFirstCharOnThisLine = _penX == 0;
		auto tooBigToFitOnThisLine = charWidth + _penX > _maxSize.width;

		std::wcout << L"'" << ch << L"' " << charWidth << L"x" << fontHeight << " or " << ax << "," << ay << std::endl;

		//std::cout << isFirstChar << "," << isFirstCharOnThisLine << ""

		if (isFirstChar || (!isFirstCharOnThisLine && tooBigToFitOnThisLine))
		{
			std::cout << "new line" << std::endl;
			_penX = 0;
			if (!isFirstChar)
			{
				_penY += fontHeight;
				_currentWidth = _maxSize.width;
			}
			_lines.push_back({ 0, 0 });
		}

		std::cout << "penX " << _penX << " -> ";

		_penX += charWidth;

		std::cout << _penX << std::endl;
		auto endX = std::min(_maxSize.width, _penX - 1);
		std::cout << "_currentWidth is max of " << _currentWidth << " and " << endX << std::endl;
		_currentWidth = std::max(_currentWidth, endX);
		auto &currentLine = _lines[_lines.size() - 1];
		std::cout << "currentLine.height is max of " << currentLine.height << " and " << fontHeight << std::endl;
		currentLine.height = std::max(currentLine.height, fontHeight);
		currentLine.baseline = currentLine.height;
	}

	TextBlockMetrics FreeTypeMetricBuilder::done()
	{
		int height = 0;
		for (auto &metric : _lines)
		{
			height += metric.height;
		}
		std::cout << "RESULT " << _currentWidth << ", " << height << std::endl;
		return { { _currentWidth, height }, std::move(_lines) };
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

	void FreeTypeCharRenderer::onStyleChange(
		FreeTypeFont *font, float size, Brush foreground)
	{ }

	void FreeTypeCharRenderer::onChar(
		wchar_t ch, FreeTypeFont *font, float size, Brush foreground)
	{
		std::cout << "render char" << std::endl;
	}
}
