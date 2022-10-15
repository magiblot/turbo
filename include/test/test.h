#ifndef TURBO_TEST_H
#define TURBO_TEST_H

#include <gtest/gtest.h>

#include <turbo/scintilla/internals.h>
#include <turbo/scintilla.h>

namespace turbo
{

template <class T>
struct TestCase
{
    T input;
    T result;
};

template <class T1, class T2>
inline void expectMatchingResult(const T1 &actual, const TestCase<T2> &testCase)
{
    EXPECT_EQ(actual, testCase.result) << "With test input:\n" << testCase.input;
}

struct TextState
{
    enum : char {
        chCaret = '|',
        chAnchor = '^',
    };

    std::string text;
    Sci::Position caret {-1};
    Sci::Position anchor {-1};

    static TextState decode(std::string_view);
    static std::string encode(TextState);
};

TScintilla &createScintilla(TextState state);
TextState getTextState(TScintilla &scintilla);

} // namespace turbo

#endif // TURBO_TEST_H
