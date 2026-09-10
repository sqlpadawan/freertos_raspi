/*
 * Wi-Fi connect example: a FreeRTOS task joins a Wi-Fi network and prints
 * the assigned IP address over serial once connected.
 *
 * Requires wifi_config.h with real network credentials — copy
 * wifi_config.h.example to wifi_config.h in this same directory and fill
 * it in (wifi_config.h is gitignored, so your credentials stay local).
 *
 * Only builds when pico-sdk detects lwIP is available (i.e. lib/lwip has
 * been fetched) on a W board — see src/CMakeLists.txt and
 * docs/11-wifi-networking.md.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "FreeRTOS.h"
#include "task.h"

#include "wifi_config.h"

#define WIFI_CONNECT_TIMEOUT_MS   30000
#define WIFI_TASK_STACK_SIZE      1024
#define WIFI_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)

static void wifi_connect_task(void *params) {
    (void)params;

    /* cyw43_arch_init() is called from inside a task (rather than in
     * main() before the scheduler starts) since it and the Wi-Fi connect
     * call below both rely on FreeRTOS delays/timers to work correctly —
     * safest to only call them once the scheduler is actually running. */
    if (cyw43_arch_init()) {
        printf("cyw43_arch_init failed\n");
        vTaskDelete(NULL);
        return;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi SSID '%s'...\n", WIFI_SSID);

    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS
    );

    if (result) {
        printf("Wi-Fi connect failed, error %d\n", result);
    } else {
        printf("Connected. IP address: %s\n",
               ip4addr_ntoa(netif_ip4_addr(netif_default)));
    }

    /* Keep this task alive so the connection (and its background service,
     * managed internally by pico_cyw43_arch_lwip_sys_freertos) keeps
     * running. A real application would do useful networking work here —
     * open a socket, make requests, etc. — instead of just looping. */
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));
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
        wifi_connect_task,
        "WifiConnect",
        WIFI_TASK_STACK_SIZE,
        NULL,
        WIFI_TASK_PRIORITY,
        NULL
    );

    vTaskStartScheduler();

    /* Should never reach here unless the scheduler fails to start. */
    for (;;) {}
    return 0;
}
