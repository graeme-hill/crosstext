#include <iostream>
#include "CrossText.hpp"
#include "FreeType.hpp"

int test3()
{
	ct::FreeTypeRenderOptions options({ 4096, 4096 }, 2);
	ct::TextManager manager(options);

	ct::Font font1
	{
		L"Times New Roman",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		40.0f,
		L"en-US",
		{ 0xff0000ff }
	};

	std::vector<ct::TextBlock> blocks;

	blocks.push_back(ct::TextBlock(manager, std::wstring(L"Hello World"),
		ct::TextOptions::fromFont(font1).withBackground({ 0x00000001 })));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"Some other text"),
		ct::TextOptions::fromFont(font1).withBackground({ 0x00000001 })));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"blah blah"),
		ct::TextOptions::fromFont(font1).withBackground({ 0x00000001 })));

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
