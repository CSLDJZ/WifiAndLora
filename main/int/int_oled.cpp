#include "int_oled.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "soc/soc_caps.h"

#define INT_OLED_SPI_HOST SPI2_HOST
#define INT_OLED_SPI_CLOCK_HZ (8 * 1000 * 1000)

/* SSD1309 命令码，见数据手册第 9 章命令表 */
#define INT_OLED_CMD_SET_COLUMN_ADDRESS 0x21
#define INT_OLED_CMD_SET_PAGE_ADDRESS 0x22

/* 非 DMA 模式下单次传输上限等于 SPI FIFO 深度 */
#define INT_OLED_SPI_CHUNK_SIZE SOC_SPI_MAXIMUM_BUFFER_SIZE

static spi_device_handle_t s_spi_device = NULL;

/**
 * @brief 检查 GPIO 是否可用作输出。
 *
 * @param pin GPIO 编号。
 * @return 有效输出引脚返回 true，否则返回 false。
 */
static bool int_oled_is_valid_output_pin(int pin) {
    if (pin < 0) {
        return false;
    }
    return GPIO_IS_VALID_OUTPUT_GPIO((gpio_num_t)pin);
}

/**
 * @brief 将 GPIO 编号转换为引脚掩码。
 *
 * 未配置的引脚（-1）返回 0，避免对负值做移位。
 *
 * @param pin GPIO 编号。
 * @return 对应位的掩码，未配置时返回 0。
 */
static uint64_t int_oled_pin_mask(int pin) {
    if (pin < 0) {
        return 0;
    }
    return (uint64_t)1 << (unsigned)pin;
}

/**
 * @brief 检查 OLED 所需引脚是否有效且互不冲突。
 *
 * @return 引脚配置有效时返回 true，否则返回 false。
 */
static bool int_oled_pins_are_valid(void) {
    return int_oled_is_valid_output_pin(INT_OLED_PIN_MOSI) &&
           int_oled_is_valid_output_pin(INT_OLED_PIN_SCLK) &&
           int_oled_is_valid_output_pin(INT_OLED_PIN_DC) &&
           int_oled_is_valid_output_pin(INT_OLED_PIN_RESET) &&
           INT_OLED_PIN_MOSI != INT_OLED_PIN_SCLK &&
           INT_OLED_PIN_MOSI != INT_OLED_PIN_DC &&
           INT_OLED_PIN_MOSI != INT_OLED_PIN_RESET &&
           INT_OLED_PIN_SCLK != INT_OLED_PIN_DC &&
           INT_OLED_PIN_SCLK != INT_OLED_PIN_RESET &&
           INT_OLED_PIN_DC != INT_OLED_PIN_RESET &&
           (INT_OLED_PIN_CS == -1 ||
            (int_oled_is_valid_output_pin(INT_OLED_PIN_CS) &&
             INT_OLED_PIN_CS != INT_OLED_PIN_MOSI &&
             INT_OLED_PIN_CS != INT_OLED_PIN_SCLK &&
             INT_OLED_PIN_CS != INT_OLED_PIN_DC &&
             INT_OLED_PIN_CS != INT_OLED_PIN_RESET));
}

/**
 * @brief 通过 SPI 发送一段数据。
 *
 * 非 DMA 模式下单次传输不能超过 FIFO 深度，因此按 INT_OLED_SPI_CHUNK_SIZE
 * 在字节边界分段发送。
 *
 * @param data 待发送数据。
 * @param size 数据字节数。
 */
static void int_oled_transmit(const uint8_t *data, size_t size) {
    while (size > 0) {
        size_t chunk = (size > INT_OLED_SPI_CHUNK_SIZE) ? INT_OLED_SPI_CHUNK_SIZE : size;

        spi_transaction_t transaction;
        memset(&transaction, 0, sizeof(transaction));
        transaction.length = (uint32_t)(chunk * 8);
        transaction.tx_buffer = data;
        spi_device_transmit(s_spi_device, &transaction);

        data += chunk;
        size -= chunk;
    }
}

/**
 * @brief 将 D/C# 置低并发送命令字节。
 *
 * @param commands 命令字节数组。
 * @param size 命令字节数。
 */
static void int_oled_send_commands(const uint8_t *commands, size_t size) {
    gpio_set_level((gpio_num_t)INT_OLED_PIN_DC, 0);
    int_oled_transmit(commands, size);
}

/**
 * @brief 初始化 SSD1309、GPIO 和 SPI 总线。
 *
 * 引脚无效时直接返回，不做任何初始化。
 */
void int_oled_init(void) {
    if (s_spi_device != NULL) {
        return;
    }

    if (!int_oled_pins_are_valid()) {
        return;
    }

    gpio_config_t output_config;
    memset(&output_config, 0, sizeof(output_config));
    output_config.pin_bit_mask = int_oled_pin_mask(INT_OLED_PIN_DC) | int_oled_pin_mask(INT_OLED_PIN_RESET);
    output_config.mode = GPIO_MODE_OUTPUT;
    output_config.pull_up_en = GPIO_PULLUP_DISABLE;
    output_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    output_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&output_config);

    gpio_set_level((gpio_num_t)INT_OLED_PIN_DC, 1);
    gpio_set_level((gpio_num_t)INT_OLED_PIN_RESET, 1);

    spi_bus_config_t bus_config;
    memset(&bus_config, 0, sizeof(bus_config));
    bus_config.mosi_io_num = INT_OLED_PIN_MOSI;
    bus_config.miso_io_num = -1;
    bus_config.sclk_io_num = INT_OLED_PIN_SCLK;
    bus_config.quadwp_io_num = -1;
    bus_config.quadhd_io_num = -1;

    spi_bus_initialize(INT_OLED_SPI_HOST, &bus_config, SPI_DMA_DISABLED);

    spi_device_interface_config_t device_config;
    memset(&device_config, 0, sizeof(device_config));
    device_config.clock_speed_hz = INT_OLED_SPI_CLOCK_HZ;
    device_config.mode = 0;
    device_config.spics_io_num = INT_OLED_PIN_CS;
    device_config.queue_size = 1;
    spi_bus_add_device(INT_OLED_SPI_HOST, &device_config, &s_spi_device);

    gpio_set_level((gpio_num_t)INT_OLED_PIN_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level((gpio_num_t)INT_OLED_PIN_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    static const uint8_t init_commands[] = {
        0xAE,             // 关闭显示
        0xD5, 0x80,       // 设置显示时钟
        0xA8, 0x3F,       // 设置 1/64 多路复用
        0xD3, 0x00,       // 设置显示偏移
        0x40,             // 设置起始行为 0
        0xA1,             // 设置段重映射
        0xC8,             // 设置 COM 扫描方向
        0xDA, 0x12,       // 设置 COM 引脚配置
        0x81, 0x7F,       // 设置对比度
        0xD9, 0xF1,       // 设置预充电周期
        0xDB, 0x34,       // 设置 VCOMH 电平
        0xA4,             // 使用 GDDRAM 显示内容
        0xA6,             // 设置正常显示
        0x20, 0x00,       // 设置水平寻址模式
        0x21, 0x00, 0x7F, // 设置完整列范围
        0x22, 0x00, 0x07, // 设置完整页范围
        0xAF,             // 开启显示
    };
    int_oled_send_commands(init_commands, sizeof(init_commands));
}

/**
 * @brief 将完整帧缓冲区刷新到 OLED。
 *
 * @param framebuffer 指向至少 INT_OLED_BUFFER_SIZE 字节的帧缓冲区。
 */
void int_oled_display(const uint8_t *framebuffer) {
    if (framebuffer == NULL || s_spi_device == NULL) {
        return;
    }

    static const uint8_t address_commands[] = {
        INT_OLED_CMD_SET_COLUMN_ADDRESS, 0, INT_OLED_WIDTH - 1,
        INT_OLED_CMD_SET_PAGE_ADDRESS, 0, INT_OLED_PAGE_COUNT - 1,
    };
    int_oled_send_commands(address_commands, sizeof(address_commands));
    gpio_set_level((gpio_num_t)INT_OLED_PIN_DC, 1);
    int_oled_transmit(framebuffer, INT_OLED_BUFFER_SIZE);
}

/**
 * @brief 清空 OLED 显示内容。
 */
void int_oled_clear(void) {
    static const uint8_t blank_framebuffer[INT_OLED_BUFFER_SIZE] = {0};
    int_oled_display(blank_framebuffer);
}
