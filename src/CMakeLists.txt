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
