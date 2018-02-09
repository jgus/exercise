#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace boggle
{
    // A simple representation of an NxM Boggle board
    template<typename TChar>
    class board
    {
    public:
        using char_t = TChar;

        board(int width, int height) : _width{width}, _height{height}
        {
            _values.resize(_width * _height);
        }

        board() : board{ 0, 0 } {}

        auto width() const { return _width; }
        auto height() const { return _height; }

        char_t& operator()(int x, int y) { return _values[index(x,y)]; }
        char_t const& operator()(int x, int y) const { return _values[index(x,y)]; }

        friend std::basic_ostream<char_t>& operator<<(std::basic_ostream<char_t>& s, board const& b)
        {
            for (auto y = 0; y < b._height; ++y)
            {
                s << std::basic_string<char_t>{ b._values.begin() + (y * b._width), b._values.begin() + ((y+1) * b._width) } << std::endl;
            }
            return s;
        }

        friend std::basic_istream<char_t>& operator>>(std::basic_istream<char_t>& s, board& b)
        {
            b._width = 0;
            b._height = 0;
            b._values.clear();
            while (true)
            {
                std::basic_string<char_t> line;
                std::getline(s, line);
                if (line.length() == 0) { break; }
                if (b._width == 0)
                {
                    b._width = line.length();
                }
                else if (b._width != line.length())
                {
                    throw std::runtime_error("nonrectangular input");
                }
                b._values.insert(b._values.end(), line.begin(), line.end());
                ++b._height;
            }
            return s;
        }

    private:
        int _width;
        int _height;
        std::vector<char_t> _values;

        auto index(int x, int y) const
        {
            if (x < 0 || _width <= x) { throw std::out_of_range("x out of range"); }
            if (y < 0 || _height <= y) { throw std::out_of_range("y out of range"); }
            return y * _width + x;
        }
    };

    // A trie - one way of representing a set of sequences (ie, a dictionary) that happens to be super efficient for solving a Boggle board
    template<typename TChar>
    class trie
    {
    public:
        using char_t = TChar;

        template<typename TIter>
        bool contains_sequence(TIter const& begin, TIter const& end) const
        {
            if (begin == end) { return _contains_this; }
            auto i = _children.find(*begin);
            if (i == _children.end()) { return false; }
            assert(i->second);
            return i->second->contains_sequence(std::next(begin), end);
        }

        template<typename TSeq>
        bool contains_sequence(TSeq const& seq) const { return contains_sequence(std::begin(seq), std::end(seq)); }

        bool contains_this() const { return _contains_this; }

        template<typename TIter>
        void insert_sequence(TIter const& begin, TIter const& end)
        {
            if (begin == end)
            {
                _contains_this = true;
                return;
            }
            auto i = _children.find(*begin);
            if (i == _children.end())
            {
                std::tie(i, std::ignore) = _children.emplace(*begin, std::make_unique<trie<char_t>>());
            }
            assert(i->second);
            i->second->insert_sequence(std::next(begin), end);
        }

        template<typename TSeq>
        void insert_sequence(TSeq const& seq) { insert_sequence(std::begin(seq), std::end(seq)); }

        trie<char_t> const* subtrie(char_t index) const
        {
            auto i = _children.find(index);
            return (i == _children.end()) ? nullptr : i->second.get();
        }

    private:
        bool _contains_this;
        std::map<char_t, std::unique_ptr<trie<char_t>>> _children;
    };

    // Do a depth-first search among the space of all legal paths through the given Boggle board, with some very agressive trimming of the search tree
    template<typename TChar>
    std::set<std::basic_string<TChar>> solve(board<TChar> const& board, trie<TChar> const& dictionary)
    {

        using char_t = TChar;
        using address = std::pair<int, int>;

        // For efficiency, we're going to have a bit of state which is global across the search: the current path, and the word represented by that path. (It's much more efficient to modify these in-place as we go, rather than allocate and modify copies.
        std::basic_string<char_t> word;
        std::vector<address> path;

        // We're also going to globally track the set of valid words found so far
        std::set<std::basic_string<char_t>> found;

        // The actual recursive function. It's a static method in an inner struct, since C++ doesn't let us nest functions directly.
        struct solver
        {
            static void solve(boggle::board<char_t> const& board, trie<char_t> const& subdictionary, std::basic_string<char_t>& word, std::vector<address>& path, address next, std::set<std::basic_string<char_t>>& found)
            {
                // Is the proposed next space off the board? If not, illegal path
                if (next.first < 0 || board.width() <= next.first || next.second < 0 || board.height() <= next.second) { return; }

                // Have we already visited the next space to build our word so far? If so, illegal path
                if (std::find(path.begin(), path.end(), next) != path.end()) { return; }

                // If we add the next letter, are there any words in the dictionary that start with our sequence so far? If not, no need to keep searching this path
                char_t next_element = board(next.first, next.second);
                auto child_dictionary = subdictionary.subtrie(next_element);
                if (!child_dictionary) { return; }

                // Add the next letter to our word so far
                word.push_back(next_element);
                path.push_back(next);

                // If the path so far is a valid word, add it to the found list
                if (child_dictionary->contains_this())
                {
                    found.insert(word);
                }

                // Try recursively adding to the path in the eight legal directions
                solve(board, *child_dictionary, word, path, address{ next.first-1, next.second-1 }, found);
                solve(board, *child_dictionary, word, path, address{ next.first-1, next.second }, found);
                solve(board, *child_dictionary, word, path, address{ next.first-1, next.second+1 }, found);
                solve(board, *child_dictionary, word, path, address{ next.first, next.second-1 }, found);
                solve(board, *child_dictionary, word, path, address{ next.first, next.second+1 }, found);
                solve(board, *child_dictionary, word, path, address{ next.first+1, next.second-1 }, found);
                solve(board, *child_dictionary, word, path, address{ next.first+1, next.second }, found);
                solve(board, *child_dictionary, word, path, address{ next.first+1, next.second+1 }, found);

                // Put our working state back how we found it (would be nice to guarantee this with an RAII construct)
                path.pop_back();
                word.pop_back();
            }
        };

        // Try starting paths from all locations on the board
        for (int x = 0; x < board.width(); ++x)
        {
            for (int y = 0; y < board.height(); ++y)
            {
                solver::solve(board, dictionary, word, path, address{ x, y }, found);
            }
        }
        return found;
    }
}
