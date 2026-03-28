#pragma once
#include <string>

struct OmniConfig {
    int poll_interval_seconds = 900;
    bool force_vulkan = true;
    bool background_little_core_only = true;
    
    static const OmniConfig& get();
    static void reload();
};
