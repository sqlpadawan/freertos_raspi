/*
 * DIAGNOSTIC TARGET — no FreeRTOS at all.
 *
 * Purpose: isolate whether a "flashed, but no LED / no serial" symptom is
 * caused by something in the FreeRTOS/RP2350 configuration, or something
 * more fundamental (toolchain, board mismatch, hardware). If THIS blinks
 * and prints over serial but the main FreeRTOS-based blink target doesn't,
 * the problem is isolated to the FreeRTOS side of the project — not the
 * board, the flash process, or the base pico-sdk setup.
 *
 * See docs/09-troubleshooting.md.
 */

#include <stdio.h>
#include "pico/stdlib.h"

#if defined(CYW43_WL_GPIO_LED_PIN)
#include "pico/cyw43_arch.h"
#elif !defined(PICO_DEFAULT_LED_PIN)
#warning "Neither PICO_DEFAULT_LED_PIN nor CYW43_WL_GPIO_LED_PIN is defined for this board"
#endif

static bool led_init(void) {
#if defined(CYW43_WL_GPIO_LED_PIN)
    return cyw43_arch_init() == 0;
#elif defined(PICO_DEFAULT_LED_PIN)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return true;
#else
    return false;
#endif
}

static void led_set(bool led_on) {
#if defined(CYW43_WL_GPIO_LED_PIN)
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#elif defined(PICO_DEFAULT_LED_PIN)
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#else
    (void)led_on;
#endif
}

int main(void) {
    stdio_init_all();

    if (!led_init()) {
        printf("LED init failed\n");
    }

    bool led_on = false;
    for (;;) {
        led_on = !led_on;
        led_set(led_on);
        printf("bare LED %s\n", led_on ? "ON" : "OFF");
        sleep_ms(500);
    }
}
