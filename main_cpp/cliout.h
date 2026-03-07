#ifndef CLIOUT_SEARCH_H
#define CLIOUT_SEARCH_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <mutex>
#include <windows.h>

// #include "searchNameFile.h"

// ограничение частоты вывода
std::chrono::steady_clock::time_point lastPrintTime;
std::mutex printMutex;
const auto printInterval = std::chrono::milliseconds(50);


// Предварительное объявление для предотвращения багов
namespace searchNF {
    void searchFileinFolders();
}

// Проверки ввода
bool enterPathCheck(std::string& directoryPath){
        // Пустой
        if(directoryPath.empty()){
            std::cerr << "Путь не может быть пустым.\n" << std::endl;
            return false;
        }

        std::filesystem::path dirPath(directoryPath);
        // Не существует
        if(!std::filesystem::exists(dirPath)){
            std::cerr << "Путь не существует.\n" << std::endl;
            return false;
        }

        // Не директория
        if (!std::filesystem::is_directory(dirPath)) {
        std::cerr << "Указанный путь не является директорией.\n" << std::endl;
        return false;
    }

    return true;
}

namespace cliout{

    // Начало проги
    void startProgram(){
        std::cout << "vv Для начала работы в КРУТОМ СКАНЕРЕ КОДА укажите директорию проекта vv\n" << std::endl;
        searchNF::searchFileinFolders();
    }

    std::string enterDirPath(){
        std::string directoryPath = "";
        std::cout << "Введите путь до директории проекта.\n"
        << "Пример: >> S:\\vscode_projects\\krutoy_scaner\\Coolskanercode"
        << std::endl;

        while(true){
            std::cout << "Введите директорию проекта >> " << std::flush;
            std::getline(std::cin, directoryPath);
            if (enterPathCheck(directoryPath)){
                std::cout << "Успех\n" << std::endl;
                break;
            }
        }
        
        return directoryPath;
    }


    std::string progressBarFill(int &barFilled){
        std::string barLine = "";
        for(double i = 0; i < static_cast<double>(barFilled) / 4; i++){
            barLine += "|";
        }
        return barLine;
    }
    
    void fileProgressBar(int &cF, int &aF){
        auto now = std::chrono::steady_clock::now();
    
        if (now - lastPrintTime >= printInterval || cF == aF) {
            std::lock_guard<std::mutex> lock(printMutex);
            
            float currentFile = static_cast<float>(cF);
            float allFiles = static_cast<float>(aF);
            int barFilled = static_cast<float>(currentFile / allFiles * 100);
            
            static int lastLineCount = 0;
            
            for (int i = 0; i < lastLineCount; ++i) {
                std::cout << "\033[A\033[K";
            }

            std::cout << "\n\nОбратотано " << cF << " из " << aF << " файлов" << std::endl;
            std::cout << " _______________________" << std::endl;
            std::cout << std::left << std::setw(26) << progressBarFill(barFilled) 
            << barFilled << "%" << std::endl;
            std::cout << " ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n" << std::endl;
            lastLineCount = 7;
            
            if (barFilled == 100) {
                std::cout << "Обработка завершена.\n" << std::endl;
            }
            
            lastPrintTime = now;
        }
    }
    // main tests
    // int per1 = 0; int per2 = 2000;
    // for(int i = 0; i <= per2; i++){
    //     cliout::fileProgressBar(i, per2);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // }
    

}

#endif