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

pico_enable_stdio_usb(blink_bare 1)
pico_enable_stdio_uart(blink_bare 0)

pico_add_extra_outputs(blink_bare)
