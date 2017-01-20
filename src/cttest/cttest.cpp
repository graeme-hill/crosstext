// cttest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../ctlib/CrossText.hpp"
#include "../ctlib/DirectWrite.hpp"
#include <Windows.h>

int main()
{
	ct::DirectWriteRenderOptions options(ct::Size(1024, 1024), 2);
	ct::TextManager manager(options);

	ct::FontOptions fontOptions1(
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		20.0f,
		L"en-US",
		ct::Color(0xf00fffff));

	ct::FontOptions fontOptions2(
		L"Times New Roman",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		60.0f,
		L"en-US",
		ct::Color(0xff00ffff));

	ct::FontOptions fontOptions3(
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		14.0f,
		L"en-US",
		ct::Color(0xffff00ff));

	ct::FontRange range(fontOptions2, ct::Range(3, 2));

	std::vector<ct::TextBlock> blocks;

	ct::TextBlock testa(manager, std::wstring(L"Graeme"), fontOptions1, { range });

	for (unsigned int i = 0; i < 16; ++i)
	{
		ct::TextBlock test(manager, std::wstring(L"Graeme"), fontOptions1, { range });
		blocks.push_back(std::move(test));
	}

	std::wstring fileNameWithoutSuffix(L"C:\\temp\\_");
	for (auto &texture : manager.textures())
	{
		texture.imageData().savePng((fileNameWithoutSuffix + std::wstring(L".png")).c_str());
		fileNameWithoutSuffix += L"_";
	}

	return 0;
}

