add_executable(blink
    main.c
)

target_include_directories(blink PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..   # FreeRTOSConfig.h lives at repo root
)

target_link_libraries(blink
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

# On W (wireless) boards, the onboard LED is wired through the CYW43439
# chip rather than a plain GPIO. pico_cyw43_arch_none is the minimal
# library for this (LED control only, no actual Wi-Fi/lwip needed). This
# target only exists when pico-sdk detects a W board with the
# lib/cyw43-driver submodule fetched, so the guard is safe on non-W
# builds where it's simply absent. See docs/01-hardware-setup.md.
if (TARGET pico_cyw43_arch_none)
    target_link_libraries(blink pico_cyw43_arch_none)
endif()

# USB serial for printf(), no UART wiring required
pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)

pico_add_extra_outputs(blink)

# DIAGNOSTIC TARGET — bare-metal, no FreeRTOS. See src/blink_bare/main.c
# and docs/09-troubleshooting.md for what this is for.
add_executable(blink_bare
    blink_bare/main.c
)

target_link_libraries(blink_bare
    pico_stdlib
)

if (TARGET pico_cyw43_arch_none)
    target_link_libraries(blink_bare pico_cyw43_arch_none)
endif()

pico_enable_stdio_usb(blink_bare 1)
pico_enable_stdio_uart(blink_bare 0)

pico_add_extra_outputs(blink_bare)

# WIRELESS NETWORKING example — only added when pico-sdk detects lwIP is
# available (i.e. lib/lwip has been fetched, which the top-level
# CMakeLists.txt does automatically for W boards) AND the board actually
# supports it. See src/wifi_connect/ and docs/11-wifi-networking.md.
if (TARGET pico_cyw43_arch_lwip_sys_freertos)
    add_executable(wifi_connect
        wifi_connect/main.c
    )

    target_include_directories(wifi_connect PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/..            # FreeRTOSConfig.h lives at repo root
        ${CMAKE_CURRENT_LIST_DIR}/wifi_connect  # lwipopts.h and wifi_config.h
    )

    target_link_libraries(wifi_connect
        pico_stdlib
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_cyw43_arch_lwip_sys_freertos
    )

    pico_enable_stdio_usb(wifi_connect 1)
    pico_enable_stdio_uart(wifi_connect 0)

    pico_add_extra_outputs(wifi_connect)
endif()
