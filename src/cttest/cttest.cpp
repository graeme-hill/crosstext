// cttest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../ctlib/CrossText.hpp"
#include "../ctlib/DirectWrite.hpp"
#include <Windows.h>

int test1()
{
	ct::DirectWriteRenderOptions options({ 4096, 4096 }, 2);
	ct::TextManager manager(options);

	ct::FontOptions fontOptions1
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		200.0f,
		L"en-US",
		{ 0xf00fffff }
	};

	ct::FontOptions fontOptions2
	{
		L"Times New Roman",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		60.0f,
		L"en-US",
		{ 0xff00ffff }
	};

	ct::FontOptions fontOptions3
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xffff00ff }
	};

	ct::FontRange range{ fontOptions2, { 3, 2 } };

	std::vector<ct::TextBlock> blocks;

	ct::Timer t;

	for (unsigned int i = 0; i < 60; ++i)
	{
		ct::TextBlock test(manager, std::wstring(L"Hello World"), fontOptions1);
		blocks.push_back(std::move(test));
	}

	//for (unsigned int i = 0; i < 1000; ++i)
	//{
	//	ct::TextBlock test(manager, std::wstring(L"Hello World"), fontOptions3);
	//	blocks.push_back(std::move(test));
	//}

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

int test2()
{
	ct::DirectWriteRenderOptions options({ 4096, 4096 }, 4);
	ct::TextManager manager(options);

	std::wstring letters(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

	std::vector<ct::FontOptions> fonts;
	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		60.0f,
		L"en-US",
		{ 0x000fffff }
	});

	fonts.push_back(
	{
		L"Times New Roman",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		40.0f,
		L"en-US",
		{ 0xff99ffff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xff0099ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		20.0f,
		L"en-US",
		{ 0x99ff00ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		22.0f,
		L"en-US",
		{ 0xff9900ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xff0099ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		20.0f,
		L"en-US",
		{ 0x99ff00ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		22.0f,
		L"en-US",
		{ 0xff9900ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xff0099ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		20.0f,
		L"en-US",
		{ 0x99ff00ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		22.0f,
		L"en-US",
		{ 0xff9900ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xff0099ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		20.0f,
		L"en-US",
		{ 0x99ff00ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		22.0f,
		L"en-US",
		{ 0xff9900ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xff0099ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		20.0f,
		L"en-US",
		{ 0x99ff00ff }
	});

	fonts.push_back(
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		22.0f,
		L"en-US",
		{ 0xff9900ff }
	});

	std::vector<ct::TextBlock> blocks;

	double min = 1000000000000000000.0f;
	double max = 0.0f;
	double total = 0.0f;
	int count = 0;

	for (auto i = 0; i < 1200; i++)
	{
		count++;
		auto fontIndex = rand() % fonts.size();
		auto font = fonts[fontIndex];

		auto textLength = 3 + (rand() % 50);
		std::wstring text(L"");
		for (auto i = 0; i < textLength; i++)
		{
			auto charIndex = rand() % letters.size();
			text += letters[charIndex];
		}

		ct::Timer t;
		ct::TextBlock block(manager, text, font);
		auto millis = t.millis();
		if (millis < min) { min = millis; }
		if (millis > max) { max = millis; }
		total += millis;
		blocks.push_back(std::move(block));
		//std::cout << millis << std::endl;
	}

	for (auto i = 0; i < 50000; i++)
	{
		count++;
		auto random = rand() % 2;
		if (random == 0)
		{
			auto fontIndex = rand() % fonts.size();
			auto font = fonts[fontIndex];

			auto textLength = 3 + (rand() % 50);
			std::wstring text(L"");
			for (auto i = 0; i < textLength; i++)
			{
				auto charIndex = rand() % letters.size();
				text += letters[charIndex];
			}

			ct::Timer t;
			ct::TextBlock block(manager, text, font);
			auto millis = t.millis();
			if (millis < min) { min = millis; }
			if (millis > max) { max = millis; }
			total += millis;
			blocks.push_back(std::move(block));
			//std::cout << millis << std::endl;
		}
		else
		{
			auto randomIndex = rand() % blocks.size();
			ct::Timer t;
			blocks.erase(blocks.begin() + randomIndex);
			auto millis = t.millis();
			if (millis < min) { min = millis; }
			if (millis > max) { max = millis; }
			total += millis;
		}
	}

	for (auto i = 0; i < 1200; i++)
	{
		count++;
		auto fontIndex = rand() % fonts.size();
		auto font = fonts[fontIndex];

		auto textLength = 3 + (rand() % 50);
		std::wstring text(L"");
		for (auto i = 0; i < textLength; i++)
		{
			auto charIndex = rand() % letters.size();
			text += letters[charIndex];
		}

		ct::Timer t;
		ct::TextBlock block(manager, text, font);
		auto millis = t.millis();
		if (millis < min) { min = millis; }
		if (millis > max) { max = millis; }
		total += millis;
		blocks.push_back(std::move(block));
		//std::cout << millis << std::endl;
	}

	double avg = total / static_cast<double>(count);
	std::cout << "size=" << blocks.size() << " min=" << min << " max=" << max << " total=" << total << " avg=" << avg << std::endl;

	std::cout << "---" << std::endl;

	std::wstring fileNameWithoutSuffix(L"C:\\temp\\_");
	for (auto &texture : manager.textures())
	{
		texture.imageData().savePng((fileNameWithoutSuffix + std::wstring(L".png")).c_str());
		fileNameWithoutSuffix += L"_";
	}

	return 0;
}

int test3()
{
	ct::DirectWriteRenderOptions options({ 4096, 4096 }, 2);
	ct::TextManager manager(options);

	ct::FontOptions fontOptions1
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xffffffff }
	};

	std::vector<ct::TextBlock> blocks;

	blocks.push_back(ct::TextBlock(manager, std::wstring(L"Hello World"), fontOptions1));

	std::wstring fileNameWithoutSuffix(L"C:\\temp\\_");
	for (auto &texture : manager.textures())
	{
		texture.imageData().savePng((fileNameWithoutSuffix + std::wstring(L".png")).c_str());
		fileNameWithoutSuffix += L"_";
	}

	return 0;
}

int test4()
{
	ct::DirectWriteRenderOptions options({ 4096, 4096 }, 1);
	ct::TextManager manager(options);

	ct::FontOptions fontOptions1
	{
		L"Arial",
		ct::FontWeight::Normal,
		ct::FontStyle::Normal,
		ct::FontStretch::Normal,
		18.0f,
		L"en-US",
		{ 0xffff00ff }
	};

	std::vector<ct::TextBlock> blocks;
	
	double total = 0;
	double min = 0;
	double max = 0;

	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
	blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));

	for (int i = 0; i < 10000; i++)
	{
		ct::Timer t;
		blocks.erase(blocks.begin());
		blocks.push_back(ct::TextBlock(manager, std::wstring(L"This is a sample message. Just want to test what happens when it is relatively long but still all on one line. :-) Already did this once but even then it was still way too short.... I need this to be the worst case scenario in a chat dialog or something like that so I'll just type a bit more things."), fontOptions1));
		auto millis = t.millis();
		total += millis;
		if (millis < min) { min = millis; }
		if (millis > max) { max = millis; }
	}

	auto avg = total / 10000.0;

	std::cout << total << "," << avg << "," << min << "," << max << std::endl;

	std::wstring fileNameWithoutSuffix(L"C:\\temp\\_");
	for (auto &texture : manager.textures())
	{
		texture.imageData().savePng((fileNameWithoutSuffix + std::wstring(L".png")).c_str());
		fileNameWithoutSuffix += L"_";
	}

	return 0;
}

int main()
{
	test3();
}