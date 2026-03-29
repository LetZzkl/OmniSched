# OmniSched | 全域灵动 🚀

![Android](https://img.shields.io/badge/Android-12%2B-3DDC84?style=flat-square&logo=android)
![C++](https://img.shields.io/badge/C++-17-00599C?style=flat-square&logo=c%2B%2B)
![Magisk](https://img.shields.io/badge/Magisk-23.0%2B-000000?style=flat-square)
![KernelSU](https://img.shields.io/badge/KernelSU-WebUI-cyan?style=flat-square)
![License](https://img.shields.io/badge/License-Mulan_PubL_2.0-red?style=flat-square)

OmniSched 是一款重构安卓调度逻辑的现代化 Android 底层优化模组。**v3.1 版本在 v3.0 的响应式架构基础上，导入了更精细的自动化场景识别与功耗控制**。我们彻底抛弃了传统模组中「固定轮询」与「属性硬编码」的做法，全面转向 **响应式事件驱动 (Event-Driven)** 与 **资料驱动配置**，并深度适配 Android 14+ 的底层特性。

## 💡 为什么选择 OmniSched？
市面上多数的开源调度模组往往针对特定机型硬编码，或使用极度耗电的 Shell 循环监听。一旦刷入不同架构的设备，轻则无效，重则导致系统卡死、发热耗电。
  * **零功耗常驻**：OmniSched 采用 C++ `epoll` + `inotify` 实现监听，仅当系统节点被窜改或配置更新时瞬间唤醒，达成近乎零的无效开销。
  * **全平台自适应**：自动侦测 CPU 拓扑结构（大小核丛集）与处理器平台（高通/联发科），动态套用最佳优化参数。

---
## 🌟 v3.1 新功能与核心黑科技
### 1. 🚀 智慧场景识别与自动游戏模式 (v3.1 新增)
  - **自动侦测前台应用**：守护进程会即时识别当前活动应用的包名。
  - **游戏白名单自动切换**：当侦测到用户定义在 `gamelist` 中的游戏（如原神、三角洲）时，系统会自动切换至 **Performance (性能模式)**；退出游戏后自动恢复 **Balance (均衡模式)**。

### 2. 🍃 全新 Lite Mode (轻量模式)
  - **效能与能耗的完美平衡**：在性能模式设定中新增「Lite」选项。
  - **精准算力钳制**：启用 Lite Mode 时，`uclamp.min` 将从 50 调降至 20，并放宽 CPU 调度器限制，在维持流畅度的同时大幅降低发热与耗电。

### 3. 🛠️ 三大 Root 环境原生适配 (Unified Root Adapter)
  - **APatch 支援**：除了 Magisk 与 KernelSU 外，v3.1 完整引入了对 APatch 的原生侦测与环境适配。
  - **自动属性注入**：透过 `RootAdapter` 统一管理 `resetprop`，确保渲染引擎劫持与底层参数能准确作用于不同 Root 环境。

### 4. ⚙️ Android 14+ 深度调度与记忆体优化
  - **MGLRU 强化**：强制启用 Multi-Gen LRU 并固定于层级 7，优化记忆体回收效率。
  - **渲染引擎劫持**：可选强制开启全域 **SkiaVK (Vulkan)** 渲染，并针对 Android 14+ 调整 `Graphite` 引擎参数，降低 GPU 负载。
  - **背景锁死小核**：将 Background 背景进程严格限制于小核丛集，并设定 `uclamp.max` 为 50，彻底改善待机功耗。

## 📥 安装与使用
1.  **环境要求**：Android 12+ (核心优化针对 A14+ 最佳化)。
2.  **安装**：在 Root 管理器（Magisk/KSU/APatch）中选择 `OmniSched-v3.x.x.zip` 安装并重启设备。
3.  **动态配置 (热重载)**：
**WebUI (推荐)**：KernelSU 用户可直接点击模组清单中的齿轮图示，开启图形化介面调整「性能配置」、「巡检间隔」、「Vulkan 开关」与「游戏名单」。
**手动修改**：编辑 `/data/adb/omnisched/config.json`，存档后守护进程将瞬间感应并套用新设定，**无需重启**。

## ⚖️ 授权协议 (License)
本专案采用 木兰公共许可证第2版 (Mulan PubL-2.0) 授权：
1. **强制要求下游开源**：任何基于本专案的修改、二次开发、程式码引用，其衍生专案必须同样采用 Mulan PubL-2.0 或同等 Copyleft 协议开源。禁止任何形式的闭源二改。
2. **必须保留署名与原出处**：在您的模组安装介面（UI）、README 说明、程式码注解中，必须显著标注原作者署名及本专案仓库连结。严禁抹除作者资讯或伪装成自研。
3. **禁止商业牟利与欺诈**：
  - **禁止倒卖**：严禁将本模组打包在闲鱼、淘宝等平台付费出售。
  - **禁止收费进群**：严禁将本模组作为付费群、付费频道的「独家内容」。
  - **禁止欺骗**：若发现下游专案违反上述条款，原作者有权对违规专案进行公开披露。
> 程式码可以免费分享，但劳动成果不容剽窃。尊重开源，遵守约定，否则请勿使用本专案程式码。

---
⚠️ 免责声明
本模组涉及 Android 底层 Cpuset 与 GPU/CPU 调度调整。刷机有风险，因使用本模组导致的任何资料遗失或设备异常，开发者不承担任何责任。建议刷入前做好重要资料备份。