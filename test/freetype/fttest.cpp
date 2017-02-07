#include <iostream>
#include "FreeType.hpp"

int test3()
{
	ct::TextManager<ct::FreeTypeSysContext, ct::FreeTypeImageData, ct::FreeTypeFont> manager({ { 256, 256 }, 2});

	auto font1 = manager.loadFont(
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

	ct::Style<ct::FreeTypeFont> style1
	{
		&font1,
		20.0f,
		0x00ff00ff
	};

	std::vector<ct::TextBlock<
		ct::FreeTypeSysContext,
		ct::FreeTypeImageData,
		ct::FreeTypeMetricBuilder,
		ct::FreeTypeCharRenderer,
		ct::FreeTypeFont>> blocks;

	std::wstring str(L"Wi");
	auto textOpt = ct::TextOptions<ct::FreeTypeFont>::fromStyle(style1)
		.withBackground({ 0x00000000 });

	ct::TextBlock<
		ct::FreeTypeSysContext,
		ct::FreeTypeImageData,
		ct::FreeTypeMetricBuilder,
		ct::FreeTypeCharRenderer,
		ct::FreeTypeFont> testBlock(manager, str, textOpt);

	blocks.push_back(std::move(testBlock));

	std::string fileNameWithoutSuffix("C:\\temp\\_");
	for (auto &texture : manager.textures())
	{
		texture.imageData().savePng(fileNameWithoutSuffix + std::string(".png"));
		fileNameWithoutSuffix += "_";
	}

	return 0;
}

int main()
{
    test3();
}
