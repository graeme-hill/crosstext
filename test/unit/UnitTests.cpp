#include <string>
#include <iostream>
#include <functional>
#include "CrossText.hpp"

using namespace xt;

unsigned totalFailures = 0;
unsigned totalAssertions = 0;

template <typename T>
void recordAssertionResult(std::string desc, bool passed, T expected, T actual)
{
	totalAssertions += 1;
	totalFailures += passed ? 0 : 1;

	std::string label(passed ? "  PASS " : "x FAIL ");

	std::cout << label << desc
		<< " -> expected: <" << expected
		<< "> actual: <" << actual
		<< ">" << std::endl;
}

template <typename T>
void assertEqual(std::string desc, T expected, T actual)
{
	recordAssertionResult(desc, actual == expected, expected, actual);
}

void assertTrue(std::string desc, bool test)
{
	assertEqual(desc, true, test);
}

void test(std::string desc, std::function<void()> action)
{
	std::cout << std::endl << desc << std::endl;
	try
	{
		action();
	}
	catch (...)
	{
		totalFailures += 1;
		std::cout << "Test threw exception: " << desc << std::endl;
	}
}

int summary()
{
	if (totalFailures == 0)
	{
		std::cout << std::endl << "Success :) " << std::endl;
	}
	else
	{
		std::cout << std::endl << "No bueno :( " << std::endl;
	}

	auto passed = totalAssertions - totalFailures;
	std::cout << passed << "/" << totalAssertions << " passed" << std::endl;

	return totalFailures;
}

void applyChars(
	TextLayout &layout, std::wstring text, Size size, unsigned kerning)
{
	for (auto &ch : text)
	{
		layout.nextChar(ch, size, kerning);
	}
}

int main()
{
	// TextLayout

	test("TextLayout: no chars", []()
	{
		TextLayout layout({ 100, 100 });
		auto metrics = layout.metrics();

		assertEqual("width", 0u, metrics.size.width);
		assertEqual("height", 0u, metrics.size.height);
		assertEqual("line count", 0u, metrics.size.height);
	});

	test("TextLayout: one char", []()
	{
		TextLayout layout({ 100, 100 });
		layout.nextChar(L"x"[0], { 20, 30 }, 0);
		auto metrics = layout.metrics();

		assertEqual("width", 20u, metrics.size.width);
		assertEqual("height", 30u, metrics.size.height);
		assertEqual("line count", size_t{1}, metrics.lines.size());
		assertEqual("line char count", 1u, metrics.lines.at(0).chars);
		assertEqual("line height", 30u, metrics.lines.at(0).height);
	});

	test("TextLayout: one long word", []()
	{
		TextLayout layout({ 30, 30 });
		applyChars(layout, L"wasd", { 8, 8 }, 0);
		auto metrics = layout.metrics();

		assertEqual("width", 30u, metrics.size.width);
		assertEqual("height", 16u, metrics.size.height);
		assertEqual("line count", size_t{2}, metrics.lines.size());
		assertEqual("1st line char count", 3u, metrics.lines.at(0).chars);
		assertEqual("2nd line char count", 1u, metrics.lines.at(1).chars);
		assertEqual("1st line height", 8u, metrics.lines.at(0).height);
		assertEqual("2nd line height", 8u, metrics.lines.at(1).height);
	});

	test("TextLayout: space at end of line", []()
	{
		TextLayout layout({ 30, 30 });
		applyChars(layout, L"was d", { 8, 8 }, 0);
		auto metrics = layout.metrics();

		assertEqual("width", 30u, metrics.size.width);
		assertEqual("height", 16u, metrics.size.height);
		assertEqual("line count", size_t{2}, metrics.lines.size());
		assertEqual("1st line char count", 4u, metrics.lines.at(0).chars);
		assertEqual("2nd line char count", 1u, metrics.lines.at(1).chars);
		assertEqual("1st line height", 8u, metrics.lines.at(0).height);
		assertEqual("2nd line height", 8u, metrics.lines.at(1).height);
	});

	test("TextLayout: space before end of line", []()
	{
		TextLayout layout({ 45, 45 });
		applyChars(layout, L"was qwe", { 8, 8 }, 0);
		auto metrics = layout.metrics();

		assertEqual("width", 45u, metrics.size.width);
		assertEqual("height", 16u, metrics.size.height);
		assertEqual("line count", size_t{2}, metrics.lines.size());
		assertEqual("1st line char count", 4u, metrics.lines.at(0).chars);
		assertEqual("2nd line char count", 3u, metrics.lines.at(1).chars);
		assertEqual("1st line height", 8u, metrics.lines.at(0).height);
		assertEqual("2nd line height", 8u, metrics.lines.at(1).height);
	});

	test("TextLayout: wrapping sentence with weird whitespace", []()
	{
		TextLayout layout({ 150, 100 });
		applyChars(
			layout,
			L"xxx x   xxx xxxxx xxx xxxxx x xxxxxxxxxx         x",
			//^           ^               ^              ^
			{ 10, 10 },
			0);
		auto metrics = layout.metrics();

		assertEqual("width", 150u, metrics.size.width);
		assertEqual("height", 40u, metrics.size.height);
		assertEqual("line count", size_t{4}, metrics.lines.size());
		assertEqual("1st line char count", 12u, metrics.lines.at(0).chars);
		assertEqual("2nd line char count", 16u, metrics.lines.at(1).chars);
		assertEqual("3rd line char count", 15u, metrics.lines.at(2).chars);
		assertEqual("4th line char count", 7u, metrics.lines.at(3).chars);
		assertEqual("1st line height", 10u, metrics.lines.at(0).height);
		assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
		assertEqual("3rd line height", 10u, metrics.lines.at(2).height);
		assertEqual("4th line height", 10u, metrics.lines.at(3).height);
	});

	test("TextLayout: one very long word", []()
	{
		TextLayout layout({ 105, 105 });
		applyChars(
			layout,
			L"hello world this is a loooooooooooooooooooooooooooong word :-)",
			//^     ^          ^    ^         ^         ^         ^
			{ 10, 10 },
			0);
		auto metrics = layout.metrics();

		assertEqual("width", 105u, metrics.size.width);
		assertEqual("height", 70u, metrics.size.height);
		assertEqual("line count", size_t{7}, metrics.lines.size());
		assertEqual("1st line char count", 6u, metrics.lines.at(0).chars);
		assertEqual("2nd line char count", 11u, metrics.lines.at(1).chars);
		assertEqual("3rd line char count", 5u, metrics.lines.at(2).chars);
		assertEqual("4nd line char count", 10u, metrics.lines.at(3).chars);
		assertEqual("5th line char count", 10u, metrics.lines.at(4).chars);
		assertEqual("6nd line char count", 10u, metrics.lines.at(5).chars);
		assertEqual("7st line char count", 10u, metrics.lines.at(6).chars);
		assertEqual("1st line height", 10u, metrics.lines.at(0).height);
		assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
		assertEqual("3rd line height", 10u, metrics.lines.at(2).height);
		assertEqual("4nd line height", 10u, metrics.lines.at(3).height);
		assertEqual("5sh line height", 10u, metrics.lines.at(4).height);
		assertEqual("6nd line height", 10u, metrics.lines.at(5).height);
		assertEqual("7st line height", 10u, metrics.lines.at(6).height);
	});

	test("TextLayout: one very long chunk of whitespace", []()
	{
		TextLayout layout({ 105, 105 });
		applyChars(
			layout,
			L"hello world this is a                                  word :-)",
			//^     ^          ^         ^         ^         ^       ^
			{ 10, 10 },
			0);
		auto metrics = layout.metrics();

		assertEqual("width", 105u, metrics.size.width);
		assertEqual("height", 70u, metrics.size.height);
		assertEqual("line count", size_t{7}, metrics.lines.size());
		assertEqual("1st line char count", 6u, metrics.lines.at(0).chars);
		assertEqual("2nd line char count", 11u, metrics.lines.at(1).chars);
		assertEqual("3rd line char count", 10u, metrics.lines.at(2).chars);
		assertEqual("4th line char count", 10u, metrics.lines.at(3).chars);
		assertEqual("5th line char count", 10u, metrics.lines.at(4).chars);
		assertEqual("6th line char count", 8u, metrics.lines.at(5).chars);
		assertEqual("7th line char count", 8u, metrics.lines.at(6).chars);
		assertEqual("1st line height", 10u, metrics.lines.at(0).height);
		assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
		assertEqual("3rd line height", 10u, metrics.lines.at(2).height);
		assertEqual("4th line height", 10u, metrics.lines.at(3).height);
		assertEqual("5th line height", 10u, metrics.lines.at(4).height);
		assertEqual("6th line height", 10u, metrics.lines.at(5).height);
		assertEqual("7th line height", 10u, metrics.lines.at(6).height);
	});

	test("TextLayout: wrap word with leading whitespace", []()
	{
		TextLayout layout({ 105, 105 });
		applyChars(
			layout,
			L"      hello            world",
			//^     ^         ^      ^
			{ 10, 10 },
			0);
		auto metrics = layout.metrics();

		assertEqual("width", 105u, metrics.size.width);
		assertEqual("height", 40u, metrics.size.height);
		assertEqual("line count", size_t{4}, metrics.lines.size());
		assertEqual("1st line char count", 6u, metrics.lines.at(0).chars);
		assertEqual("2nd line char count", 10u, metrics.lines.at(1).chars);
		assertEqual("3rd line char count", 7u, metrics.lines.at(2).chars);
		assertEqual("4th line char count", 5u, metrics.lines.at(3).chars);
		assertEqual("1st line height", 10u, metrics.lines.at(0).height);
		assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
		assertEqual("3rd line height", 10u, metrics.lines.at(2).height);
		assertEqual("4th line height", 10u, metrics.lines.at(3).height);
	});

	test("TextLayout: various char sizes", []()
	{
		TextLayout layout({ 100, 100 });
		applyChars(layout, L"as", { 10, 20 }, 0);
		applyChars(layout, L"D", { 20, 30 }, 0);
		applyChars(layout, L"fg", { 5, 10 }, 0);
		auto metrics = layout.metrics();
		assertEqual("width", 50u, metrics.size.width);
		assertEqual("height", 30u, metrics.size.height);
		assertEqual("line count", size_t{1}, metrics.lines.size());
		assertEqual("1st line char count", 5u, metrics.lines.at(0).chars);
		assertEqual("1st line height", 30u, metrics.lines.at(0).height);
	});

	test("TextLayout: various char sizes multi-line", []()
	{
		TextLayout layout({ 100, 100 });
		applyChars(layout, L"hell", { 10, 10 }, 0);
		applyChars(layout, L"o ", { 20, 20 }, 0);
		applyChars(layout, L"WORLD", { 12, 12 }, 0);
		applyChars(layout, L"!", { 30, 30 }, 0);
		auto metrics = layout.metrics();
		assertEqual("width", 100u, metrics.size.width);
		assertEqual("height", 50u, metrics.size.height);
		assertEqual("line count", size_t{2}, metrics.lines.size());
		assertEqual("1st line char count", 6u, metrics.lines.at(0).chars);
		assertEqual("1st line height", 20u, metrics.lines.at(0).height);
		assertEqual("2nd line char count", 6u, metrics.lines.at(1).chars);
		assertEqual("2nd line height", 30u, metrics.lines.at(1).height);
	});

	test("TextLayout: kerning", []()
	{
		TextLayout layout({ 100, 100 });
		applyChars(layout, L"as", { 10, 10 }, 0);
		applyChars(layout, L"D", { 10, 10 }, 2);
		applyChars(layout, L"fg", { 10, 10 }, 3);
		auto metrics = layout.metrics();
		assertEqual("width", 50u, metrics.size.width);
		assertEqual("height", 10u, metrics.size.height);
		assertEqual("line count", size_t{1}, metrics.lines.size());
		assertEqual("1st line char count", 5u, metrics.lines.at(0).chars);
		assertEqual("1st line height", 10u, metrics.lines.at(0).height);
	});

	test("TextLayout: kerning wrap word", []()
	{
		TextLayout layout({ 115, 115 });
		applyChars(layout, L"hello worl", { 10, 10 }, 0);
		applyChars(layout, L"d", { 10, 10 }, 6);
		applyChars(layout, L" ", { 10, 10 }, 0);
		applyChars(layout, L"12345", { 10, 12 }, 0);
		auto metrics = layout.metrics();
		assertEqual("width", 115u, metrics.size.width);
		assertEqual("height", 32u, metrics.size.height);
		assertEqual("line count", size_t{3}, metrics.lines.size());
		assertEqual("1st line char count", 6u, metrics.lines.at(0).chars);
		assertEqual("1st line height", 10u, metrics.lines.at(0).height);
		assertEqual("2nd line char count", 6u, metrics.lines.at(1).chars);
		assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
		assertEqual("3rd line char count", 5u, metrics.lines.at(2).chars);
		assertEqual("3rd line height", 12u, metrics.lines.at(2).height);
	});

	test("TextLayout: kerning wrap letter", []()
	{
		TextLayout layout({ 105, 105 });
		applyChars(layout, L"helloworl", { 10, 10 }, 0);
		applyChars(layout, L"d", { 10, 10 }, 6);
		applyChars(layout, L" 12345678", { 10, 10 }, 0);
		auto metrics = layout.metrics();
		assertEqual("width", 105u, metrics.size.width);
		assertEqual("height", 20u, metrics.size.height);
		assertEqual("line count", size_t{2}, metrics.lines.size());
		assertEqual("1st line char count", 9u, metrics.lines.at(0).chars);
		assertEqual("1st line height", 10u, metrics.lines.at(0).height);
		assertEqual("2nd line char count", 10u, metrics.lines.at(1).chars);
		assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
	});

	// RectangleOrganizer

	test("RectangleOrganizer: zero size tests", []()
	{
		RectangleOrganizer org{{100, 100}};
		auto c1 = org.tryClaimSlot({ 0, 10 });
		auto c2 = org.tryClaimSlot({ 10, 0 });
		auto c3 = org.tryClaimSlot({ 0, 0 });
		assertEqual("no width not found", false, c1.isFound);
		assertEqual("no height not found", false, c2.isFound);
		assertEqual("no width or height not found", false, c3.isFound);
	});

	test("RectangleOrganizer: additive positioning", []()
	{
		RectangleOrganizer org{{100, 100}};
		auto c1 = org.tryClaimSlot({ 10, 10 });
		assertEqual("1st found", true, c1.isFound);
		assertEqual("1st rect", { 0, 0, 10, 10 }, c1.slot.rect);

		auto c2 = org.tryClaimSlot({ 95, 95 });
		assertEqual("2nd too big for remaining space", false, c2.isFound);
		assertEqual("unfound has zeroed slot", { 0 }, c2.slot);

		auto c3 = org.tryClaimSlot({ 10, 10 });
		assertEqual("3rd rect", { 10, 0, 10, 10 }, c3.slot.rect);

		auto c4 = org.tryClaimSlot({ 81, 20 });
		assertEqual("4th rect", { 0, 10, 81, 20 }, c4.slot.rect);

		auto c5 = org.tryClaimSlot({ 5, 5 });
		assertEqual("5th rect", { 81, 10, 5, 5 }, c5.slot.rect);

		auto c6 = org.tryClaimSlot({ 10, 20 });
		assertEqual("6th rect", { 86, 10, 10, 20 }, c6.slot.rect);

		auto c7 = org.tryClaimSlot({ 100, 70 });
		assertEqual("7th rect", { 0, 30, 100, 70 }, c7.slot.rect);

		auto c8 = org.tryClaimSlot({ 80, 10 });
		assertEqual("8th rect", { 20, 0, 80, 10 }, c8.slot.rect);
	});

	test("RectangleOrganizer: adding and removing", []()
	{
		RectangleOrganizer org{{100, 100}};
		auto c1 = org.tryClaimSlot({ 10, 10 });
		assertEqual("1st rect", { 0, 0, 10, 10 }, c1.slot.rect);

		assertEqual("release non-existant", false, org.releaseSlot(1000));
		assertEqual("release 1st rect", true, org.releaseSlot(c1.slot.index));
		assertEqual("re-release rect", false, org.releaseSlot(c1.slot.index));

		auto c2 = org.tryClaimSlot({ 100, 100 });
		assertEqual("2nd rect", { 0, 0, 100, 100 }, c2.slot.rect);
		assertEqual("release 2nd rect", true, org.releaseSlot(c2.slot.index));

		auto c3 = org.tryClaimSlot({ 10, 10 });
		assertEqual("3rd rect", { 0, 0, 10, 10 }, c3.slot.rect);

		auto c4 = org.tryClaimSlot({ 10, 10 });
		assertEqual("4th rect", { 10, 0, 10, 10 }, c4.slot.rect);

		assertEqual("release 3rd rect", true, org.releaseSlot(c3.slot.index));

		auto c5 = org.tryClaimSlot({ 10, 10 });
		assertEqual("5th rect", { 0, 0, 10, 10 }, c5.slot.rect);

		auto c6 = org.tryClaimSlot({ 10, 10 });
		assertEqual("6th rect", { 20, 0, 10, 10 }, c6.slot.rect);
	});

	test("RectangleOrganizer: text ring", []()
	{
		RectangleOrganizer org{{100, 100}};
		auto c1 = org.tryClaimSlot({ 100, 10 });
		assertEqual("1st rect", { 0, 0, 100, 10 }, c1.slot.rect);
		auto c2 = org.tryClaimSlot({ 100, 10 });
		assertEqual("2nd rect", { 0, 10, 100, 10 }, c2.slot.rect);
		auto c3 = org.tryClaimSlot({ 100, 10 });
		assertEqual("3rd rect", { 0, 20, 100, 10 }, c3.slot.rect);

		assertEqual("release 1st", true, org.releaseSlot(c1.slot.index));

		auto c4 = org.tryClaimSlot({ 100, 10 });
		assertEqual("4th rect", { 0, 0, 100, 10 }, c4.slot.rect);

		assertEqual("release 2nd", true, org.releaseSlot(c2.slot.index));

		auto c5 = org.tryClaimSlot({ 100, 10 });
		assertEqual("5th rect", { 0, 10, 100, 10 }, c5.slot.rect);

		assertEqual("release 3rd", true, org.releaseSlot(c3.slot.index));

		auto c6 = org.tryClaimSlot({ 100, 10 });
		assertEqual("6th rect", { 0, 20, 100, 10 }, c6.slot.rect);
	});

	return summary();
}
