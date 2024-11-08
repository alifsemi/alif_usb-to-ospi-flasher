/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "RTE_Components.h"
#include "se_services_port.h"
#include "system_utils.h"
#include "ux_api.h"
#include "ux_dcd_dwc3.h"
#include "ux_device_class_cdc_acm.h"
#include "ux_device_stack.h"
#include "ux_system.h"
#include "ux_utility.h"
#if defined(RTE_Compiler_IO_STDOUT)
#include "Driver_Common.h"
#include "retarget_stdout.h"
#endif /* RTE_Compiler_IO_STDOUT */

#include "board.h"
#include "cdc_printf.h"
#include "prg_desc.h"
#include "prg_queue.h"
#include "program_thread.h"
#include "usb_framework_def.h"
#include "xmodem.h"

/* Define constants.  */
#define ONE_KB 1024
#define UX_DEMO_NS_SIZE (32 * ONE_KB)
#define BYTE_POOL_SIZE (64 * ONE_KB)
#define XMODEM_THREAD_STACK_SIZE (32 * ONE_KB)
#define PRG_THREAD_STACK_SIZE (4 * ONE_KB)

TX_BYTE_POOL byte_pool;
UCHAR pool_memory_area[BYTE_POOL_SIZE];

/* A pointer to store CDC-ACM device instance. */
UX_SLAVE_CLASS_CDC_ACM *g_cdc = UX_NULL;

VOID xmodem_thread_entry(ULONG arg);
VOID program_thread_entry(ULONG arg);
VOID ux_cdc_device0_instance_activate(VOID *activated);
VOID ux_cdc_device0_instance_acm_parameter(VOID *activated);
VOID ux_cdc_device0_instance_deactivate(VOID *deactivated);
VOID error_handler(void);

void clock_init(bool enable);

// xmodem thread
TX_THREAD xmodem_thread;
static uint8_t xmodem_buffer[8 * ONE_KB];

// flash programming thread
TX_THREAD program_thread;

uint8_t dma_buf[UX_DEMO_NS_SIZE] __attribute__((section("usb_dma_buf")));
UX_SLAVE_CLASS_CDC_ACM_PARAMETER cdc_acm0_parameter;

int main(void) {
    uint32_t error_code = 0;
    uint32_t service_error_code = 0;
    run_profile_t runp = {0};
    /* Initialize the SE services */
    se_services_port_init();
    clock_init(true);
    BOARD_Pinmux_Init();

    /* Get the current run configuration from SE */
    error_code = SERVICES_get_run_cfg(se_services_s_handle, &runp, &service_error_code);
    if (error_code) {
        printf("SE: get_run_cfg error = %d\n", error_code);
        return 0;
    }

    runp.phy_pwr_gating |= USB_PHY_MASK;

    /* Set the current run configuration to SE */
    error_code = SERVICES_set_run_cfg(se_services_s_handle, &runp, &service_error_code);
    if (error_code) {
        printf("SE: set_run_cfg error = %d\n", error_code);
        return 0;
    }

#if defined(RTE_Compiler_IO_STDOUT_User)
    int32_t ret;
    ret = stdout_init();
    if (ret != ARM_DRIVER_OK) {
        while (1) {
        }
    }
#endif

    tx_kernel_enter();
    return (0);
}

void tx_application_define(void *first_unused_memory) {
    UINT status;

    /* Initialize USBX Memory */
    status = ux_system_initialize(dma_buf, UX_DEMO_NS_SIZE, UX_NULL, 0x00);

    if (status != UX_SUCCESS) {
        error_handler();
    }

    /* The code below is required for installing the device portion of USBX.
       In this demo, DFU is possible and we have a call back for state change. */
    status = ux_device_stack_initialize(device_framework_high_speed, DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED, device_framework_full_speed,
                                        DEVICE_FRAMEWORK_LENGTH_FULL_SPEED, string_framework, STRING_FRAMEWORK_LENGTH, language_id_framework,
                                        LANGUAGE_ID_FRAMEWORK_LENGTH, UX_NULL);

    if (status != UX_SUCCESS) {
        error_handler();
    }
    /* Setting cdc acm activation and deavtivation functionality */
    cdc_acm0_parameter.ux_slave_class_cdc_acm_instance_activate = ux_cdc_device0_instance_activate;

    cdc_acm0_parameter.ux_slave_class_cdc_acm_instance_deactivate = ux_cdc_device0_instance_deactivate;

    cdc_acm0_parameter.ux_slave_class_cdc_acm_parameter_change = ux_cdc_device0_instance_acm_parameter;

    /* Register the class driver as CDC-ACM  */
    status = ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name, ux_device_class_cdc_acm_entry, 1, 0, (VOID *)&cdc_acm0_parameter);
    if (status != UX_SUCCESS) {
        error_handler();
    }

    status = _ux_dcd_dwc3_initialize();
    if (status != UX_SUCCESS) {
        error_handler();
    }

    status = tx_byte_pool_create(&byte_pool, "byte pool", pool_memory_area, BYTE_POOL_SIZE);
    if (status != UX_SUCCESS) {
        error_handler();
    }

    // Init queues for inter-thread communication
    status = prg_queue_init(&byte_pool);
    if (status != UX_SUCCESS) {
        error_handler();
    }

    // thread for serial communication (xmodem)
    CHAR *pointer = TX_NULL;
    status = tx_byte_allocate(&byte_pool, (VOID **)&pointer, XMODEM_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        error_handler();
    }
    status = tx_thread_create(&xmodem_thread, "xmodem thread", xmodem_thread_entry, 0, pointer, XMODEM_THREAD_STACK_SIZE, 4, 4, 2, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        error_handler();
    }

    // thread for programming flash
    status = program_thread_init(&byte_pool);
    if (status != TX_SUCCESS) {
        error_handler();
    }

    status = tx_byte_allocate(&byte_pool, (VOID **)&pointer, PRG_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        error_handler();
    }
    status = tx_thread_create(&program_thread, "program thread", program_thread_entry, 0, pointer, PRG_THREAD_STACK_SIZE, 8, 8, 2, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        error_handler();
    }
}

VOID xmodem_thread_entry(ULONG arg) {
    printf("Communication thread started\n");
    while (1) {
        if (g_cdc != UX_NULL) {
            printf("\nStarting xmodemReceive...\n");
            cdc_printf(g_cdc, "\nStarting xmodemReceive... (flash sector_size=%u)\n", get_program_thread_sector_size());
            int ret = xmodemReceive(xmodem_buffer, sizeof(xmodem_buffer), get_program_thread_sector_size());
            if (ret < 0) {
                printf("\nFailed, ret = %d\n", ret);
                cdc_printf(g_cdc, "\nFailed = %d\n", ret);
            } else {
                printf("\nSuccess, ret = %d\n", ret);
                cdc_printf(g_cdc, "\nSuccess, ret = %d\n", ret);
            }
        } else {
            tx_thread_sleep(10);
        }
    }
}

VOID ux_cdc_device0_instance_activate(VOID *activated) {
    /* Save the CDC instance.  */
    g_cdc = (UX_SLAVE_CLASS_CDC_ACM *)activated;

    // Set 1s timeout for receive
    ULONG timeout = TX_TIMER_TICKS_PER_SECOND;
    ux_device_class_cdc_acm_ioctl(g_cdc, UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_READ_TIMEOUT, (ULONG *)timeout);
}

VOID ux_cdc_device0_instance_deactivate(VOID *deactivated) { g_cdc = UX_NULL; }

VOID ux_cdc_device0_instance_acm_parameter(VOID *activated) { /* For now only notifications.  */ }

VOID error_handler(void) {
    clock_init(false);
    while (1) {
        __WFE();
    }
}

void clock_init(bool enable) {
    uint32_t service_error_code = 0;
    uint32_t error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_USB, enable, &service_error_code);
    if (error_code || service_error_code) {
        printf("SE: SDMMC 20MHz clock enable error_code=%u se_error_code=%u\n", error_code);
        return;
    }
}
