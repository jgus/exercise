
#include <gtest/gtest.h>
#include <random>

#include "boggle/boggle.hpp"

TEST(Boggle, SmallKnownBoard)
{
    boggle::board<char> board;
    auto board_source = std::istringstream{ "uthe\nkefn\nwxrp\nolbz\n" };
    board_source >> board;

    std::set<std::string> included_words
    {
        "blow",
        "blower",
        "brew",
        "fern",
        "few",
        "hen",
        "her",
        "her",
        "lower",
        "then",
    };
    std::set<std::string> excluded_words
    {
        "eel",
        "fox",
        "hunter",
        "knee",
        "loner",
        "pot",
        "reflex",
    };
    boggle::trie<char> dictionary;
    for (auto const& s : included_words) { dictionary.insert_sequence(s); }
    for (auto const& s : excluded_words) { dictionary.insert_sequence(s); }

    auto found_words = boggle::solve(board, dictionary);

    ASSERT_EQ(found_words.size(), included_words.size());
    for (auto const& s : found_words)
    {
        ASSERT_TRUE(included_words.find(s) != included_words.end());
    }
}

TEST(Boggle, LargeRandomPerformance)
{
    std::default_random_engine random{ std::random_device{}() };
    std::uniform_int_distribution<char> letters{ 'a', 'z' };

    boggle::board<char> board{ 50, 50 };
    for (auto x = 0; x < board.width(); ++x)
    {
        for (auto y = 0; y < board.height(); ++y)
        {
            board(x, y) = letters(random);
        }
    }

    auto get_random_string = [&letters, &random](int size)
    {
        std::string value;
        for (int i = 0; i < size; ++i)
        {
            value.push_back(letters(random));
        }
        return value;
    };

    boggle::trie<char> dictionary;
    for (int i = 0; i < 500; ++i)
    {
        dictionary.insert_sequence(get_random_string(3));
        dictionary.insert_sequence(get_random_string(4));
        dictionary.insert_sequence(get_random_string(5));
        dictionary.insert_sequence(get_random_string(6));
        dictionary.insert_sequence(get_random_string(7));
    }

    auto begin_time = std::chrono::high_resolution_clock::now();

    boggle::solve(board, dictionary);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - begin_time).count();
    ASSERT_LE(duration_ms, 1000);
}
