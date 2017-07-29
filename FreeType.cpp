#include "FreeType.hpp"
#include <iostream>

BEGIN_XT_NAMESPACE

// FreeTypeSysContext

FreeTypeSysContext::FreeTypeSysContext(TextManagerOptions options) :
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
	_layout(maxSize)
{ }

void FreeTypeMetricBuilder::onStyleChange(
	FreeTypeFont *font, float size, Brush foreground)
{
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
	auto charWidth = static_cast<unsigned>(charMetrics.horiAdvance >> 6);
	auto fontHeight = static_cast<unsigned>(fontMetrics.height >> 6);

	_layout.nextChar(ch, { charWidth, fontHeight }, 0);
}

TextBlockMetrics FreeTypeMetricBuilder::done()
{
	return _layout.metrics();
}

END_XT_NAMESPACE
