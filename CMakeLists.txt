cmake_minimum_required(VERSION 3.13)

# Default to the original Pico (RP2040) unless overridden:
#   cmake -B build -DPICO_BOARD=pico2 .
if(NOT PICO_BOARD)
    set(PICO_BOARD pico CACHE STRING "Pico board type")
endif()

# W (wireless) boards need extra pico-sdk submodules that aren't fetched
# by default (see docs/02-host-toolchain-setup.md — skipped for non-W
# boards since they're unused there and lwip in particular is large).
# Rather than relying on a manual step that's easy to forget, detect this
# from the board name and fetch whatever's missing automatically, before
# pico_sdk_init() needs it.
if(PICO_BOARD MATCHES "_w$")
    # Only attempt this if the outer pico-sdk submodule itself is already
    # present — if it isn't, skip straight to pico_sdk_import.cmake's own
    # clearer error message about that instead of failing here with a
    # confusing git pathspec error.
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk/pico_sdk_init.cmake")
        # Each entry: submodule path relative to lib/pico-sdk, and a real,
        # always-present file inside it to check for (neither of these
        # submodules has its own top-level CMakeLists.txt — pico-sdk
        # references their source files directly — so we can't just check
        # for that).
        set(_W_BOARD_SUBMODULES
            "lib/cyw43-driver|src/cyw43_ll.c"
            "lib/lwip|src/core/init.c"
        )
        foreach(_entry ${_W_BOARD_SUBMODULES})
            string(REPLACE "|" ";" _entry_list ${_entry})
            list(GET _entry_list 0 _submodule_path)
            list(GET _entry_list 1 _marker_file)
            set(_full_marker "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk/${_submodule_path}/${_marker_file}")
            if(NOT EXISTS "${_full_marker}")
                message(STATUS "Board '${PICO_BOARD}' is a wireless (W) board - fetching ${_submodule_path}...")
                execute_process(
                    COMMAND git submodule update --init ${_submodule_path}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib/pico-sdk"
                    RESULT_VARIABLE _fetch_result
                )
                if(NOT _fetch_result EQUAL 0)
                    message(FATAL_ERROR
                        "Failed to auto-fetch the ${_submodule_path} submodule needed "
                        "for board '${PICO_BOARD}'. Run this manually, then re-run cmake:\n"
                        "    git -C lib/pico-sdk submodule update --init ${_submodule_path}")
                endif()
            endif()
        endforeach()
    endif()
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
