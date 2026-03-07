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
#include <fstream>

// Структура для хранения информации о найденном секрете
struct SecretInfo {
    std::string source;      // откуда обнаружено: "search", "entropy", "keywords"
    std::string value;       // значение/описание
    std::string full_line;   // полная строка, в которой найден секрет
    int line_number;         // номер строки в файле
    std::string filename;    // имя файла
    double entropy;          // значение энтропии (для entropy)
};

// Глобальный вектор для хранения всех секретов (плоская структура)
static std::vector<SecretInfo> all_secrets;
static std::mutex global_mutex;

// Функция добавления нового секрета в общий вектор (принимает только SecretInfo)
void addSecret(const SecretInfo& secret) {
    std::lock_guard<std::mutex> lock(global_mutex);
    all_secrets.push_back(secret);
}

// ----------------------------------------------------------------------
// Функции для совместимости со старым кодом (могут быть удалены, 
// но оставлены чтобы не ломать существующие вызовы)
// ----------------------------------------------------------------------
int addFile(const std::string& /*filename*/) {
    return 0;   // фиктивный индекс файла
}

void addLine(int /*fileIndex*/, int /*lineNum*/, const std::string& /*line*/) {
    // ничего не делаем, информация о строке хранится в SecretInfo
}

int getCurrentLineIndex(int /*fileIndex*/) {
    return 0;   // фиктивный индекс строки
}

// ----------------------------------------------------------------------
// Алгоритмы поиска (логика не изменена, только исправлены вызовы addSecret)
// ----------------------------------------------------------------------
namespace search {

    std::vector<std::string> search_regular(const std::string& prov_regular,
                                             int /*fileIndex*/, int /*lineIndex*/,
                                             const std::string& filename, int lineNum) {
        std::vector<std::string> result;
        std::string str{ prov_regular };
        std::regex rex{ "(\\S+)" };
        std::sregex_iterator beg{ str.cbegin(), str.cend(), rex };
        std::sregex_iterator end{};

        for (auto i = beg; i != end; ++i) {
            std::string word = i->str();

            bool is_secret = false;
            if (word.length() > 8) is_secret = true;

            if (word.find('=') != std::string::npos ||
                word.find(':') != std::string::npos ||
                word.find(' ') != std::string::npos ||
                word.find(' (') != std::string::npos ||
                word.find(' {') != std::string::npos ||
                word.find(' [') != std::string::npos) {
                is_secret = true;
            }

            if (is_secret) {
                result.push_back(i->str() + "(" + std::to_string(i->position()) + ")" +
                                 "Вся строка: " + prov_regular);

                SecretInfo si;
                si.source = "search";
                si.value = i->str() + "(" + std::to_string(i->position()) + ")";
                si.full_line = prov_regular;
                si.line_number = lineNum;
                si.filename = filename;
                si.entropy = 0.0;

                // ИСПРАВЛЕНО: передаём только si, без fileIndex и lineIndex
                addSecret(si);
            }
        }
        return result;
    }
}

namespace entropic_analysis {

    double entropicanalysis(const std::string& s,
                            int /*fileIndex*/, int /*lineIndex*/,
                            const std::string& filename, int lineNum) {
        if (s.empty()) return 0.0;

        std::unordered_map<char, int> map;
        for (char c : s) {
            map[c]++;
        }

        double result = 0.0;
        int len = s.length();
        for (const auto& item : map) {
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

            // ИСПРАВЛЕНО: передаём только si
            addSecret(si);
        }
        return result;
    }
}

namespace contextual_analysis {

    class keywords {
    private:
        std::vector<std::string> targetWords = {
            "password", "token", "api", "key"
        };

        std::string toLower(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            return result;
        }

        bool hasContexChar(const std::string& text, size_t pos, size_t len) {
            if (pos > 0) {
                char prev = text[pos - 1];
                if (prev == ':' || prev == '=' || prev == ' ' ||
                    prev == ' (' || prev == ' {' || prev == ' [')
                    return true;
            }

            size_t end = pos + len;
            if (end < text.length()) {
                char next = text[end];
                if (next == ':' || next == '=' || next == ' ' ||
                    next == ' (' || next == ' {' || next == ' [')
                    return true;
            }
            return false;
        }

    public:
        std::map<std::string, int> analyze(const std::string& text,
                                           int /*fileIndex*/, int /*lineIndex*/,
                                           const std::string& filename, int lineNum) {
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

                    // ИСПРАВЛЕНО: передаём только si
                    addSecret(si);
                }
            }
            return result;
        }
    };
}

// ----------------------------------------------------------------------
// Вспомогательная функция для экранирования JSON
// ----------------------------------------------------------------------
std::string escapeJson(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else result += c;
    }
    return result;
}

// ----------------------------------------------------------------------
// Экспорт результатов в JSON для Python-отчёта
// ----------------------------------------------------------------------
void exportToPython() {
    std::map<std::string, std::map<int, std::vector<SecretInfo>>> grouped;
    {
        std::lock_guard<std::mutex> lock(global_mutex);
        for (const auto& sec : all_secrets) {
            grouped[sec.filename][sec.line_number].push_back(sec);
        }
    }

    std::ofstream file("scan_results.json");
    file << "{\n";
    file << "  \"scan_time\": \"" << __TIMESTAMP__ << "\",\n";
    file << "  \"total_files\": " << grouped.size() << ",\n";
    file << "  \"files\": [\n";

    bool firstFile = true;
    for (const auto& fileEntry : grouped) {
        if (!firstFile) file << ",\n";
        firstFile = false;

        file << "    {\n";
        file << "      \"filename\": \"" << escapeJson(fileEntry.first) << "\",\n";
        file << "      \"lines\": [\n";

        bool firstLine = true;
        for (const auto& lineEntry : fileEntry.second) {
            if (!firstLine) file << ",\n";
            firstLine = false;

            file << "        {\n";
            file << "          \"line_number\": " << lineEntry.first << ",\n";
            file << "          \"content\": \"" << escapeJson(lineEntry.second[0].full_line) << "\",\n";
            file << "          \"secrets\": [\n";

            for (size_t s = 0; s < lineEntry.second.size(); ++s) {
                const auto& secret = lineEntry.second[s];
                file << "            {\n";
                file << "              \"type\": \"" << secret.source << "\",\n";
                file << "              \"value\": \"" << escapeJson(secret.value) << "\",\n";
                file << "              \"entropy\": " << secret.entropy << "\n";
                file << "            }";
                if (s < lineEntry.second.size() - 1) file << ",";
                file << "\n";
            }
            file << "          ]\n";
            file << "        }";
        }
        file << "\n      ]\n";
        file << "    }";
    }
    file << "\n  ]\n";
    file << "}\n";
    file.close();

    // Вызов внешнего Python-скрипта для формирования отчёта
    system("python report.py");
}

// ----------------------------------------------------------------------
// Потоковая обёртка для параллельного запуска трёх видов анализа
// ----------------------------------------------------------------------
namespace searchpotoks {

    void search_potoks(const std::string& text, int fileIndex, int lineIndex,
                       const std::string& filename, int lineNum) {
        std::thread t1([&]() {
            try {
                search::search_regular(text, fileIndex, lineIndex, filename, lineNum);
            } catch (const std::exception& e) {
                std::cerr << "Exception in search_regular: " << e.what() << std::endl;
            }
        });

        std::thread t2([&]() {
            try {
                entropic_analysis::entropicanalysis(text, fileIndex, lineIndex, filename, lineNum);
            } catch (const std::exception& e) {
                std::cerr << "Exception in entropicanalysis: " << e.what() << std::endl;
            }
        });

        std::thread t3([&]() {
            try {
                contextual_analysis::keywords().analyze(text, fileIndex, lineIndex, filename, lineNum);
            } catch (const std::exception& e) {
                std::cerr << "Exception in keywords analyze: " << e.what() << std::endl;
            }
        });

        t1.join();
        t2.join();
        t3.join();
    }
}

#endif // ALGORITHM_SEARCH_H