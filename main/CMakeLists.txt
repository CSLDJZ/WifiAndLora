file(GLOB_RECURSE app_sources "*.c" "*.cpp")

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS
        "."
        "app"
        "int"
    REQUIRES
        esp_driver_gpio
        esp_driver_spi
)
