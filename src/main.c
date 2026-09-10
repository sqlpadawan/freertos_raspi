/*
 * Blink example: a single FreeRTOS task toggles the onboard LED.
 * See docs/04-project-structure.md and docs/08-freertos-config.md.
 *
 * LED handling is portable across plain boards (Pico, Pico 2 — LED on a
 * normal GPIO) and "W" wireless boards (Pico W, Pico 2 W — LED wired
 * through the CYW43439 wireless chip, not a plain GPIO). This is the
 * same #ifdef pattern used by pico-examples upstream. See
 * docs/01-hardware-setup.md for why W boards need this.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#if defined(CYW43_WL_GPIO_LED_PIN)
#include "pico/cyw43_arch.h"
#elif !defined(PICO_DEFAULT_LED_PIN)
#warning "Neither PICO_DEFAULT_LED_PIN nor CYW43_WL_GPIO_LED_PIN is defined for this board"
#endif

#define BLINK_TASK_STACK_SIZE   256
#define BLINK_TASK_PRIORITY     (tskIDLE_PRIORITY + 1)
#define BLINK_PERIOD_MS         500

/* Returns true on success. On W boards this also brings up the wireless
 * chip driver (cyw43_arch_init()), since the LED is wired through it —
 * no actual Wi-Fi/Bluetooth use happens here, this is the minimal init
 * needed just to reach the LED pin. */
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

static void blink_task(void *params) {
    (void)params;

    if (!led_init()) {
        printf("LED init failed\n");
    }

    bool led_on = false;
    for (;;) {
        led_on = !led_on;
        led_set(led_on);
        printf("LED %s\n", led_on ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
    }
}

void vAssertCalled(const char *file, int line) {
    printf("ASSERT FAILED: %s:%d\n", file, line);
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

/* Required because configUSE_MALLOC_FAILED_HOOK == 1 in FreeRTOSConfig.h */
void vApplicationMallocFailedHook(void) {
    printf("MALLOC FAILED - out of FreeRTOS heap (see configTOTAL_HEAP_SIZE)\n");
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

/* Required because configCHECK_FOR_STACK_OVERFLOW == 2 in FreeRTOSConfig.h */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    printf("STACK OVERFLOW in task: %s\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

int main(void) {
    stdio_init_all();

    xTaskCreate(
        blink_task,
        "Blink",
        BLINK_TASK_STACK_SIZE,
        NULL,
        BLINK_TASK_PRIORITY,
        NULL
    );

    vTaskStartScheduler();

    /* Should never reach here unless the scheduler fails to start
     * (e.g. out of heap for the idle/timer task). */
    for (;;) {}
    return 0;
}
