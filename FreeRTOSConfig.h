#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* See docs/08-freertos-config.md for explanations of these settings. */

#include <stdint.h>

/* NOTE: Do NOT declare `uint32_t SystemCoreClock;` here. pico-sdk's CMSIS
 * stub (src/rp2_common/cmsis/stub/CMSIS/Device/RP2040/Source/system_RP2040.c)
 * already defines this global. Redeclaring it here as a second definition
 * causes a "multiple definition of `SystemCoreClock'" link error, since
 * FreeRTOSConfig.h is pulled into many translation units across both
 * pico-sdk and the FreeRTOS kernel. */

#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                  0
#define configCPU_CLOCK_HZ                       133000000UL
#define configTICK_RATE_HZ                       1000
#define configMAX_PRIORITIES                     5
#define configMINIMAL_STACK_SIZE                 128
#define configMAX_TASK_NAME_LEN                  16
#define configUSE_16_BIT_TICKS                   0
#define configIDLE_SHOULD_YIELD                  1
#define configUSE_TASK_NOTIFICATIONS             1
#define configUSE_MUTEXES                        1
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES            1
#define configQUEUE_REGISTRY_SIZE                8
#define configUSE_QUEUE_SETS                     0
#define configUSE_TIME_SLICING                   1
#define configUSE_NEWLIB_REENTRANT                0

#define configSUPPORT_STATIC_ALLOCATION            0
#define configSUPPORT_DYNAMIC_ALLOCATION          1
#define configTOTAL_HEAP_SIZE                     (128 * 1024)
#define configAPPLICATION_ALLOCATED_HEAP           0

#define configUSE_IDLE_HOOK                       0
#define configUSE_TICK_HOOK                       0
#define configCHECK_FOR_STACK_OVERFLOW             2
#define configUSE_MALLOC_FAILED_HOOK               1
#define configUSE_DAEMON_TASK_STARTUP_HOOK          0

#define configUSE_TIMERS                          1
#define configTIMER_TASK_PRIORITY                 (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                  10
#define configTIMER_TASK_STACK_DEPTH               configMINIMAL_STACK_SIZE

/* RP2040/RP2350 SMP support - single core by default, see docs/08. */
#define configNUM_CORES                           1
#define configTICK_CORE                           0
#define configRUN_MULTIPLE_PRIORITIES              0
#define configUSE_CORE_AFFINITY                    0

/* Required by the RP2350 (Pico 2) Cortex-M33 port only; harmless/unused on
 * RP2040 (original Pico), but must be defined either way since the port
 * headers #error out if these are missing when building for RP2350. */
#define configENABLE_FPU                          1
#define configENABLE_MPU                          0
#define configENABLE_TRUSTZONE                     0
#define configMAX_SYSCALL_INTERRUPT_PRIORITY        (5 << 5)  /* RP2350/Cortex-M33 only */

/* Optional API inclusions */
#define INCLUDE_vTaskPrioritySet                  1
#define INCLUDE_uxTaskPriorityGet                 1
#define INCLUDE_vTaskDelete                       1
#define INCLUDE_vTaskSuspend                      1
#define INCLUDE_vTaskDelayUntil                   1
#define INCLUDE_vTaskDelay                        1
#define INCLUDE_xTaskGetSchedulerState             1
#define INCLUDE_xTaskGetCurrentTaskHandle           1
#define INCLUDE_uxTaskGetStackHighWaterMark         1
#define INCLUDE_eTaskGetState                      1
#define INCLUDE_xTimerPendFunctionCall              1
#define INCLUDE_xSemaphoreGetMutexHolder             1

/* Assert used by the kernel port; SDK's panic() halts and prints via stdio. */
void vAssertCalled(const char *file, int line);
#define configASSERT(x) if ((x) == 0) vAssertCalled(__FILE__, __LINE__)

#endif /* FREERTOS_CONFIG_H */
