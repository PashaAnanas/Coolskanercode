#ifndef ALGORITHM_SEARCH_H
#define ALGORITHM_SEARCH_H

#include <iostream>
#include <string>
#include <regex>
#include <unordered_map>
#include <iomanip>
#include <cmath>
#include <set>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>


namespace search {

    std::vector<std::string> search_regular(const std::string& prov_regular) {
        std::vector<std::string> result; //для результатов

        std::string str{ prov_regular }; 
        std::regex rex{ "(\\S+)" };
        std::sregex_iterator beg{ str.cbegin(), str.cend(), rex }; //итератор на начало последовательности найденных совподений
        std::sregex_iterator end{};

        for (auto i = beg; i != end; i++) { //Проходит по всем совпадениям выводим значение (*1), их индексы относительно начала исходной строки и длину совпадение строки 
            /*std::cout << i->str() << "(" << i->position() << ")" << "[" << i->length() << "]\n"; */
            result.push_back(i->str() + "(" + std::to_string(i->position()) + ")" + "[" + std::to_string(i->length()) + "]");
        }
        return result;
    }
}

namespace entropic_analysis {
    double entropicanalysis(const std::string& s) {
        if (s.empty()) return 0.0; // Защита от пустой строки

        // Словарь для хранения пар: символ -> сколько раз встретился
        std::unordered_map<char, int>map;

        for (char c : s) { // Первый проход по строке: подсчитываем частоту каждого символа для каждого символа c в строке s увеличиваем счётчик для этого символа на 1 eсли символа не было, он создаётся автоматически
            map[c]++;
        }
        double result = 0.0; // Переменная для накопления результата
        int len = s.length();

        for (const auto& item : map) { // Второй проход: по всем уникальным символам (ключам словаря)
            double frequence = static_cast<double>(item.second) / len; // Вычисляем частоту символа: сколько раз встретился / общая длина
            result -= frequence * (log(frequence) / log(2));  // log(frequence) / log(2) — это смена основания логарифма на двоичное


        }
        return result;
    }
}
namespace contextual_analysis {
        class keywords {
        private:
            std::vector<std::string> targetWords = {
            "password", "token", "api", "key"//ключевые слова
            };
            std::string toLower(const std::string& str) {
                std::string result = str; //копируем исходнуую строку
                std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {return std::tolower(c); }); //преобразование к каждому символу, сохраняем результят, лямда для каждого символа, приобразование в нижний регистор
                return result; //возвращяем строку в нижнем регистре
            }
        public:
            std::map<std::string, int> analyze(const std::string& text) {
                std::map<std::string, int> result; //сохраняем найденые слова и их количество

                std::string lowerText = toLower(text); //нижний регистор

                for (const auto& word : targetWords) {
                    std::string lowerWords = toLower(word);
                    size_t pos = 0; //позиция в тексте
                    int count = 0; //счетчик найденых слов

                    while ((pos = lowerText.find(lowerWords, pos)) != std::string::npos) {
                        count++; //счетчик слов
                        pos += lowerWords.length();
                    }
                    if (count > 0) { //слово найдено добавляем
                        result[word] = count; //сохраняем мар ключ оргинальныое слово, значчение количество 
                    }
                }
            return result;
        }
    };
}

#endif