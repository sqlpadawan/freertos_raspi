#pragma once

/*
 * lwIP configuration for this project's FreeRTOS + Wi-Fi (STA) example.
 *
 * These values are based on Raspberry Pi's own official reference configs
 * for pico_w FreeRTOS examples — specifically the settings confirmed in
 * pico-examples/pico_w/wifi/lwipopts_examples_common.h and
 * pico-examples/pico_w/wifi/freertos/ping/lwipopts.h — verified against
 * that source rather than invented from scratch, since getting subtle
 * settings like NO_SYS wrong here can cause hard-to-diagnose hangs.
 *
 * See docs/11-wifi-networking.md for what each section is for, and
 * https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html for the full
 * list of options this project doesn't override (lwIP's own defaults
 * apply to anything not set here).
 */

/* --- OS integration ---
 * NO_SYS=0 is required for the FreeRTOS sys_arch port
 * (pico_cyw43_arch_lwip_sys_freertos / pico_lwip_freertos). This is the
 * single most important setting in this file — this is what makes lwIP
 * run as its own FreeRTOS task using real FreeRTOS mutexes/semaphores,
 * instead of the interrupt-only polling model non-RTOS pico-sdk examples
 * use. */
#define NO_SYS                      0

/* FreeRTOS task/mailbox sizing (confirmed values from the official
 * pico_w/wifi/freertos/ping example). */
#define TCPIP_THREAD_STACKSIZE      1024
#define DEFAULT_THREAD_STACKSIZE    2048
#define TCPIP_MBOX_SIZE             8
#define DEFAULT_RAW_RECVMBOX_SIZE   8
#define DEFAULT_TCP_RECVMBOX_SIZE   8
#define DEFAULT_UDP_RECVMBOX_SIZE   8
#define LWIP_TIMEVAL_PRIVATE        0

/* MEM_LIBC_MALLOC is documented upstream as incompatible with the
 * non-polling cyw43_arch variants (threadsafe_background / sys_freertos,
 * which is what this project uses) — must stay 0. */
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000

#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24

#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1

#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1

/* Neither the netconn nor BSD-socket APIs are enabled here — this example
 * only joins Wi-Fi and reads the assigned IP via netif_ip4_addr(), which
 * doesn't need either. Enable LWIP_SOCKET (and reconsider MEM_LIBC_MALLOC
 * above) when you're ready to add actual TCP/UDP socket code. */
#define LWIP_NETCONN                0
#define LWIP_SOCKET                 0

#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1

#define LWIP_STATS                  0
#define LWIP_DEBUG                  0
