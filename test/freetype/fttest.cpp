#include <iostream>
#include "CrossText.hpp"
#include "FreeType.hpp"

int test3()
{
	ct::FreeTypeOptions options({ 256, 256 }, 2);
	ct::TextManager manager(options);

	ct::TFont font1 = manager.loadFont(
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

	ct::Style style1
	{
		&font1,
		20.0f,
		0x00ff00ff
	};

	std::vector<ct::TextBlock> blocks;

	blocks.push_back(ct::TextBlock(manager, std::wstring(L"Wi"),
		ct::TextOptions::fromStyle(style1).withBackground({ 0x00000000 })));

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
