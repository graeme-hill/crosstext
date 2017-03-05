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
        layout.nextChar(L'w', { 8, 8 }, 0);
        layout.nextChar(L'a', { 8, 8 }, 0);
        layout.nextChar(L's', { 8, 8 }, 0);
        layout.nextChar(L'd', { 8, 8 }, 0);
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
        layout.nextChar(L'w', { 8, 8 }, 0);
        layout.nextChar(L'a', { 8, 8 }, 0);
        layout.nextChar(L's', { 8, 8 }, 0);
        layout.nextChar(L' ', { 8, 8 }, 0);
        layout.nextChar(L'd', { 8, 8 }, 0);
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
        layout.nextChar(L'w', { 8, 8 }, 0);
        layout.nextChar(L'a', { 8, 8 }, 0);
        layout.nextChar(L's', { 8, 8 }, 0);
        layout.nextChar(L' ', { 8, 8 }, 0);
        layout.nextChar(L'q', { 8, 8 }, 0);
        layout.nextChar(L'w', { 8, 8 }, 0);
        layout.nextChar(L'e', { 8, 8 }, 0);
        auto metrics = layout.metrics();

        assertEqual("width", 45u, metrics.size.width);
        assertEqual("height", 16u, metrics.size.height);
        assertEqual("line count", size_t{2}, metrics.lines.size());
        assertEqual("1st line char count", 4u, metrics.lines.at(0).chars);
        assertEqual("2nd line char count", 3u, metrics.lines.at(1).chars);
        assertEqual("1st line height", 8u, metrics.lines.at(0).height);
        assertEqual("2nd line height", 8u, metrics.lines.at(1).height);
    });

    return summary();
}
