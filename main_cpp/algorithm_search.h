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
#include <thread>
#include <mutex>
namespace search {

    std::vector<std::string> search_regular(const std::string& prov_regular) {
        std::vector<std::string> result; 

        std::string str{ prov_regular }; 
        std::regex rex{ "(\\S+)" };
        std::sregex_iterator beg{ str.cbegin(), str.cend(), rex }; //итератор на начало последовательности найденных совподений
        std::sregex_iterator end{};

        for (auto i = beg; i != end; i++) { //Проходит по всем совпадениям выводим значение (*1), их индексы относительно начала исходной строки и длину совпадение строки 
            result.push_back(i->str() + "(" + std::to_string(i->position()) + ")" + "[" + std::to_string(i->length()) + "]");
        }
        return result;
    }
}

namespace entropic_analysis {
    double entropicanalysis(const std::string& s) {
        if (s.empty()) return 0.0;

        // Словарь для хранения пар: символ -> сколько раз встретился
        std::unordered_map<char, int>map;

        for (char c : s) { // Первый проход по строке: подсчитываем частоту каждого символа для каждого символа c в строке s увеличиваем счётчик для этого символа на 1 eсли символа не было, он создаётся автоматически
            map[c]++;
        }
        double result = 0.0; 
        int len = s.length();

        for (const auto& item : map) { // Второй проход: по всем уникальным символам (ключам словаря)
            double frequence = static_cast<double>(item.second) / len; 
            result -= frequence * (log(frequence) / log(2));  


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
                std::string result = str; 
                std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {return std::tolower(c); }); //преобразование к каждому символу, сохраняем результят, лямда для каждого символа, приобразование в нижний регистор
                return result; 
            }
            bool hasContexChar(const std::string& text, size_t pos, size_t len) {
            //Проверяем символ перед словом
            if (pos > 0) {
                char prev = text[pos-1];
                if (prev == ':' || prev == '=' || prev == ' ' || prev == ' (' || prev == ' {' || prev == ' [')
                    return true;
            }
            
            //Проверяем символ после слова
            size_t end = pos + len;
            if (end < text.length()) {
                char next = text[end];
                if (next == ':' || next == '=' || next == ' ' || next == ' (' || next == ' {' || next == ' [')
                    return true;
            }
            
            return false;  
        }
        public:
            std::map<std::string, int> analyze(const std::string& text) {
                std::map<std::string, int> result; 

                std::string lowerText = toLower(text); 
                
                for (const auto& word : targetWords) {
                    std::string lowerWords = toLower(word);
                    size_t pos = 0; 
                    int count = 0; 

                    while ((pos = lowerText.find(lowerWords, pos)) != std::string::npos) {
                       if (hasContexChar(lowerText, pos, lowerWords.length())) { 
                        count++;
                    }
                        pos += lowerWords.length();
                    }
                    if (count > 0) { //слово найдено добавляем
                        result[word] = count; 
                    }
                }
            return result;
        }
    };
}
namespace searchpotoks{
    void search_potoks(const std::string& text){
        std::vector<std::string> search_result;
        double entropy_result = 0.0;
        std::map<std::string, int> keyword_result;

        std::mutex cout_mutex;
        std::mutex data_mutex;

        std::thread t1([&]() {auto res = search::search_regular(text); std::lock_guard<std::mutex> lock(data_mutex); search_result = std::move(res); });\

        std::thread t2([&]() {auto res = entropic_analysis::entropicanalysis(text);std::lock_guard<std::mutex> lock(data_mutex);entropy_result = std::move(res);}); 

        std::thread t3([&]() {contextual_analysis::keywords analyzer; auto res = analyzer.analyze(text); std::lock_guard<std::mutex> lock(data_mutex); keyword_result = std::move(res);});

        t1.join();
        t2.join();
        t3.join();
        
    }
}
#endif