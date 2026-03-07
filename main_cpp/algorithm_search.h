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
#include <fstream>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <Python.h>

struct SecretInfo {
    std::string source;
    std::string value;
    std::string full_line;
    int line_number;
    std::string filename;
    double entropy;
};
struct LineResult {
    std::string full_line;
    int line_number;
    std::vector<SecretInfo> secrets;
};
struct FileResult {
    std::string filename;
    std::vector<LineResult> lines;
};
//global vector
std::vector<FileResult> all_results;
std::mutex global_mutex;
//Работа с вектором
int addFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(global_mutex);
    FileResult fr;
    fr.filename = filename;
    all_results.push_back(fr);
    return all_results.size() - 1;
};
//Добавление в вектор
void addLine(int fileIndex, int lineNum, const std::string& line) {
    std::lock_guard<std::mutex> lock(global_mutex);
    if (fileIndex >= 0 && fileIndex < all_results.size()) {
        LineResult lr;
        lr.full_line = line;
        lr.line_number = lineNum;
        all_results[fileIndex].lines.push_back(lr);
    }
};
//Добавление в секрет
void addSecret(int fileIndex, int lineIndex, const SecretInfo& secret) {
    std::lock_guard<std::mutex> lock(global_mutex);
    if (fileIndex >= 0 && fileIndex < all_results.size()) {
        auto& lines = all_results[fileIndex].lines;
        if (lineIndex >= 0 && lineIndex < lines.size()) {
            lines[lineIndex].secrets.push_back(secret);
        }
    }
};
//Получение индекс последней строки
int getCurrentLineIndex(int fileIndex) {
    if (fileIndex >= 0 && fileIndex < all_results.size()) {
        return all_results[fileIndex].lines.size() - 1;
    }
    return -1;
}

namespace search {

    std::vector<std::string> search_regular(const std::string& prov_regular, int fileIndex, int lineIndex, const std::string& filename, int lineNum) {
        std::vector<std::string> result;

        std::string str{ prov_regular };
        std::regex rex{ "(\\S+)" };
        std::sregex_iterator beg{ str.cbegin(), str.cend(), rex }; //итератор на начало последовательности найденных совподений
        std::sregex_iterator end{};

        for (auto i = beg; i != end; i++) { //Проходит по всем совпадениям выводим значение (*1), их индексы относительно начала исходной строки и длину совпадение строки 
            std::string word = i->str();

            bool is_secret = false;
            if (word.length() > 8) is_secret = true;

            if (word.find('=') != std::string::npos || word.find(':') != std::string::npos
                || word.find(' ') != std::string::npos || word.find(' (') != std::string::npos
                || word.find(' {') != std::string::npos || word.find(' [') != std::string::npos) is_secret = true;

            if (is_secret) {
                result.push_back(i->str() + "(" + std::to_string(i->position()) + ")" + "Вся строка: " + prov_regular);
                

                SecretInfo si;
                si.source = "search";
                si.value = i->str() + "(" + std::to_string(i->position()) + ")";
                si.full_line = prov_regular;
                si.line_number = lineNum;
                si.filename = filename;
                si.entropy = 0.0;

                addSecret(fileIndex, lineIndex, si);
            }
        }
        return result;
    }
}

namespace entropic_analysis {
    double entropicanalysis(const std::string& s, int fileIndex, int lineIndex, const std::string& filename, int lineNum) {
        if (s.empty()) return 0.0;


        std::unordered_map<char, int>map;

        for (char c : s) {
            map[c]++;
        }
        double result = 0.0;
        int len = s.length();

        for (const auto& item : map) { // Второй проход: по всем уникальным символам (ключам словаря)
            double frequence = static_cast<double>(item.second) / len;
            result -= frequence * (log(frequence) / log(2));
        }
        if (result > 3.5) {
            SecretInfo si;
            si.source = "entropy";
            si.value = "high_entropy: " + std::to_string(result);
            si.full_line = s;
            si.line_number = lineNum;
            si.filename = filename;
            si.entropy = result;

            addSecret(fileIndex, lineIndex, si);
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
                char prev = text[pos - 1];
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
        std::map<std::string, int> analyze(const std::string& text, int fileIndex, int lineIndex, const std::string& filename, int lineNum) {
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
                if (count > 0) {
                    result[word] = count;
                    SecretInfo si;
                    si.source = "keywords";
                    si.value = word + ":" + std::to_string(count);
                    si.full_line = text;
                    si.line_number = lineNum;
                    si.filename = filename;
                    si.entropy = 0.0;

                    addSecret(fileIndex, lineIndex, si);
                }
            }
            return result;
        }
    };
}


// Функция для вызова Python из C++
void sendToPython(const std::string& text, int fileIndex, int lineIndex, 
                  const std::string& filename, int lineNum) {
    
    // Инициализируем Python
    Py_Initialize();
    
    // Импортируем наш модуль
    PyObject* pName = PyUnicode_DecodeFSDefault("app");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    
    if (pModule != NULL) {
        // Получаем функцию add_data
        PyObject* pFunc = PyObject_GetAttrString(pModule, "add_data");
        
        if (pFunc && PyCallable_Check(pFunc)) {
            // Создаем кортеж с аргументами
            PyObject* pArgs = PyTuple_New(5);
            PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(text.c_str()));
            PyTuple_SetItem(pArgs, 1, PyLong_FromLong(fileIndex));
            PyTuple_SetItem(pArgs, 2, PyLong_FromLong(lineIndex));
            PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(filename.c_str()));
            PyTuple_SetItem(pArgs, 4, PyLong_FromLong(lineNum));
            
            // Вызываем функцию
            PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
            
            // Очищаем
            Py_DECREF(pArgs);
            Py_XDECREF(pResult);
        }
        
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    
    // Завершаем Python
    Py_Finalize();
}

// Используй эту функцию везде, где есть Array_data
void processData() {
    // Твои переменные
    std::string text = "пример";
    int fileIndex = 0;
    int lineIndex = 0;
    std::string filename = "file.cpp";
    int lineNum = 10;
    
    // Отправляем в Python
    sendToPython(text, fileIndex, lineIndex, filename, lineNum);
}

#include <fstream>
#include <filesystem>

void saveSimpleTextData() {
    // Открываем файл для записи (перезаписываем)
    std::ofstream file("scan_results_simple.txt");
    
    if (!file.is_open()) {
        std::cout << "НЕ МОГУ СОЗДАТЬ ФАЙЛ!" << std::endl;
        return;
    }
    
    // Пишем количество файлов
    file << "TOTAL_FILES: " << all_results.size() << "\n";
    file << "====================\n";
    
    // Проходим по всем результатам
    for (const auto& file_result : all_results) {
        file << "FILE: " << file_result.filename << "\n";
        file << "----------------\n";
        
        for (const auto& line : file_result.lines) {
            if (!line.secrets.empty()) {
                file << "LINE: " << line.line_number << "\n";
                file << "CONTENT: " << line.full_line << "\n";
                
                for (const auto& secret : line.secrets) {
                    file << "  SECRET: " << secret.source 
                         << " | " << secret.value 
                         << " | entropy: " << secret.entropy << "\n";
                }
                file << "---\n";
            }
        }
        file << "====================\n";
    }
    
    file.close();
    std::cout << "ФАЙЛ СОЗДАН: scan_results_simple.txt" << std::endl;
    std::cout << "ПУТЬ: " << std::filesystem::current_path() << std::endl;
}

namespace searchpotoks {
    void search_potoks(const std::string& text, int fileIndex, int lineIndex, const std::string& filename, int lineNum) {
        std::vector<std::string> search_result;
        double entropy_result = 0.0;
        std::map<std::string, int> keyword_result;

        std::mutex cout_mutex;
        std::mutex data_mutex;

        std::thread t1([&]() {auto res = search::search_regular(text, fileIndex, lineIndex, filename, lineNum); std::lock_guard<std::mutex> lock(data_mutex); search_result = std::move(res); });

        std::thread t2([&]() {auto res = entropic_analysis::entropicanalysis(text, fileIndex, lineIndex, filename, lineNum); std::lock_guard<std::mutex> lock(data_mutex); entropy_result = std::move(res); });

        std::thread t3([&]() {contextual_analysis::keywords analyzer; auto res = analyzer.analyze(text, fileIndex, lineIndex, filename, lineNum); std::lock_guard<std::mutex> lock(data_mutex); keyword_result = std::move(res); });

        t1.join();
        t2.join();
        t3.join();

    }
}


#endif