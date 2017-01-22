// cttest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../ctlib/CrossText.hpp"
#include "../ctlib/DirectWrite.hpp"
#include <Windows.h>

int main()
{
	ct::DirectWriteRenderOptions options(ct::Size(4096, 4096), 2);
	ct::TextManager manager(options);

	ct::FontOptions fontOptions1(
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		200.0f,
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
		18.0f,
		L"en-US",
		ct::Color(0xffff00ff));

	ct::FontRange range(fontOptions2, ct::Range(3, 2));

	std::vector<ct::TextBlock> blocks;

	ct::Timer t;

	for (unsigned int i = 0; i < 60; ++i)
	{
		ct::TextBlock test(manager, std::wstring(L"Hello World"), fontOptions1);
		blocks.push_back(std::move(test));
	}

	for (unsigned int i = 0; i < 1000; ++i)
	{
		ct::TextBlock test(manager, std::wstring(L"Hello World"), fontOptions3);
		blocks.push_back(std::move(test));
	}

	auto millis = t.millis();
	std::cout << millis << std::endl;

	std::wstring fileNameWithoutSuffix(L"C:\\temp\\_");
	for (auto &texture : manager.textures())
	{
		texture.imageData().savePng((fileNameWithoutSuffix + std::wstring(L".png")).c_str());
		fileNameWithoutSuffix += L"_";
	}

	return 0;
}

