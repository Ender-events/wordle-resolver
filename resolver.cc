#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "debug.hh"

template <std::size_t N>
class Wordle {
public:
    Wordle(const std::string& allowPath, const std::string& allPath);
    std::pair<std::string, bool> nextWord();
    void validWord(const std::string& word, const std::string& input);
    void trimWord();

private:
    std::vector<std::string> loadList(const std::string& path);
    std::array<uint, 26> histogramLetter();
    void dumpHistogramLetter(const std::array<uint, 26>& histo);
    std::string discoverLetter();

    class Letters {
    public:
        Letters(bool defaultVal = true)
            : letters_ {}
        {
            letters_.fill(defaultVal);
        };
        bool contains(char c)
        {
            assert(c >= 'a' && c <= 'z');
            return letters_[c - 'a'];
        };
        void unset(char c)
        {
            assert(c >= 'a' && c <= 'z');
            letters_[c - 'a'] = false;
        };
        void set(char c)
        {
            assert(c >= 'a' && c <= 'z');
            letters_[c - 'a'] = true;
        };
        void is(char c)
        {
            assert(c >= 'a' && c <= 'z');
            letters_.fill(false);
            letters_[c - 'a'] = true;
        }

    private:
        std::array<bool, 26> letters_;
    };
    std::array<Letters, N> word_;
    std::array<Letters, N> atLeast_;
    std::vector<std::string> allowWord_;
    std::vector<std::string> allWord_;
};

template <std::size_t N>
Wordle<N>::Wordle(const std::string& allowPath, const std::string& allPath)
    : word_ {}
    , atLeast_ {}
    , allowWord_ { loadList(allowPath) }
    , allWord_ { loadList(allPath) }
{
    atLeast_.fill(false);
    debug << "allowWord size: " << allowWord_.size() << '\n';
}

template <std::size_t N>
std::vector<std::string> Wordle<N>::loadList(const std::string& path)
{
    std::vector<std::string> vect {};
    std::string line;
    std::ifstream infile(path);
    while (std::getline(infile, line)) {
        vect.push_back(std::move(line));
    }
    return vect;
}

template <std::size_t N>
std::array<uint, 26> Wordle<N>::histogramLetter()
{
    std::array<uint, 26> res {};
    res.fill(0);
    for (const auto& word : allowWord_) {
        Letters onlyOnce {};
        for (std::size_t i = 0; i < N; ++i) {
            char c = word[i];
            if (atLeast_[i].contains(c))
                continue;
            onlyOnce.unset(c);
        }
        for (char c = 'a'; c <= 'z'; c++) {
            if (not onlyOnce.contains(c))
                res[c - 'a']++;
        }
    }
    return res;
}

template <std::size_t N>
void Wordle<N>::dumpHistogramLetter(const std::array<uint, 26>& histo)
{
    std::multimap<uint, char> sorted {};
    for (char c = 'a'; c <= 'z'; c++) {
        sorted.emplace(histo[c - 'a'], c);
    }
    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
        std::cout << it->second << ": " << it->first << "; ";
    }
    std::cout << '\n';
}

template <std::size_t N>
std::string Wordle<N>::discoverLetter()
{
    auto histo = histogramLetter();
    dumpHistogramLetter(histo);
    std::string bestWord;
    uint64_t bestScore = 0;
    for (const auto& word : allWord_) {
        uint64_t score = 0;
        Letters onlyOnce { false };
        for (char c : word) {
            onlyOnce.set(c);
        }
        for (char c = 'a'; c <= 'z'; c++) {
            if (onlyOnce.contains(c))
                score += histo[c - 'a'];
        }
        if (score > bestScore) {
            bestWord = word;
            bestScore = score;
        }
    }
    debug << "best score: " << bestScore << '\n';
    return bestWord;
}

template <std::size_t N>
std::pair<std::string, bool> Wordle<N>::nextWord()
{
    if (allowWord_.size() == 1)
        return { allowWord_[0], true };
    if (allowWord_.size() < 10) {
        for (const auto& word : allowWord_) {
            debug << "maybe: " << word << '\n';
        }
    }
    return { discoverLetter(), false };
}

template <std::size_t N>
void Wordle<N>::validWord(const std::string& word, const std::string& input)
{
    for (std::size_t i = 0; i < N; ++i) {
        char c = word[i];
        if (input[i] == 'b') {
            for (std::size_t j = 0; j < N; ++j) {
                word_[j].unset(c);
            }
        } else if (input[i] == 'y') {
            word_[i].unset(c);
            atLeast_[i].set(c);
        } else if (input[i] == 'g') {
            atLeast_[i].is(c);
            word_[i].is(c);
        } else {
            throw "invalid input";
        }
    }
}

template <std::size_t N>
void Wordle<N>::trimWord()
{
    allowWord_.erase(std::remove_if(
                         allowWord_.begin(),
                         allowWord_.end(),
                         [this](const auto& word) {
                             std::size_t i = 0;
                             for (char c : word) {
                                 if (not word_[i].contains(c)) {
                                     return true;
                                 }
                                 i++;
                             };
                             return false;
                         }),
        allowWord_.end());
    debug << "allowWord size: " << allowWord_.size() << '\n';
}

int main()
{
    Wordle<5> wordle { "allow_word.txt", "all_word.txt" };
    while (true) {
        auto [word, find] = wordle.nextWord();
        std::string input;
        std::cout << "< " << word << '\n';
        if (find)
            return 0;
        std::cout << "> ";
        std::cin >> input;
        if (input.empty())
            return 0;
        wordle.validWord(word, input);
        wordle.trimWord();
    }
}
