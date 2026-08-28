cmake_minimum_required(VERSION 3.13)

# Default to the original Pico (RP2040) unless overridden:
#   cmake -B build -DPICO_BOARD=pico2 ..
if(NOT PICO_BOARD)
    set(PICO_BOARD pico CACHE STRING "Pico board type")
endif()

include(pico_sdk_import.cmake)

project(pico_freertos_devsetup C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# FreeRTOS kernel port selection: RP2040 vs RP2350 both live under GCC/RP2040
# in the raspberrypi/FreeRTOS-Kernel fork; the SDK/HAL layer handles the
# actual chip differences.
if(DEFINED ENV{FREERTOS_KERNEL_PATH})
    set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH})
else()
    set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)
endif()

include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

add_subdirectory(src)
