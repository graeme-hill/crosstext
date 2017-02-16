#include <iostream>
#include "FreeType.hpp"
#include "LibPngWriter.hpp"

/*
TODO: FreeTypeMetricBuilder, FreeTypeCharRenderer, and FreeTypeFont are
tightly coupled to FreeTypeSysContext so they should ne inner classes with
conventional names across platforms like ::MetricBuilder, ::CharRenderer, etc.
so that they can just be accessed as TSysContext::MetricBuilder instead of
having their own template parameters. The only type parameters that client code
needs access to are TSysContext and TImageData.

TSysContext could be renamed something like TPlatform.
TImageData could be renamed something like TWriter.

The only thing that client code should have to do differently per platform is
construct their text manager. eg:

#ifdef LINUX
using TextManager = ct::TextManager<ct::FreeType, ct::OpenGlWriter>;
#elif WINDOWS
using TextManager = ct::TextManager<ct::DirectWrite, ct::DirectXWriter>;
#elif MAC
using TextManager = ct::TextManager<ct::CoreText, ct::OpenGlWriter>;
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

using Text = ct::TextPlatform<ct::FreeType<ct::LibPngWriter>>;

int test3()
{
	ct::LibPngWriter t1({256, 256});
	ct::LibPngWriter t2({256, 256});

	std::vector<ct::LibPngWriter> textureWriters;
	textureWriters.push_back(std::move(t1));
	textureWriters.push_back(std::move(t2));

	Text::Manager manager(
		{ { 256, 256 } },
		std::move(textureWriters));

	auto font1 = manager.loadFont(
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

	Text::Style style1
	{
		&font1,
		20.0f,
		0x00ff00ff
	};

	std::vector<Text::Block> blocks;

	std::wstring str(L"Wi");
	auto textOpt = Text::Options::fromStyle(style1)
		.withBackground({ 0x00000000 });

	Text::Block testBlock(manager, str, textOpt);

	blocks.push_back(std::move(testBlock));

	return 0;
}

int main()
{
    test3();
}
