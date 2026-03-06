#ifndef CLIOUT_SEARCH_H
#define CLIOUT_SEARCH_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <windows.h>

// #include "searchNameFile.h"

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
        

        // std::cout << directoryPath << std::endl;
        return directoryPath;
    }

}

#endif