#include "core_sched.h"
#include <unistd.h>
#include <thread>
#include <chrono>

int main() {
    init_daemon();

    while (true) {
        apply_core_optimizations();
        std::this_thread::sleep_for(std::chrono::minutes(15));
    }

    return 0;
}
