#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <shlwapi.h>


#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

void xorCrypt(std::string& data, const std::string& key) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= key[i % key.size()];
    }
}

std::string getExecutableDir() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    PathRemoveFileSpecW(buffer);
    std::wstring exePath(buffer);
    return std::string(exePath.begin(), exePath.end());
}


void createTemplate(const std::string& templateName, const std::string& rootDir = ".") {
    std::string key = "tmplKeyy";
    std::string templateFileName = getExecutableDir() + "/" + templateName + ".tmpl";
    std::ofstream templateFile(templateFileName, std::ios::binary);

    for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (fs::is_regular_file(entry)) {
            std::string relativePath = fs::relative(entry.path(), rootDir).string();
            std::ifstream inputFile(entry.path(), std::ios::binary);
            std::ostringstream contentStream;
            contentStream << inputFile.rdbuf();
            std::string content = contentStream.str();

            xorCrypt(content, key);

            templateFile << "FILE: " << relativePath << "\n";
            templateFile << "SIZE: " << content.size() << "\n";
            templateFile.write(content.data(), content.size());
            templateFile << "\nEND_OF_FILE\n";
        }
    }

    templateFile.close();
    std::cout << "Template created: " << templateName << std::endl;
}

void applyTemplate(const std::string& templateName) {
    std::string key = "tmplKeyy";
    std::string templateFileName = getExecutableDir() + "/" + templateName + ".tmpl";
    std::ifstream templateFile(templateFileName, std::ios::binary);

    if (!templateFile.is_open()) {
        std::cerr << "Template '" << templateName << "' not found." << std::endl;
        return;
    }

    std::string line;
    std::string currentDirectory;
    while (std::getline(templateFile, line)) {
        if (line.rfind("FILE: ", 0) == 0) {
            std::string fileName = line.substr(6);
            std::getline(templateFile, line);
            if (line.rfind("SIZE: ", 0) == 0) {
                size_t fileSize = std::stoul(line.substr(6));
                std::vector<char> content(fileSize);
                templateFile.read(content.data(), fileSize);

                std::string contentStr(content.begin(), content.end());
                xorCrypt(contentStr, key);

                size_t lastSlashPos = fileName.find_last_of("/\\");
                if (lastSlashPos != std::string::npos) {
                    currentDirectory = fileName.substr(0, lastSlashPos);
                    fs::create_directories(currentDirectory);
                }

                std::ofstream outputFile(fileName, std::ios::binary);
                outputFile.write(contentStr.data(), fileSize);
                outputFile.close();
            }
        } else if (line == "END_OF_FILE") {
            currentDirectory.clear();
        }
    }

    templateFile.close();
    std::cout << "Template applied: " << templateName << std::endl;
}

void deleteTemplate(const std::string& templateName) {
    std::string templateFileName = getExecutableDir() + "/" + templateName + ".tmpl";

    if (!fs::exists(templateFileName)) {
        std::cerr << "Template '" << templateName << "' not found." << std::endl;
        return;
    }

    fs::remove(templateFileName);

    std::cerr << "Template '" << templateName << "' deleted successfully !" << std::endl;
    
}

void templateList() {

    std::string directory = getExecutableDir() + "/";
    std::vector<std::string> templates;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".tmpl") {
            templates.push_back(entry.path().stem().string());
        }
    }

    std::cerr << "Your created Templates : " << std::endl << std::endl;

    for (const std::string& templateFile : templates) {
        std::cerr << templateFile << std::endl;
    }

}

void actualVersion() {
    std::cerr << "Version of tmpl : 1.2" << std::endl;
}



int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: tmpl <command> <template_name | command_argument>" << std::endl << std::endl;
        std::cerr << "Example: " << std::endl;
        std::cerr << "tmpl create myTemplateName  [Create a Template using the folder in which you ran this command]" << std::endl;
        std::cerr << "tmpl get myTemplateName  [Import the template you created to the selected folder.]" << std::endl;
        std::cerr << "tmpl delete myTemplateName  [Delete a Template.]" << std::endl;
        std::cerr << "tmpl show -l  [Shows the list of templates you created.]" << std::endl;
        std::cerr << "tmpl show -v  [Shows the current version of tmpl installed on your computer.]" << std::endl;


        return 1;
    }

    std::string command = argv[1];
    std::string argumentCommand = argv[2];

    if (command == "create") {
        createTemplate(argumentCommand);
    } else if (command == "get") {
        applyTemplate(argumentCommand);
    } else if(command == "delete") {
        deleteTemplate(argumentCommand);
    } else if (command == "show") {

        if (argumentCommand == "-l") {
            templateList();
        } else if (argumentCommand == "-v") {
            actualVersion();
        }

    } else {
        std::cerr << "Invalid command. Use 'create', 'get', 'delete', 'show -l' or 'show -v'." << std::endl;
        return 1;
    }

    return 0;
}