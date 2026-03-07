#ifndef CLIOUT_SEARCH_H
#define CLIOUT_SEARCH_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <mutex>
#include <atomic>
#include <windows.h>

// #include "searchNameFile.h"
extern std::mutex printMutex;
static std::atomic<bool> cancelRequested{false};
static std::atomic<bool> cancelHandled{false};

double globalProgramTimer = 0;

void barPrintCancel(int delLines) {
    std::lock_guard<std::mutex> lock(printMutex);
    for (int i = 0; i < delLines; ++i) {
        std::cout << "\033[A\033[K";
    }
    std::cout << "Операция прервана пользователем.\n" << std::endl;
}

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        cancelRequested = true;
        // Возвращаем TRUE, чтобы сигнал не передавался дальше (процесс не завершался принудительно)
        return TRUE;
    }
    return FALSE;
}

// ограничение частоты вывода
std::chrono::steady_clock::time_point lastPrintTime;
std::mutex printMutex;
const auto printInterval = std::chrono::milliseconds(50);


// Предварительное объявление для предотвращения багов
namespace searchNF {
    void searchFileinFolders(std::string path);
    void searchDirinFolders(std::string path);
}


// Проверки ввода
int enterPathCheck(std::string& directoryPath){
    std::filesystem::path dirPath(directoryPath);
        // Пустой
        if(directoryPath.empty()){
            std::cerr << "Путь не может быть пустым.\n" << std::endl;
            return -1;
        } else if(!std::filesystem::exists(dirPath)){
            // Не существует
            std::cerr << "Путь не существует.\n" << std::endl;
            return 0;
        } else if (!std::filesystem::is_directory(dirPath)) {
            // Не директория
            return 1;
        } else { 
            // Директория
            return 2;
        }

    return 0;
}

void programTimer(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end){
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double milliseconds = static_cast<double>(elapsed_ms.count());
    double seconds = milliseconds / 1000;
    globalProgramTimer = seconds;
}


std::string enterDirPath(){
        std::string directoryPath = "";
        std::cout << "Введите путь до директории проекта или путь файла.\n"
        << "Пример: >> S:\\Coolskanercode или S:\\Coolskanercode\\main.py\n"
        << std::endl;

        while(true){
            std::cout << "Введите директорию проекта или путь файла>> " << std::flush;
            std::getline(std::cin, directoryPath);
            int errCheck = enterPathCheck(directoryPath);
            if (errCheck == 1){
                // std::cout << "Успех 1\n" << std::endl;
                std::cout << "\n\nАнализ в " << directoryPath << std::endl;
                auto start = std::chrono::steady_clock::now();
                searchNF::searchFileinFolders(directoryPath);
                auto end = std::chrono::steady_clock::now();
                break;
            } else if (errCheck == 2){
                // std::cout << "Успех 2\n" << std::endl;
                std::cout << "\n\nАнализ в " << directoryPath << std::endl;
                auto start = std::chrono::steady_clock::now();
                searchNF::searchDirinFolders(directoryPath);
                auto end = std::chrono::steady_clock::now();
                programTimer(start, end);
                break;
            }
        }
        
        return directoryPath;
    }

namespace cliout{

    // Начало проги
    void startProgram(){
        std::cout << "Для начала работы в КРУТОМ СКАНЕРЕ КОДА укажите\n"
        << "   vv директорию проекта или путь к файлу vv\n" << std::endl;

        enterDirPath();

        std::cout << "\nСкан завершен за: " << globalProgramTimer << " c\n";
    }


    std::string progressBarFill(int &barFilled){
        std::string barLine = "";
        for(double i = 0; i < static_cast<double>(barFilled) / 4; i++){
            barLine += "|";
        }
        return barLine;
    }
    
    void fileProgressBar(int &cF, int &aF){
        if (cancelHandled) return;

        if (cancelRequested && !cancelHandled) {
            barPrintCancel(7-5);
            cancelHandled = true;
            return;
        }



        auto now = std::chrono::steady_clock::now();
    
        if (now - lastPrintTime >= printInterval || cF == aF) {
            std::lock_guard<std::mutex> lock(printMutex);
            static int lastLineCount = 0;
            
            if (cancelRequested && !cancelHandled) {
                barPrintCancel(lastLineCount-5);
                cancelHandled = true;
                return;
            }

            float currentFile = static_cast<float>(cF);
            float allFiles = static_cast<float>(aF);
            int barFilled = static_cast<float>(currentFile / allFiles * 100);
            
            
            for (int i = 0; i < lastLineCount; ++i) {
                std::cout << "\033[A\033[K";
            }

            std::cout << "Обратотано " << cF << " из " << aF << " файлов" << std::endl;
            std::cout << " _______________________" << std::endl;
            std::cout << std::left << std::setw(26) << progressBarFill(barFilled) 
            << barFilled << "%" << std::endl;
            std::cout << " ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n" << std::endl;
            std::cout << "Нажмите Ctrl+C для прерывания операции\n" << std::endl;
            lastLineCount = 7;
            
            if (barFilled == 100) {
                for (int i = 0; i < lastLineCount-5; ++i) {
                    std::cout << "\033[A\033[K";
                }
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