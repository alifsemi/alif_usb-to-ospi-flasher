/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include "xmodem_callbacks.h"

#include "prg_desc.h"
#include "prg_queue.h"
#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"

extern UX_SLAVE_CLASS_CDC_ACM *g_cdc;

static uint8_t rx_buffer[1024];
static uint8_t tx_buffer[512];

// Get single byte, called from xmodemReceive
static uint8_t *rx_data_read_ptr = rx_buffer;
static uint8_t *rx_data_end_ptr = rx_buffer;

// When one full sector (or last sector) is received, send it to
// programming thread.
int xmodemReceivedBlock(uint32_t block_idx, uint8_t *ptr, int len) {
    // Wait for free prg desc
    prg_desc pd;
    tx_queue_receive(get_prg_queue_free(), &pd, TX_WAIT_FOREVER);

    // Programmer reported error(s) --> stop xmodem
    if (pd.error) {
        tx_queue_send(get_prg_queue_ready(), &pd, TX_WAIT_FOREVER);
        return -1;
    }

    pd.size = len;
    pd.block_idx = block_idx;
    memcpy(pd.buffer, ptr, len);

    tx_queue_send(get_prg_queue_ready(), &pd, TX_WAIT_FOREVER);
    return len;
}

int _inbyte(unsigned short timeout) {
    if (!g_cdc) {  // Should not happen, but wait if we don't have the cdc device yet
        int timeout_ticks = timeout * TX_TIMER_TICKS_PER_SECOND / 1000;
        while (!g_cdc && timeout_ticks >= 0) {
            tx_thread_sleep(1);
            timeout_ticks--;
        }
    }

    // We do not care about the requested timeout for cdc read
    // A constant 1s timeout is set using IOCTL call in ux_cdc_device0_instance_activate()

    // Give a buffered byte
    if (rx_data_read_ptr != rx_data_end_ptr) {
        return *rx_data_read_ptr++;
    } else {
        // Empty buffer --> receive more
        ULONG actual_length = 0;
        UINT status = ux_device_class_cdc_acm_read(g_cdc, rx_buffer, sizeof(rx_buffer), &actual_length);
        if (status == UX_SUCCESS && actual_length > 0) {
            rx_data_end_ptr = &rx_buffer[actual_length];
            rx_data_read_ptr = &rx_buffer[0];
            return *rx_data_read_ptr++;
        }
    }

    return -1;
}

// Put single byte, called from xmodemReceive
// Don't care about the status, the protocol has retry functionality
void _outbyte(int c) {
    if (!g_cdc) {
        return;
    }

    ULONG actual_length = 0;
    tx_buffer[0] = c;
    ux_device_class_cdc_acm_write(g_cdc, tx_buffer, 1, (ULONG *)&actual_length);
}
