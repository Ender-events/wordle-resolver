#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "debug.hh"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define uint unsigned int
#endif

template <std::size_t N>
class Wordle {
public:
    Wordle(const std::string& allowPath, const std::string& allPath);
    std::pair<std::string, bool> nextWord();
    std::vector<char> validWord(const std::string& word, const std::string& input);
    void trimWord(const std::vector<char>& yellow);

private:
    std::vector<std::string> loadList(const std::string& path);
    std::array<uint, 26> histogramLetter();
    void dumpHistogramLetter(const std::array<uint, 26>& histo);
    std::string discoverLetter();
    bool transformYellowToGreen();

    class Letters {
    public:
        static const char NIL = '\0';
        Letters(bool defaultVal = true)
            : letters_ {}
            , c_ { NIL }
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
            c_ = c;
            letters_.fill(false);
            letters_[c - 'a'] = true;
        }
        char getChar()
        {
            return c_;
        }
        bool isGreen()
        {
            return c_ != NIL;
        }
        std::vector<char> toList()
        {
            std::vector<char> res;
            for (char c = 'a'; c <= 'z'; ++c) {
                if (contains(c))
                    res.push_back(c);
            }
            return res;
        }

    private:
        std::array<bool, 26> letters_;
        char c_;
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
std::vector<char> Wordle<N>::validWord(const std::string& word, const std::string& input)
{
    std::vector<char> yellow {};
    for (std::size_t i = 0; i < N; ++i) {
        char c = word[i];
        if (input[i] == 'b') {
            for (std::size_t j = 0; j < N; ++j) {
                word_[j].unset(c);
            }
        } else if (input[i] == 'y') {
            word_[i].unset(c);
            atLeast_[i].set(c);
            yellow.push_back(c);
        } else if (input[i] == 'g') {
            atLeast_[i].is(c);
            word_[i].is(c);
        } else {
            throw "invalid input";
        }
    }

    while (transformYellowToGreen())
        continue;

    for (std::size_t i = 0; i < N; ++i) {
        char c = word_[i].getChar();
        if (c == Letters::NIL)
            c = 'X';
        debug << c;
    }
    debug << '\n';
    return yellow;
}

template <std::size_t N>
bool Wordle<N>::transformYellowToGreen()
{
    bool haveDoTransform = false;
    for (std::size_t i = 0; i < N; ++i) {
        if (word_[i].isGreen())
            continue;
        for (char c : atLeast_[i].toList()) {
            // c is a yellow
            std::unordered_set<char> potentialPos {};
            for (std::size_t j = 0; j < N; ++j) {
                if (word_[j].getChar() == c) {
                    // but c is also green, no potential pos foundable
                    potentialPos.clear();
                    break;
                }
                if (not atLeast_[j].contains(c) && not word_[j].isGreen()) {
                    potentialPos.emplace(j);
                }
            }
            if (potentialPos.size() == 1) {
                std::size_t pos = *potentialPos.begin();
                atLeast_[pos].is(c);
                word_[pos].is(c);
                haveDoTransform = true;
            }
        }
    }
    return haveDoTransform;
}

template <std::size_t N>
void Wordle<N>::trimWord(const std::vector<char>& yellow)
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

    for (char yChar : yellow) {
        allowWord_.erase(std::remove_if(
                             allowWord_.begin(),
                             allowWord_.end(),
                             [yChar](const auto& word) {
                                 for (char c : word) {
                                     if (yChar == c) {
                                         return false;
                                     }
                                 };
                                 return true;
                             }),
            allowWord_.end());
    }
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
#ifdef __EMSCRIPTEN__
        input = emscripten_run_script_string("prompt('Enter 5 letters result (b = black, g = green, y = yellow', 'bgbyb');");
        std::cout << "> " << input << '\n';
#else
        std::cout << "> ";
        std::cin >> input;
#endif
        if (input.empty())
            return 0;
        auto yellow = wordle.validWord(word, input);
        wordle.trimWord(yellow);
    }
}
