#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <shlwapi.h>
#include <winhttp.h>


#pragma comment(lib, "winhttp.lib")

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace fs = std::filesystem;
using namespace std;

const string VERSION = "1.5";

void xorCrypt(string& data, const string& key) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= key[i % key.size()];
    }
}

string getExecutableDir() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    PathRemoveFileSpecW(buffer);
    wstring exePath(buffer);
    return string(exePath.begin(), exePath.end());
}

void createTemplate(const string& templateName, const string& rootDir = ".") {
    string key = "tmplKeyy";
    string templateFileName = getExecutableDir() + "/" + templateName + ".tmpl";
    ofstream templateFile(templateFileName, ios::binary);

    for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (fs::is_regular_file(entry)) {
            string relativePath = fs::relative(entry.path(), rootDir).string();
            ifstream inputFile(entry.path(), ios::binary);
            ostringstream contentStream;
            contentStream << inputFile.rdbuf();
            string content = contentStream.str();

            xorCrypt(content, key);

            templateFile << "FILE: " << relativePath << "\n";
            templateFile << "SIZE: " << content.size() << "\n";
            templateFile.write(content.data(), content.size());
            templateFile << "\nEND_OF_FILE\n";
        }
    }

    templateFile.close();
    cout << "Template created: " << templateName << endl;
}

void applyTemplate(const string& templateName) {
    string key = "tmplKeyy";
    string templateFileName = getExecutableDir() + "/" + templateName + ".tmpl";
    ifstream templateFile(templateFileName, ios::binary);

    if (!templateFile.is_open()) {
        cerr << "Template '" << templateName << "' not found." << endl;
        return;
    }

    string line;
    string currentDirectory;
    while (getline(templateFile, line)) {
        if (line.rfind("FILE: ", 0) == 0) {
            string fileName = line.substr(6);
            getline(templateFile, line);
            if (line.rfind("SIZE: ", 0) == 0) {
                size_t fileSize = stoul(line.substr(6));
                vector<char> content(fileSize);
                templateFile.read(content.data(), fileSize);

                string contentStr(content.begin(), content.end());
                xorCrypt(contentStr, key);

                size_t lastSlashPos = fileName.find_last_of("/\\");
                if (lastSlashPos != string::npos) {
                    currentDirectory = fileName.substr(0, lastSlashPos);
                    fs::create_directories(currentDirectory);
                }

                ofstream outputFile(fileName, ios::binary);
                outputFile.write(contentStr.data(), fileSize);
                outputFile.close();
            }
        } else if (line == "END_OF_FILE") {
            currentDirectory.clear();
        }
    }

    templateFile.close();
    cout << "Template applied: " << templateName << endl;
}

void deleteTemplate(const string& templateName) {
    string templateFileName = getExecutableDir() + "/" + templateName + ".tmpl";

    if (!fs::exists(templateFileName)) {
        cerr << "Template '" << templateName << "' not found." << endl;
        return;
    }

    fs::remove(templateFileName);

    cerr << "Template '" << templateName << "' deleted successfully!" << endl;
}

void templateList() {
    string directory = getExecutableDir() + "/";
    vector<string> templates;
    for (const auto& entry : filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".tmpl") {
            templates.push_back(entry.path().stem().string());
        }
    }


    if (templates.empty()) {
        cerr << "No templates found." << endl;
    } else {

        cerr << "Your Templates: " << endl << endl;

        for (const string& templateFile : templates) {
            cerr << templateFile << endl;
        }
    }
}

void actualVersion()
{
    cerr << "Version of tmpl: " << VERSION << endl;
}

string getLatestReleaseVersion() {
    wstring url = L"/MrTigerST/tmpl/main/version";
    HINTERNET hSession = WinHttpOpen(L"tmpl/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        cerr << "Failed to open HTTP session." << endl;
        return "";
    }

    HINTERNET hConnect = WinHttpConnect(hSession, L"raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        cerr << "Failed to connect to server." << endl;
        WinHttpCloseHandle(hSession);
        return "";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", url.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        cerr << "Failed to open HTTP request." << endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        cerr << "Failed to send HTTP request." << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        cerr << "Failed to receive HTTP response." << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    string response;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            cerr << "Failed to query data available." << endl;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        if (dwSize == 0) {
            break;
        }

        vector<char> buffer(dwSize);
        if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
            cerr << "Failed to read HTTP response." << endl;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        response.append(buffer.begin(), buffer.begin() + dwDownloaded);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);


    return response;
}

std::string __space__remover__(const std::string &str) {
    std::string result;
    std::remove_copy_if(str.begin(), str.end(), std::back_inserter(result), [](char ch) {
        return std::isspace(ch);
    });
    return result;
}

void checkForUpdates() {
    string latestVersion = getLatestReleaseVersion();

    if (!latestVersion.empty()) {


        cerr << "Latest version: " <<  __space__remover__(latestVersion) << endl;
        
        cerr << "Your version: " << VERSION << endl;
        if ( __space__remover__(latestVersion) != VERSION) {
            cerr << "A new version of tmpl is available! Please download the Update from GitHub (https://github.com/MrTigerST/tmpl/releases)." << endl;
        } else {
            cerr << "You are using the latest version." << endl;
        }

    } else {
        cerr << "Failed to check for updates. Check your internet connection and try again." << endl;
    }

}

void exportTemplate(const string& templateName, const string& templateOutputDirectory) {

    string templateFileName = getExecutableDir() + "/" + templateName + ".tmpl";

    if (!fs::exists(templateFileName)) {
        cerr << "Template '" << templateName << "not found." << endl;
        return;
    }

    if (!fs::exists(templateOutputDirectory)) {
        cout << "Output directory '" << templateOutputDirectory << "' does not exist. Creating it . . ." << endl;
        if (!fs::create_directories(templateOutputDirectory)) {
            cerr << "Failed to create output directory '" << templateOutputDirectory << "'." << endl;
            return;
        }
    }

    string outputTemplateFileName = templateOutputDirectory + "/" + templateName + ".tmpl";

    try {
        fs::copy_file(templateFileName, outputTemplateFileName, fs::copy_options::overwrite_existing);
        cerr << "Template '" << templateName << "' exported to '" << outputTemplateFileName << "' successfully !" << endl;
    } catch (const fs::filesystem_error& e) {
        cerr << "Failed to export template '" << templateName << "' to '" << outputTemplateFileName << "': " << e.what() << endl;
    }
}




void importTemplate(const string& templateInputFile, const string& templateName = "") {
    string key = "tmplKeyy";
    ifstream inputFile(templateInputFile, ios::binary);

    if (!inputFile.is_open()) {
        cerr << "Template input file '" << templateInputFile << "' not found." << endl;
        return;
    }

    string executableDir = getExecutableDir();
    string newTemplateName = (templateName.empty()) ? fs::path(templateInputFile).filename().replace_extension("").string() : templateName;
    string templateFileName = (newTemplateName) + ".tmpl";
    string outputFilePath = executableDir + "/" + templateFileName;

    try {
        fs::copy_file(templateInputFile, outputFilePath, fs::copy_options::overwrite_existing);
        cerr << "Template imported from '" << templateInputFile << "' as '" << newTemplateName << "' successfully !" << endl;
    } catch (const fs::filesystem_error& e) {
        cerr << "Failed to import template '" << templateName << "': " << e.what() << endl;
    }

}




void showHelpCommand() {
    cerr << "Usage: tmpl <command> <template_name | command_argument>" << endl << endl;
    cerr << "Example: " << endl;
    cerr << "tmpl create <template_name>  (Create a Template using the folder in which you ran this command)" << endl << endl;
    cerr << "tmpl get <template_name>  (Import the template you created to the selected folder.)" << endl << endl;
    cerr << "tmpl delete <template_name>  (Delete a Template.)" << endl << endl;
    cerr << "tmpl list  (Shows the list of templates you created.)" << endl << endl;
    cerr << "tmpl import <template_input_file> [template_name] (Import an external template that you can immediately use. In the template_name parameter, you must put the name you want to give to the template which, if left empty, will use the name of the Template file.)" << endl << endl;
    cerr << "tmpl export <template_name> <template_output_directory> (Export your template to share it.)" << endl << endl;
    cerr << "tmpl -v  (Shows the current version of tmpl installed on your computer.)" << endl << endl;
    cerr << "tmpl -u  (Check for updates on GitHub.)" << endl << endl;
    cerr << "tmpl help  (Shows this help.)" << endl << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {

        showHelpCommand();

        return 1;
    }

    string command = argv[1];

    if (argc == 2) {
        string argumentCommand = argv[1];
        if (fs::path(argumentCommand).extension() == ".tmpl") {
            importTemplate(argumentCommand);
            system("pause");
        }
    }

    if (argc == 3) {
        string argumentCommand = argv[2];

        if (command == "create") {
            createTemplate(argumentCommand);
        } else if (command == "get") {
            applyTemplate(argumentCommand);
        } else if (command == "delete") {
            deleteTemplate(argumentCommand);
        } else if (command == "import") {
            importTemplate(argv[2]);
        } else {
            showHelpCommand();
            return 1;
        }

    } else if (argc > 3) {

        if (command == "export") {
            string templateName = argv[2];
            string templateOutputDirectory = argv[3];
            exportTemplate(templateName, templateOutputDirectory);
        } else if (command == "import") {
            string templateInputFile = argv[2];
            string templateNewName = argv[3];
            importTemplate(templateInputFile, templateNewName);
        } else {
            showHelpCommand();
            return 1;
        }


    } else {

        if (command == "list") {
            templateList();
        } else if (command  == "-v") {
            actualVersion();
        } else if (command == "-u") {
            checkForUpdates();
        } else {
            showHelpCommand();
            return 1;
        }

    }
        

    return 0;
}
