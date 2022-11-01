#include <test/test.h>
#include <turbo/turbo.h>

namespace turbo
{

static std::string toggleComment(const Language &language, std::string_view input)
{
    auto &&inputState = TextState::decode(input);
    auto &scintilla = createScintilla(inputState);
    toggleComment(scintilla, &language);
    auto &&outputState = getTextState(scintilla);
    destroyScintilla(scintilla);
    return TextState::encode(std::move(outputState));
}

TEST(EditStates, ShouldRemoveHtmlBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "<!DOCTYPE html>\n"
            "<!--<ht|ml>-->\n"
            "<head>\n",

            "<!DOCTYPE html>\n"
            "<ht|ml>\n"
            "<head>\n",
        },
        {   "<!DO^<!--CTYPE html>\n"
            "<html>\n"
            "<he-->|ad>\n",

            "<!DO^CTYPE html>\n"
            "<html>\n"
            "<he|ad>\n",
        },
        {   "<!DO|<!--CTYPE html>\n"
            "<html>\n"
            "<he-->^ad>\n",

            "<!DO|CTYPE html>\n"
            "<html>\n"
            "<he^ad>\n",
        },
        {   "<!DOCTYPE html>\n"
            "^<!--<html>-->\n"
            "|<head>\n",

            "<!DOCTYPE html>\n"
            "^<html>\n"
            "|<head>\n",
        },
        {   "^<!--\n"
            "\n"
            "-->\n"
            "|\n",

            "^\n"
            "\n"
            "\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::HTML, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldRemoveCppBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "^/*         */\n"
            "/*         */\n"
            "/*         */\n"
            "|/**/\n",

            "^         */\n"
            "/*         */\n"
            "/*         \n"
            "|/**/\n",
        },
        {   "void foo(int |/*unused*/^) { }\n",

            "void foo(int |unused^) { }\n",
        },
        {   "void foo(int |/* unused */^) { }\n",

            "void foo(int | unused ^) { }\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldRemoveBashLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "|\n"
            "# echo a\n"
            "   #sleep 3\n"
            "\n"
            " #  cat^\n",

            "|\n"
            "echo a\n"
            "   sleep 3\n"
            "\n"
            "  cat^\n",
        },
        {   "\n"
            "# ech|o a\n"
            "   #sleep 3 # && echo b\n"
            "\n"
            " ^#  cat\n",

            "\n"
            "ech|o a\n"
            "   sleep 3 # && echo b\n"
            "\n"
            " ^ cat\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::Bash, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldRemoveBatchLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "^rem echo a\n"
            "rem  echo b\n"
            "|\n",

            "^echo a\n"
            " echo b\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::Batch, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldRemoveCppLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "// int i| = 0;\n",

            "int i| = 0;\n",
        },
        {   "|// int i = 0;\n"
            "// int j = 0;\n"
            "// int z = 0;^\n",

            "|int i = 0;\n"
            "int j = 0;\n"
            "int z = 0;^\n",
        },
        {   "// i^nt i = 0;\n"
            "// in|t j = 0;\n",

            "i^nt i = 0;\n"
            "in|t j = 0;\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldInsertHtmlBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "|\n",

            "<!--|-->\n",
        },
        {   "|<!DOCTYPE html>\n",

            "<!--|<!DOCTYPE html>-->\n",
        },
        {   "<|!DOCTYPE html>\n",

            "<!--<|!DOCTYPE html>-->\n",
        },
        {   "^\n"
            "\n"
            "\n"
            "|\n",

            "^<!--\n"
            "\n"
            "-->\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::HTML, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldInsertCppBlockComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "void foo(int |unused^) { }\n",

            "void foo(int |/*unused*/^) { }\n",
        },
        {   "void foo(int | unused ^) { }\n",

            "void foo(int |/* unused */^) { }\n",
        },
        {   "^int i = 0;\n"
            "int j = 0|;\n",

            "^/*int i = 0;\n"
            "int j = 0*/|;\n",
        },
        {   "i^nt i = 0;\n"
            "int j = 0;\n"
            "|\n",

            "i^/*nt i = 0;\n"
            "int j = 0;*/\n"
            "|\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldInsertCppLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "|\n",

            "// |\n",
        },
        {   " |\n",

            "//  |\n",
        },
        {   "|int i = 0;\n",

            "// |int i = 0;\n",
        },
        {   "int i| = 0;\n",

            "// int i| = 0;\n",
        },
        {   "|int i = 0;\n"
            "int j = 0;\n"
            "int z = 0;^\n",

            "|// int i = 0;\n"
            "// int j = 0;\n"
            "// int z = 0;^\n",
        },
        {   "^ int i = 0;\n"
            "\n"
            "        int j = 0;\n"
            "    int z = 0;|\n",

            "^ // int i = 0;\n"
            " // \n"
            " //        int j = 0;\n"
            " //    int z = 0;|\n",
        },
        {   "|            int i = 0;\n"
            "\t\tint j = 0;^\n",

            "|  //           int i = 0;\n"
            "\t\t// int j = 0;^\n",
        },
        {   "int i = 0;|\n"
            "^\n",

            "// int i = 0;|\n"
            "^\n",
        },
        {   "int i = 0;^\n"
            "|\n",

            "// int i = 0;^\n"
            "|\n",
        },
        {   "|\n"
            "\n"
            "\n"
            "^\n",

            "|// \n"
            "// \n"
            "// \n"
            "^\n",
        }
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::CPP, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

TEST(EditStates, ShouldInsertBatchLineComments)
{
    static constexpr TestCase<std::string_view> testCases[] =
    {
        {   "ech|o a\n",

            "rem ech|o a\n",
        },
    };

    for (auto &testCase : testCases)
    {
        auto &&actual = toggleComment(Language::Batch, testCase.input);
        expectMatchingResult(actual, testCase);
    }
}

} // namespace turbo
