/*
 * Blink example: a single FreeRTOS task toggles the onboard LED.
 * See docs/04-project-structure.md and docs/08-freertos-config.md.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef PICO_DEFAULT_LED_PIN
#warning "PICO_DEFAULT_LED_PIN not defined for this board"
#endif

#define BLINK_TASK_STACK_SIZE   256
#define BLINK_TASK_PRIORITY     (tskIDLE_PRIORITY + 1)
#define BLINK_PERIOD_MS         500

static void blink_task(void *params) {
    (void)params;

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    bool led_on = false;
    for (;;) {
        led_on = !led_on;
        gpio_put(PICO_DEFAULT_LED_PIN, led_on);
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
