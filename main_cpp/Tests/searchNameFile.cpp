#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "searchNameFile.cpp"
#include "cliout.cpp"
void SetupRussianConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
}


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

int globalFileId = 0;
int globalLineId = 0;


void ReadAndIndexFile(const std::string& filepath, const std::string& filename, int fileId) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "  Хуй тебе " << filename << std::endl;
        return;
    }
    
    std::string line;
    int lineNum = 1;
    
    std::cout << "  Читаем файл ID " << fileId << ": " << filename << std::endl;
    
    while (std::getline(file, line)) {
        LineInfo li;
        li.lineId = ++globalLineId;
        li.fileId = fileId;
        li.lineNum = lineNum++;
        li.content = line;
        li.filename = filename;
    }
    file.close();
}

void SearchFilesInFolders(const std::string& basePath) {
    std::string searchPath = basePath + "\\*.*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cout << "Нихуя " << basePath << std::endl;
        return;
    }
    
    std::cout << "\n📁 Папка: " << basePath << std::endl;
    
    do {
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
            
            std::cout << "\n🔍 Найден файл ID " << fi.fileId << ": " << name << std::endl;
            
            ReadAndIndexFile(fullPath, name, fi.fileId);
        }
        
    } while (FindNextFileA(hFind, &fd) != 0);
    
    FindClose(hFind);
}

namespace searchNF{
    void searchFileinFolders()
    {
        SetupRussianConsole();
        std::string path = cliout::enterDirPath();
        SearchFilesInFolders(path);
    }
}

// int main() {
    
//     SetupRussianConsole();
//     std::string path = "C:\\Users\\lizun\\Desktop\\RPM";
    
//     SearchFilesInFolders(path);
    
//     std::cout << "\n====================================\n";
//     std::cout << "===ИТОГО:\n===";
//     std::cout << "\n ===Найдено файлов: ===" << globalFileId << std::endl;
//     std::cout << "\n ===Всего строк: ===" << globalLineId << std::endl;
    
//     return 0;
// }
