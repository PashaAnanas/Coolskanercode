#ifndef SAVERESULTS_H
#define SAVERESULTS_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <mutex>
#include "algorithm_search.h"


// Вспомогательная функция для группировки секретов по файлам и строкам
static std::map<std::string, std::map<int, std::vector<SecretInfo>>> groupSecrets() {
    std::lock_guard<std::mutex> lock(global_mutex);
    std::map<std::string, std::map<int, std::vector<SecretInfo>>> grouped;
    for (const auto& sec : all_secrets) {
        grouped[sec.filename][sec.line_number].push_back(sec);
    }
    return grouped;
}

// Сохранение в текстовый файл
void saveToTxt(const std::string& filename = "scan_results.txt") {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << " Не могу создать файл " << filename << std::endl;
        return;
    }

    auto grouped = groupSecrets();

    file << "\n" << std::string(60, '=') << "\n";
    file << " РЕЗУЛЬТАТЫ ПОИСКА СЕКРЕТОВ\n";
    file << std::string(60, '=') << "\n";

    int totalSecrets = 0;
    int filesWithSecrets = 0;

    for (const auto& fileEntry : grouped) {
        filesWithSecrets++;
        bool firstLineInFile = true;

        for (const auto& lineEntry : fileEntry.second) {
            if (firstLineInFile) {
                file << "\n ФАЙЛ: " << fileEntry.first << "\n";
                file << std::string(40, '-') << "\n";
                firstLineInFile = false;
            }

            file << "   Строка " << lineEntry.first << ": " << lineEntry.second[0].full_line << "\n";

            for (const auto& secret : lineEntry.second) {
                file << "     [" << secret.source << "] " << secret.value;
                if (secret.entropy > 0) {
                    file << " (энтропия: " << std::fixed << std::setprecision(2) << secret.entropy << ")";
                }
                file << "\n";
                totalSecrets++;
            }
            file << "\n";
        }
    }

    file << std::string(60, '=') << "\n";
    file << " Файлов с секретами: " << filesWithSecrets << "\n";
    file << " Всего найдено секретов: " << totalSecrets << "\n";
    file << std::string(60, '=') << "\n";

    file.close();
    std::cout << " Результаты сохранены в " << filename << std::endl;
}


// Сохранение в HTML
void saveToHtml(const std::string& filename = "scan_results.html") {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << " Не могу создать файл " << filename << std::endl;
        return;
    }

    auto grouped = groupSecrets();

    int totalSecrets = 0;
    int filesWithSecrets = 0;
    for (const auto& fileEntry : grouped) {
        filesWithSecrets++;
        for (const auto& lineEntry : fileEntry.second) {
            totalSecrets += lineEntry.second.size();
        }
    }

    file << "<!DOCTYPE html>\n";
    file << "<html lang='ru'>\n";
    file << "<head>\n";
    file << "    <meta charset='UTF-8'>\n";
    file << "    <title>Результаты поиска секретов</title>\n";
    file << "    <style>\n";
    file << "        body { font-family: 'Courier New', monospace; margin: 20px; background: #1e1e1e; color: #d4d4d4; }\n";
    file << "        .container { max-width: 1200px; margin: 0 auto; background: #252526; padding: 20px; border-radius: 10px; }\n";
    file << "        h1 { color: #569cd6; border-bottom: 2px solid #569cd6; padding-bottom: 10px; }\n";
    file << "        .stats { background: #2d2d30; padding: 15px; border-radius: 5px; margin: 20px 0; border-left: 4px solid #569cd6; display: grid; grid-template-columns: repeat(2, 1fr); }\n";
    file << "        .stat-item { text-align: center; }\n";
    file << "        .stat-value { font-size: 2em; font-weight: bold; color: #4ec9b0; }\n";
    file << "        .stat-label { color: #858585; }\n";
    file << "        .file { background: #2d2d30; margin: 20px 0; padding: 15px; border-left: 4px solid #4ec9b0; border-radius: 5px; }\n";
    file << "        .filename { font-size: 1.3em; font-weight: bold; color: #4ec9b0; margin-bottom: 10px; }\n";
    file << "        .line { background: #1e1e1e; margin: 10px 0; padding: 10px; border: 1px solid #3e3e42; border-radius: 5px; }\n";
    file << "        .line-number { color: #858585; margin-right: 10px; }\n";
    file << "        .secret { margin: 5px 0 5px 20px; padding: 5px; border-left: 3px solid; background: #1e1e1e; }\n";
    file << "        .search { border-left-color: #6a9955; }\n";
    file << "        .entropy { border-left-color: #dcdcaa; }\n";
    file << "        .keywords { border-left-color: #ce9178; }\n";
    file << "        .footer { margin-top: 30px; text-align: center; color: #858585; }\n";
    file << "    </style>\n";
    file << "</head>\n";
    file << "<body>\n";
    file << "    <div class='container'>\n";
    file << "        <h1> РЕЗУЛЬТАТЫ ПОИСКА СЕКРЕТОВ</h1>\n";
    file << "        <div class='stats'>\n";
    file << "            <div class='stat-item'><div class='stat-value'>" << filesWithSecrets << "</div><div class='stat-label'> Файлов с секретами</div></div>\n";
    file << "            <div class='stat-item'><div class='stat-value'>" << totalSecrets << "</div><div class='stat-label'> Секретов</div></div>\n";
    file << "        </div>\n";

    for (const auto& fileEntry : grouped) {
        file << "        <div class='file'>\n";
        file << "            <div class='filename'> ФАЙЛ: " << fileEntry.first << "</div>\n";

        for (const auto& lineEntry : fileEntry.second) {
            file << "            <div class='line'>\n";
            file << "                <span class='line-number'> Строка " << lineEntry.first << ":</span> " << lineEntry.second[0].full_line << "\n";

            for (const auto& secret : lineEntry.second) {
                std::string secretClass;
                if (secret.source == "search") secretClass = "search";
                else if (secret.source == "entropy") secretClass = "entropy";
                else secretClass = "keywords";

                file << "                <div class='secret " << secretClass << "'>\n";
                file << "                     [" << secret.source << "] " << secret.value;
                if (secret.entropy > 0) {
                    file << " (энтропия: " << std::fixed << std::setprecision(2) << secret.entropy << ")";
                }
                file << "\n";
                file << "                </div>\n";
            }
            file << "            </div>\n";
        }
        file << "        </div>\n";
    }

    file << "        <div class='footer'>\n";
    file << "            " << std::string(60, '=') << "\n";
    file << "            <p>Сгенерировано КРУТЫМ СКАНЕРОМ КОДА</p>\n";
    file << "        </div>\n";
    file << "    </div>\n";
    file << "</body>\n";
    file << "</html>\n";

    file.close();
    std::cout << " Результаты сохранены в " << filename << "\n\n" << std::endl;
}


// функция сохранения в txt
void saveResults(const std::string& format = "txt") {
    if (format == "txt" || format == "all" || format == "both") {
        saveToTxt();
    }
    if (format == "html" || format == "all" || format == "both") {
        saveToHtml();
    }
}

#endif