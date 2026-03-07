#ifndef SEARCHNAME_FILE_H
#define SEARCHNAME_FILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <windows.h>

#include "cliout.h"
#include "algorithm_search.h"



struct FileInfo {
    int fileId;
    std::string filename;
    std::string fullpath;
    int lineCount;
};

struct LineInfo {
    int lineId;
    int fileId;
    int lineNum;
    std::string content;
    std::string filename;
};

// глобальные
int globalFileId = 0;
int globalLineId = 0;
int globalCurrentFile = 0;
int globalAllFiles = 0;


void resetGlobal(){
    globalFileId = 0;
    globalLineId = 0;
    globalAllFiles = 0;
    globalCurrentFile = 0;
}

void ReadAndIndexFile(const std::string& filepath, const std::string& filename, int fileId) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    int lineNum = 1;

    while (std::getline(file, line)) {
        int lineIndex = lineNum;
        searchpotoks::search_potoks(line, fileId, lineIndex, filename, lineNum);

        globalLineId++;
        lineNum++;
    }
    file.close();
}

void printProgressBar(int &id, int &allFiles){
    cliout::fileProgressBar(id, allFiles);
}

void countFilesInFolders(const std::string& basePath){
    std::string searchPath = basePath + "\\*.*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        // std::cout << "Ничего " << basePath << std::endl;
        return;
    }
    
    do {
        std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        
        std::string fullPath = basePath + "\\" + name;
        
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            countFilesInFolders(fullPath);
        } else {
            globalAllFiles++;
        }
    } while (FindNextFileA(hFind, &fd) != 0);
    // std::cout << "Всего файлов: " << globalAllFiles << std::endl;
}

void SearchFilesInFolders(const std::string& basePath) {
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        return;
    }
    std::string searchPath = basePath + "\\*.*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        // std::cout << "Ничего " << basePath << std::endl;
        return;
    }
    
    // std::cout << "\n Папка: " << basePath << std::endl;
    
    do {
        printProgressBar(globalCurrentFile, globalAllFiles);
        std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        
        std::string fullPath = basePath + "\\" + name;
        
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            SearchFilesInFolders(fullPath);
        } else {
            FileInfo fi;
            fi.fileId = ++globalFileId;
            fi.filename = name;
            fi.fullpath = fullPath;
            globalCurrentFile = fi.fileId;
            
            // std::cout << "\n Найден файл ID " << fi.fileId << ": " << name 
            // << std::endl;
            
            ReadAndIndexFile(fullPath, name, fi.fileId);
            printProgressBar(fi.fileId, globalAllFiles);
        }
        if (cancelHandled) {
            break;
        }
        
    } while (FindNextFileA(hFind, &fd) != 0);
    
    FindClose(hFind);
}

void SearchFileInFolders(const std::string& searchPath){
    int kostil = 0;
    int kostil_2 = 1;
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
    int fileId = ++globalFileId;
    printProgressBar(kostil, kostil_2);

    // std::cout << "\n Найден файл ID " << fileId << ": " << fd.cFileName << std::endl;

    ReadAndIndexFile(searchPath, fd.cFileName, fileId);
    printProgressBar(kostil_2, kostil_2);
    FindClose(hFind);
}


void searchFilePrint(){
    std::cout << "  ====================================\n";
    std::cout << "\n  === Найдено файлов: " << globalFileId << " ===" << std::endl;
    std::cout << "  === Всего строк: " << globalLineId << " ===" << std::endl;
}

namespace searchNF{

    int getGlobalAllFiles(){
        return globalAllFiles;
    }


    void searchFileinFolders(std::string path)
    {
        countFilesInFolders(path);
        SearchFileInFolders(path);
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        searchFilePrint();
        resetGlobal();
    }

    void searchDirinFolders(std::string path)
    {
        countFilesInFolders(path);
        SearchFilesInFolders(path);
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        searchFilePrint();
        saveResults("both");
        saveSimpleTextData();
        resetGlobal();
    }
}


#endif