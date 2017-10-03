#include <iostream>
#include "FreeType.hpp"
#include "LibPngWriter.hpp"

/*
TODO: FreeTypeMetricBuilder, FreeTypeCharRenderer, and FreeTypeFont are
tightly coupled to FreeTypeSysContext so infoPtrthey should ne inner classes with
conventional names across platforms like ::MetricBuilder, ::CharRenderer, etc.
so that they can just be accessed as TSysContext::MetricBuilder instead of
having their own template parameters. The only type parameters that client code
needs access to are TSysContext and TImageData.

TSysContext could be renamed something like TPlatform.
TImageData could be renamed something like TWriter.

The only thing that client code should have to do differently per platform is
construct their text manager. eg:

#ifdef LINUX
using TextManager = xt::TextManager<xt::FreeType, xt::OpenGlWriter>;
#elif WINDOWS
using TextManager = xt::TextManager<xt::DirectWrite, xt::DirectXWriter>;
#elif MAC
using TextManager = xt::TextManager<xt::CoreText, xt::OpenGlWriter>;
#endif
using TextBlock = TextManager::TextBlock;
using Font = TextManager::Font;

Text manager({ 256, 256 }, 2);
Text::Font font1 = manager.loadFont("asdf");
Text::Style style1
{
	&font1,
	20.0f,
	0x00ff00ff
};
Text::Block
*/

using Text = xt::TextPlatform<xt::FreeType<xt::LibPngWriter>>;

int test3()
{
	Text::ImageData t1({1024, 1024}, "./one_");
	Text::ImageData t2({1024, 1024}, "./two_");

	std::vector<Text::ImageData> textureWriters;
	textureWriters.push_back(std::move(t1));
	textureWriters.push_back(std::move(t2));

	Text::Manager manager(
		{ { 1024, 1024 } },
		std::move(textureWriters));

	auto font1 = manager.loadFont(
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

	Text::Style style1
	{
		&font1,
		40.0f,
		0x000000ff
	};

	Text::Style style2
	{
		&font1,
		20.0f,
		0xffff00ff
	};

	std::vector<Text::Block> blocks;

	std::wstring str1(L"Oops I don't think these characters are quite lined up...");
	std::wstring str2(L"ShortOne");
	std::wstring str3(L"ShortTwo");
	std::wstring str4(L"mini");
	auto textOpt1 = Text::Options::fromStyle(style1)
		.withBackground({ 0x00000000 })
		.withStyleRanges({ { style2, { 2, 2 } } });
	auto textOpt2 = Text::Options::fromStyle(style2);

	blocks.push_back(Text::Block(manager, str1, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt2));
	blocks.push_back(Text::Block(manager, str3, textOpt2));
	blocks.push_back(Text::Block(manager, str1, textOpt2));
	blocks.push_back(Text::Block(manager, str2, textOpt2));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str1, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str1, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str1, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str1, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str1, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str1, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str2, textOpt1));
	blocks.push_back(Text::Block(manager, str3, textOpt1));
	blocks.push_back(Text::Block(manager, str4, textOpt1));
	blocks.push_back(Text::Block(manager, str4, textOpt1));
	blocks.push_back(Text::Block(manager, str4, textOpt1));
	blocks.push_back(Text::Block(manager, str4, textOpt1));
	blocks.push_back(Text::Block(manager, str4, textOpt1));

	return 0;
}

int main()
{
    test3();
}
