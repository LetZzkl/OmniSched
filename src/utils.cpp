#include "utils.h"
#include <cstdio>
#include <sys/stat.h>
#include <algorithm>

bool write_node(const char* path, const char* value) {
    const std::string current_value = read_node(path); // 模擬 val
    if (current_value == value) return true; // 減少無效 I/O

    if (FILE* file = fopen(path, "w"); file) { // 模擬 let scope
        fputs(value, file);
        fclose(file);
        return true;
    }
    return false;
}

std::string read_node(const char* path) {
    char buffer[256];
    std::string value;
    if (FILE* file = fopen(path, "r"); file) {
        if (fgets(buffer, sizeof(buffer), file)) {
            value = buffer;
            while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
                value.pop_back();
            }
        }
        fclose(file);
    }
    return value;
}

std::optional<std::string> read_node_opt(const char* path) {
    const std::string val = read_node(path);
    return val.empty() ? std::nullopt : std::make_optional(val); // 模擬 Elvis 操作符
}

std::string format_cpuset(const std::string& cpus) {
    if (cpus.empty()) return cpus;
    std::string result = cpus;
    std::replace(result.begin(), result.end(), ' ', ',');
    while (!result.empty() && (result.back() == ',' || result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

std::string combine_cpus(const std::string& cpus1, const std::string& cpus2) {
    if (cpus1.empty()) return cpus2;
    if (cpus2.empty()) return cpus1;
    return cpus1 + "," + cpus2;
}

bool path_exists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

std::string execute_command(const char* cmd) {
    char buffer[128];
    std::string result = "";
    if (FILE* pipe = popen(cmd, "r"); pipe) {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
    }
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}
