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

int main(void) {
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    bool led_on = false;
    for (;;) {
        led_on = !led_on;
        gpio_put(PICO_DEFAULT_LED_PIN, led_on);
        printf("bare LED %s\n", led_on ? "ON" : "OFF");
        sleep_ms(500);
    }
}
