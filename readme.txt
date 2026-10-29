========================================================================
第三方组件与开源许可声明

本项目在实现、构建与烧录过程中引用／使用了以下开源组件，
其著作权归各自作者所有，并遵循各自的许可证：

1. ESP-IDF v6.1.0
   用途：芯片启动、外设驱动、C 运行库、FreeRTOS 运行环境
   许可证：Apache License 2.0
   版权：Espressif Systems

2. FreeRTOS Kernel（随 ESP-IDF 分发）
   用途：任务调度与同步原语
   许可证：MIT License
   版权：Amazon.com, Inc. 及其关联公司

3. 构建与烧录工具
   - PlatformIO Core 6.2.0        Apache License 2.0
   - esptool 5.4.0                GNU GPL v2
   - xtensa-esp-elf GCC 15.2.0    GNU GPL 系列（含 GCC 运行时例外）

说明：
- 上述组件均以原始形式使用，本项目未修改其源码。
- 本项目源码中未引入其他第三方库。
- 后续如引入新的第三方库，须在本节补充库名、版本、作者及许可证。

许可证全文见各组件安装目录下的 LICENSE / COPYING 文件。
========================================================================

WiFi / LoRa 网络巡检仪

本文约定开发环境、模块边界、主程序数据流和代码命名规范

1. 项目结构与模块职责

.
├── platformio.ini           构建入口：板型、框架、源码目录配置
├── CMakeLists.txt           工程级 CMake 入口（指向 main 组件）
├── sdkconfig.defaults       Flash / PSRAM 的 ESP-IDF 默认配置
├── boards/                  自定义开发板定义（ESP32-S3 N16R8）
└── main/                    ESP-IDF 标准主组件，以下为组件内源码
    ├── CMakeLists.txt       组件构建描述，自动收集 .c/.cpp
    ├── main.cpp             程序入口：初始化并运行主循环
    ├── init.c / init.h      统一初始化注册表
    ├── data_types.h         应用层共享数据类型
    ├── app/
    │   ├── app_data_proc.c/.h   采集、整理并更新应用数据
    │   ├── app_view.c/.h        页面状态和显示内容组织
    │   ├── app_io.c/.h          SD 卡、串口等应用级输入/输出
    │   └── app_fsm.c/.h         页面/菜单有限状态机
    └── int/
        ├── int_wifi.cpp/.h      WiFi 接口
        ├── int_lora.cpp/.h      LoRa 接口
        ├── int_oled.cpp/.h      OLED 接口
        ├── int_sd.cpp/.h        SD 卡接口
        ├── int_uart.cpp/.h      UART/串口接口
        └── int_key.cpp/.h       按键接口


main 负责启动顺序和周期调度，不能涉及具体业务逻辑
init 负责按顺序调用模块初始化函数；初始化失败应明确返回失败
app 应用层，不直接承载芯片/驱动细节
int 是硬件接口层，封装对应设备的初始化、读写和状态查询。应用层通过这些接口访问外设
data_types.h 定义跨应用模块传递的数据结构，是数据载体，不负责业务处理或硬件操作

2. 开发环境与配置同步

(1) 统一编码与文件格式

所有源码、头文件、Markdown和项目配置统一使用UTF-8编码。新建文件建议保存为 UTF-8；团队成员不要用本地代码页或 ANSI 编码覆盖文件

若已有编辑器或构建配置与上述约定冲突，应先在团队内确认，再统一修改并提交，避免个人设置造成乱码或无关的大量差异

(2) VS Code 插件

团队成员应安装并启用：

PlatformIO 
Wokwi 硬件仿真

扩展推荐只负责提示安装，不会替代项目本身的构建和仿真配置。成员安装插件后仍需确认工程所需平台、开发板、框架及库依赖已正确配置

3. 程序运行与数据流

目标运行结构如下：

main
 ├─ init_all()：按注册顺序初始化外设和应用模块
 └─ while (true)
     ├─ app_data_proc：从 int_wifi / int_lora 等接口采集并整理数据
     ├─ app_view：读取应用数据，绘制当前页面并调用 int_oled 显示
     └─ app_io：按需调用 int_sd / int_uart 以保存数据或将数据上传至PC

data_types 的生产与消费

data_types.h 中的类型用于承载应用层共享数据。数据流为：

app_data_proc 从 WiFi/LoRa 等接口读取原始状态，校验并整理为应用层数据，负责生产数据载体
app_view 读取同一份数据，负责消费并生成 OLED 页面内容
app_io 读取同一份数据，负责消费并按业务要求保存到 SD 卡或通过 UART 输出

目前data_types.h中的数据类型暂定为一个队列，以便保存

4. 命名与代码规范

命名规则以现有 app_fsm.h/.c 的风格为基础，并用于后续新增代码：

应用文件以 app_ 开头，硬件接口以 int_ 开头，例如 app_fsm.c、int_wifi.cpp

函数小写并用模块前缀标明归属，例如 app_fsm_main()、init_all()

类型 / 结构体大写字母加下划线，类型名以 _t 结尾，例如 APP_FSM_PAGE_t、APP_FSM_ITEM_t

枚举常量 / 宏全大写，单词以下划线分隔，例如 PAGE_MAIN、APP_FSM_ITEM_COUNT

文件内私有全局 / 静态状态用 static 限制作用域，名称以 s_ 开头，例如 s_app_fsm_page、s_app_fsm_pages

结构体字段 / 局部变量小写，含义明确，例如 next_page、item_count
头文件保护宏由文件名转换为全大写并加 _H ，例如 APP_FSM_H

26/10/28 暂定的硬件模块清单

MCU: ESP32-S3 开发板, 其自带 wifi 模块
Lora射频: 商升特(Semtech)或其二次开发型的SX1262 外接模块
oled显示屏: 微雪电子(Waveshare)的SSD1309, 四线SPI接口
轻触按键: 品牌不相干, 数量4个起步
Micro SD卡模块: 用于存储数据，目前属于优先级最低的部分

可自行查找技术文档编写硬件接口

5. OLED 显示方案（前期决策）

当前屏幕为 SSD1309 128×64 单色屏，规划界面以状态、扫描结果和菜单信息为主。
现阶段不引入 LVGL 等完整 GUI 库：其控件、布局和内存开销超出当前需求，且项目尚无明确的复杂动画或触摸交互需求。OLED 接口使用 ESP-IDF SPI 驱动，并提供 1KB 页式帧缓冲的整屏刷新和清屏能力；帧缓冲按 SSD1309 GDDRAM 顺序排列，每页 128 字节，每字节对应竖直连续的 8 个像素。

此接口只负责屏幕初始化和帧缓冲传输，不包含字体或控件绘制。应用层后续可按页面需求使用精简绘制代码；若界面扩展到复杂字体、控件或动画，再评估引入 U8g2 或 LVGL。当前不增加第三方 GUI 依赖。

SPI 引脚在 `main/int/int_oled.h` 中通过 `INT_OLED_PIN_*` 宏配置，未配置时初始化会返回错误。`INT_OLED_PIN_CS` 可设为 `-1`（屏幕 CS# 已固定拉低）或指定 ESP32-S3 GPIO；MOSI、SCLK、D/C 和 RESET 必须配置为互不冲突的有效 GPIO。

6. 编译产物与成功判定

(1) 产物位置

.pio/build/esp32-s3-n16r8/          本工程的构建目录
    firmware.bin / firmware.elf     最终固件
    main/<相对路径>/<文件名>.o       各源文件的编译目标文件，例如 main/int/int_oled.cpp.o
    esp-idf/main/libmain.a          main 组件打包出的静态库

(2) 判定编译成功的依据

必须同时满足下列条件，才算本次编译成功：

终端最后出现 [SUCCESS] Took <秒数> seconds
日志中没有 error: 字样；本工程沿用 ESP-IDF 的 -Werror，警告同样会中断构建
firmware.bin 与 firmware.elf 的修改时间已更新
日志中出现 Successfully created ESP32-S3 image

注意：只看到 "Compiling xxx.o" 并不代表该文件编译通过，必须确认其后没有 error:，且对应的 .o 文件已生成。

(3) 避免浪费时间的检查方法

增量构建通常只需十几秒。以下操作会让 CMake 重新配置并全量重编整个 ESP-IDF 框架（本机约 20 分钟以上），不要随手触发：

新增、删除或重命名源文件（main/CMakeLists.txt 用 GLOB 收集源文件，文件列表一变化就重新配置）
修改 main/CMakeLists.txt
修改工程级 CMakeLists.txt、platformio.ini、sdkconfig.defaults

只改动已有文件的内容时，直接增量构建即可，日志里只会出现这些文件的 Compiling 行。

若只想确认某个文件能否通过编译：等日志出现该文件的 .o 生成记录、且其后没有 error:，即可中断构建，不必等链接和打包结束；随后确认 .pio/build/esp32-s3-n16r8/main/<相对路径>/<文件名>.o 已生成即为通过。

建议把编译输出同时写入日志，便于事后检索，而不是反复重跑：

pio run -e esp32-s3-n16r8 2>&1 | Tee-Object -FilePath .pio\build-log.txt
Select-String -Path .pio\build-log.txt -Pattern 'error:|warning:'

无需保留日志时，构建结束后删除 .pio\build-log.txt 即可。
