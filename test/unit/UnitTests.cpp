#include <string>
#include <iostream>
#include <functional>
#include "CrossText.hpp"

using namespace ct;

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

    test("no chars", []()
    {
        TextLayout layout({ 100, 100 });
        auto metrics = layout.metrics();

        assertEqual("width", 0u, metrics.size.width);
        assertEqual("height", 0u, metrics.size.height);
        assertEqual("line count", 0u, metrics.size.height);
    });

    test("one char", []()
    {
        TextLayout layout({ 100, 100 });
        layout.nextChar(L'x', { 20, 30 }, 0);
        auto metrics = layout.metrics();

        assertEqual("width", 20u, metrics.size.width);
        assertEqual("height", 30u, metrics.size.height);
        assertEqual("line count", size_t{1}, metrics.lines.size());
        assertEqual("line char count", 1u, metrics.lines.at(0).chars);
        assertEqual("line height", 30u, metrics.lines.at(0).height);
    });

    test("one long word", []()
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

    test("space at end of line", []()
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

    test("space before end of line", []()
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

    test("wrapping sentence with weird whitespace", []()
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

    test("one very long word", []()
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
        assertEqual("3st line char count", 5u, metrics.lines.at(2).chars);
        assertEqual("4nd line char count", 10u, metrics.lines.at(3).chars);
        assertEqual("5st line char count", 10u, metrics.lines.at(4).chars);
        assertEqual("6nd line char count", 10u, metrics.lines.at(5).chars);
        assertEqual("7st line char count", 10u, metrics.lines.at(6).chars);
        assertEqual("1st line height", 10u, metrics.lines.at(0).height);
        assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
        assertEqual("3st line height", 10u, metrics.lines.at(2).height);
        assertEqual("4nd line height", 10u, metrics.lines.at(3).height);
        assertEqual("5st line height", 10u, metrics.lines.at(4).height);
        assertEqual("6nd line height", 10u, metrics.lines.at(5).height);
        assertEqual("7st line height", 10u, metrics.lines.at(6).height);
    });

    test("one very long chunk of whitespace", []()
    {
        TextLayout layout({ 105, 105 });
        applyChars(
            layout,
            L"hello world this is a                                  word :-)",
            //^     ^          ^    ^         ^         ^         ^       ^
            { 10, 10 },
            0);
        auto metrics = layout.metrics();

        assertEqual("width", 105u, metrics.size.width);
        assertEqual("height", 80u, metrics.size.height);
        assertEqual("line count", size_t{8}, metrics.lines.size());
        assertEqual("1st line char count", 6u, metrics.lines.at(0).chars);
        assertEqual("2nd line char count", 11u, metrics.lines.at(1).chars);
        assertEqual("3rd line char count", 5u, metrics.lines.at(2).chars);
        assertEqual("4th line char count", 10u, metrics.lines.at(3).chars);
        assertEqual("5th line char count", 10u, metrics.lines.at(4).chars);
        assertEqual("6th line char count", 10u, metrics.lines.at(5).chars);
        assertEqual("7th line char count", 8u, metrics.lines.at(6).chars);
        assertEqual("8th line char count", 3u, metrics.lines.at(7).chars);
        assertEqual("1st line height", 10u, metrics.lines.at(0).height);
        assertEqual("2nd line height", 10u, metrics.lines.at(1).height);
        assertEqual("3rd line height", 10u, metrics.lines.at(2).height);
        assertEqual("4th line height", 10u, metrics.lines.at(3).height);
        assertEqual("5th line height", 10u, metrics.lines.at(4).height);
        assertEqual("6th line height", 10u, metrics.lines.at(5).height);
        assertEqual("7th line height", 10u, metrics.lines.at(6).height);
        assertEqual("8th line height", 10u, metrics.lines.at(7).height);
    });

    test("wrap word with leading whitespace", []()
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

    return summary();
}
