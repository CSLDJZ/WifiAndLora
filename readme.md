WiFi / LoRa 网络巡检仪

本文约定开发环境、模块边界、主程序数据流和代码命名规范

1. 项目结构与模块职责

.
├── main.cpp                 程序入口：初始化并运行主循环
├── init.c / init.h          统一初始化注册表
├── data_types.h             应用层共享数据类型
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