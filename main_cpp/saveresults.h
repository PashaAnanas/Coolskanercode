#ifndef SAVERESULTS_H
#define SAVERESULTS_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include "algorithm_search.h"

void saveToTxt(const std::string& filename = "scan_results.txt") {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << " Не могу создать файл " << filename << std::endl;
        return;
    }
    
    file << "\n" << std::string(60, '=') << "\n";
    file << " РЕЗУЛЬТАТЫ ПОИСКА СЕКРЕТОВ\n";
    file << std::string(60, '=') << "\n";
    
    int totalSecrets = 0;
    int totalFiles = 0;
    
    for (const auto& file_result : all_results) {
        bool fileHasSecrets = false;
        totalFiles++;
        
        for (const auto& line : file_result.lines) {
            if (!line.secrets.empty()) {
                if (!fileHasSecrets) {
                    file << "\n ФАЙЛ: " << file_result.filename << "\n";
                    file << std::string(40, '-') << "\n";
                    fileHasSecrets = true;
                }
                
                file << "   Строка " << line.line_number << ": " << line.full_line << "\n";
                
                for (const auto& secret : line.secrets) {
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
    }
    
    file << std::string(60, '=') << "\n";
    file << " Просканировано файлов: " << totalFiles << "\n";
    file << " Всего найдено секретов: " << totalSecrets << "\n";
    file << std::string(60, '=') << "\n";
    
    file.close();
    std::cout << " Результаты сохранены в " << filename << std::endl;
}

void saveToHtml(const std::string& filename = "scan_results.html") {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << " Не могу создать файл " << filename << std::endl;
        return;
    }
    
    int totalSecrets = 0;
    int totalFiles = all_results.size();
    for (const auto& file_result : all_results) {
        for (const auto& line : file_result.lines) {
            totalSecrets += line.secrets.size();
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
    file << "            <div class='stat-item'><div class='stat-value'>" << totalFiles << "</div><div class='stat-label'> Файлов</div></div>\n";
    file << "            <div class='stat-item'><div class='stat-value'>" << totalSecrets << "</div><div class='stat-label'> Секретов</div></div>\n";
    file << "        </div>\n";
    
    for (const auto& file_result : all_results) {
        bool fileHasSecrets = false;
        for (const auto& line : file_result.lines) {
            if (!line.secrets.empty()) {
                if (!fileHasSecrets) {
                    file << "        <div class='file'>\n";
                    file << "            <div class='filename'> ФАЙЛ: " << file_result.filename << "</div>\n";
                    fileHasSecrets = true;
                }
                
                file << "            <div class='line'>\n";
                file << "                <span class='line-number'> Строка " << line.line_number << ":</span> " << line.full_line << "\n";
                
                for (const auto& secret : line.secrets) {
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
        }
        if (fileHasSecrets) {
            file << "        </div>\n";
        }
    }
    
    file << "        <div class='footer'>\n";
    file << "            " << std::string(60, '=') << "\n";
    file << "            <p>Сгенерировано КРУТЫМ СКАНЕРОМ КОДА</p>\n";
    file << "        </div>\n";
    file << "    </div>\n";
    file << "</body>\n";
    file << "</html>\n";
    
    file.close();
    std::cout << " Результаты сохранены в " << filename << std::endl;
}

void saveResults(const std::string& format = "txt") {
    if (format == "txt" || format == "all" || format == "both") {
        saveToTxt();
    }
    if (format == "html" || format == "all" || format == "both") {
        saveToHtml();
    }
}

#endif