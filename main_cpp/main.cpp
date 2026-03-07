#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <windows.h>

#include "algorithm_search.h"
#include "cliout.h"
#include "searchNameFile.h"

void EnableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

int main(){
    EnableVTMode();
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    std::cout << "Для начала работы в КРУТОМ СКАНЕРЕ КОДА укажите\n"
                << "   vv директорию проекта или путь к файлу vv\n\n" << std::endl;

    bool firstLoopFlag = true;

    cliout::startProgram();
    // exportToPython();


    bool exit_bool = false;
    while(true){
        if(firstLoopFlag) { cliout::delLines(3); firstLoopFlag = false; } 
        else { cliout::delLines(6); }
        std::string s;
        std::cout << "\n\nНажмите Q для выхода или ENTER для продолжения>> " << std::endl;
        std::cout << ">> " << std::flush;
        std::getline(std::cin, s);
        if(s == ""){
        } else if(s == "Q" || s == "q"){
            exit_bool = true;
            break;
        } else {
            cliout::delLines(4);
            std::cout << "\nНеверный ввод" << std::endl;
            continue;
        }

        if (!exit_bool) {
            cliout::startProgram();
            // exportToPython();
        }
        
    }
}