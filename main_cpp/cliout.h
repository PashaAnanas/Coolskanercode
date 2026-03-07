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

void enterDirPrintCancel(){
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
            return -1;
        } else if(!std::filesystem::exists(dirPath)){
            // Не существует
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


std::string progressBarFill(int &barFilled){
    std::string barLine = "";
    for(double i = 0; i < static_cast<double>(barFilled) / 4; i++){
        barLine += "|";
    }
    return barLine;
}

void printTimer(){
    std::cout << "\nСкан завершен за: " << globalProgramTimer << " c\n";
}




namespace cliout{

    void delLines(int lines){
        for (int i = 0; i < lines; ++i) {
            std::cout << "\033[A\033[K";
        }
    }
    // Ввод пути
    void enterDirPath(){
    // Сброс флагов отмены
        cancelRequested = false;
        cancelHandled = false;

        std::string directoryPath = "";

        while(true){
            if (cancelRequested && !cancelHandled) {
                enterDirPrintCancel();
                cancelHandled = true;
                std::cin.clear(); // чтобы работало
                delLines(3);
                return;
            }

            
            std::cout << "Введите путь до директории проекта или путь файла. Ctrl+C для отмены.\n"
            << "Пример: >> S:\\Coolskanercode или S:\\Coolskanercode\\main.py\n"
            << std::endl;

            std::cout << "Введите директорию проекта или путь файла>> " << std::flush;
            if (!std::getline(std::cin, directoryPath)) {
                std::cout << "\n" << std::endl;
                std::cin.clear();
                delLines(2);
                continue;
            }

            int errCheck = enterPathCheck(directoryPath);
            if (errCheck == 1){
                std::cout << "\n\nАнализ в " << directoryPath << std::endl;
                auto start = std::chrono::steady_clock::now();
                searchNF::searchFileinFolders(directoryPath);
                auto end = std::chrono::steady_clock::now();
                // Для файла таймер не вызывается (как в оригинале)
                break;
            } else if (errCheck == 2){
                std::cout << "\n\nАнализ в " << directoryPath << std::endl;
                auto start = std::chrono::steady_clock::now();
                searchNF::searchDirinFolders(directoryPath);
                auto end = std::chrono::steady_clock::now();
                programTimer(start, end);
                break;
            } else if (errCheck == -1){
                delLines(6);
                std::cerr << "\nПуть не может быть пустым.\n" << std::flush;
            } else if (errCheck == 0){
                delLines(6);
                std::cerr << "\nПуть не существует.\n" << std::flush;
            }

        }
    }

    // Начало проги
    int startProgram(){
        enterDirPath();
        if (!cancelHandled) {
            printTimer();
            return 1;
        }
        return 0;
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