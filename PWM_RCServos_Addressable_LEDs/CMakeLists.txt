cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(PWM_RCServos_Addressable_LEDs C CXX ASM)

pico_sdk_init()

# === Create executable ===
add_executable(PWM_RCServos_Addressable_LEDs
    PWM_RCServos_Addressable_LEDs.c
    ws2812_parallel.c
)

# === Generate PIO header file ===
pico_generate_pio_header(PWM_RCServos_Addressable_LEDs ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# === Add compile definitions if needed ===
target_compile_definitions(PWM_RCServos_Addressable_LEDs PRIVATE PIN_DBG1=3)

# === Standard Pico configuration ===
pico_set_program_name(PWM_RCServos_Addressable_LEDs "PWM_RCServos_Addressable_LEDs")
pico_set_program_version(PWM_RCServos_Addressable_LEDs "0.1")
pico_enable_stdio_uart(PWM_RCServos_Addressable_LEDs 0)
pico_enable_stdio_usb(PWM_RCServos_Addressable_LEDs 0)

# === Link necessary libraries ===
target_link_libraries(PWM_RCServos_Addressable_LEDs
    pico_stdlib
    hardware_pwm
    hardware_dma
    hardware_gpio
    hardware_pio
)

# === Include current dir ===
target_include_directories(PWM_RCServos_Addressable_LEDs PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# === Final build setup ===
pico_add_extra_outputs(PWM_RCServos_Addressable_LEDs)
