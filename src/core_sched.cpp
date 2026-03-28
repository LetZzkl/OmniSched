#include "core_sched.h"
#include "utils.h"
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>
#include <cstring>

struct DeviceState {
    bool is_mtk;
    std::string all_cores;

    static const DeviceState& get() {
        static const DeviceState instance = []() {
            const std::string platform = execute_command("getprop ro.board.platform");
            const bool mtk = (platform.find("mt") != std::string::npos);
            const std::optional<std::string> cores = read_node_opt("/sys/devices/system/cpu/possible");
            return DeviceState{
                mtk,
                cores.value_or("0-7")
            };
        }();
        return instance;
    }
};

void init_daemon() {
    // 切換目錄至 "/" 並將標準輸出導向 /dev/null
    if (daemon(0, 0) < 0) {
        exit(EXIT_FAILURE);
    }
}

std::string get_best_cpu_governor(const std::string& avail_govs, bool is_mtk) {
    if (is_mtk) {
        if (avail_govs.find("sugov_ext") != std::string::npos) return "sugov_ext";
        if (avail_govs.find("schedutil") != std::string::npos) return "schedutil";
    } else {
        if (avail_govs.find("walt") != std::string::npos) return "walt";
        if (avail_govs.find("uag") != std::string::npos) return "uag";
        if (avail_govs.find("schedutil") != std::string::npos) return "schedutil";
    }
    return ""; 
}

void apply_memory_optimizations() {
    // 啟用 Multi-Gen LRU (Android 14+ 預設啟用)
    if (path_exists("/sys/kernel/mm/lru_gen/enabled")) {
        write_node("/sys/kernel/mm/lru_gen/enabled", "7"); 
    }
    // 最佳化 ZRAM 與 Page Swap 行為
    if (path_exists("/proc/sys/vm/swappiness")) {
        write_node("/proc/sys/vm/swappiness", "100");
    }
    // 降低記憶體分配延遲
    if (path_exists("/proc/sys/vm/watermark_scale_factor")) {
        write_node("/proc/sys/vm/watermark_scale_factor", "20"); 
    }
}

void apply_core_optimizations() {
    const auto& state = DeviceState::get(); // val state = DeviceState.get()
    
    apply_memory_optimizations();

    std::vector<std::string> policies;
    const char* cpufreq_dir_path = "/sys/devices/system/cpu/cpufreq/";
    
    if (DIR* dir = opendir(cpufreq_dir_path); dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strncmp(entry->d_name, "policy", 6) == 0) {
                policies.push_back(std::string(cpufreq_dir_path) + entry->d_name);
            }
        }
        closedir(dir);
    }
    std::sort(policies.begin(), policies.end());

    std::string cluster_little = "0-3"; 
    std::string cluster_mid = state.all_cores;
    std::string cluster_big = state.all_cores;

    if (policies.size() >= 3) {
        cluster_little = format_cpuset(read_node((policies[0] + "/affected_cpus").c_str()));
        cluster_mid    = format_cpuset(read_node((policies[1] + "/affected_cpus").c_str()));
        cluster_big    = format_cpuset(read_node((policies[2] + "/affected_cpus").c_str()));
    } else if (policies.size() == 2) {
        cluster_little = format_cpuset(read_node((policies[0] + "/affected_cpus").c_str()));
        cluster_big    = format_cpuset(read_node((policies[1] + "/affected_cpus").c_str()));
        cluster_mid    = cluster_big;
    }

    write_node("/dev/cpuset/top-app/cpus", state.all_cores.c_str());
    if (!cluster_mid.empty() && cluster_mid != cluster_big) {
        write_node("/dev/cpuset/foreground/cpus", combine_cpus(cluster_little, cluster_mid).c_str());
    } else {
        write_node("/dev/cpuset/foreground/cpus", state.all_cores.c_str());
    }
    
    const std::string sys_bg_cpus = combine_cpus(cluster_little, cluster_mid);
    write_node("/dev/cpuset/system-background/cpus", sys_bg_cpus.c_str());
    write_node("/dev/cpuset/background/cpus", cluster_little.c_str());

    // Android 14+ 
    if (path_exists("/dev/cpuset/top-app/uclamp.min")) {
        write_node("/dev/cpuset/top-app/uclamp.max", "max");
        write_node("/dev/cpuset/top-app/uclamp.min", "10");
        
        write_node("/dev/cpuset/background/uclamp.max", "50");
        write_node("/dev/cpuset/system-background/uclamp.max", "50");
    }

    if (DIR* cpu_dir = opendir("/sys/devices/system/cpu/"); cpu_dir) {
        struct dirent* entry;
        while ((entry = readdir(cpu_dir)) != nullptr) {
            if (strncmp(entry->d_name, "cpu", 3) == 0 && isdigit(entry->d_name[3])) {
                const std::string base_path = std::string("/sys/devices/system/cpu/") + entry->d_name + "/cpufreq/";
                const std::string avail_govs = read_node((base_path + "scaling_available_governors").c_str());
                const std::string best_gov = get_best_cpu_governor(avail_govs, state.is_mtk);
                
                if (!best_gov.empty()) {
                    write_node((base_path + "scaling_governor").c_str(), best_gov.c_str());
                }
            }
        }
        closedir(cpu_dir);
    }

    const char* adreno_path = "/sys/class/kgsl/kgsl-3d0/devfreq/governor";
    if (path_exists(adreno_path)) {
        write_node(adreno_path, "msm-adreno-tz");
    } else if (DIR* devfreq_dir = opendir("/sys/class/devfreq/"); devfreq_dir) {
        struct dirent* entry;
        while ((entry = readdir(devfreq_dir)) != nullptr) {
            if (entry->d_name[0] == '.') continue;
            
            const std::string gov_path = std::string("/sys/class/devfreq/") + entry->d_name + "/governor";
            const std::string avail_path = std::string("/sys/class/devfreq/") + entry->d_name + "/available_governors";
            const std::string avail_govs = read_node(avail_path.c_str());

            if (avail_govs.find("mali_ondemand") != std::string::npos) {
                write_node(gov_path.c_str(), "mali_ondemand");
            } else if (avail_govs.find("simple_ondemand") != std::string::npos) {
                write_node(gov_path.c_str(), "simple_ondemand");
            }
        }
        closedir(devfreq_dir);
    }
}
