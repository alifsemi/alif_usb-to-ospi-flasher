/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include "program_thread.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "Driver_Flash.h"
#include "Driver_GPIO.h"
#include "board.h"
#include "prg_desc.h"
#include "prg_queue.h"
#include "tx_api.h"

// External OSPI flash
bool init_ext_flash(void);
#define OSPI_RESET_PORT LP
#define OSPI_RESET_PIN 7
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(OSPI_RESET_PORT);
static ARM_DRIVER_GPIO *OSPI_GPIODrv = &ARM_Driver_GPIO_(OSPI_RESET_PORT);
extern ARM_DRIVER_FLASH ARM_Driver_Flash_(1);
static ARM_DRIVER_FLASH *ptrDrvFlash = &ARM_Driver_Flash_(1);

#define NUM_OF_PD (2)
static prg_desc pds[NUM_OF_PD];

UINT program_thread_init(TX_BYTE_POOL *pool) {
    // Allocate the empty programming buffer messages "descriptors"
    for (int ii = 0; ii < NUM_OF_PD; ii++) {
        UCHAR *pointer;
        UINT tx_status = tx_byte_allocate(pool, (VOID **)&pointer, get_program_thread_sector_size(), TX_NO_WAIT);
        if (tx_status != TX_SUCCESS) {
            return tx_status;
        }
        pds[ii].block_idx = 0;
        pds[ii].error = 0;
        pds[ii].size = 0;
        pds[ii].buffer = pointer;
        pds[ii].buffer_size = get_program_thread_sector_size();
    }

    return TX_SUCCESS;
}

// Write the block received by xmodemReceive to external flash.
// Programming thread can erase/program the flash while
// another thread is receiving data for the next sector
VOID program_thread_entry(ULONG arg) {
    printf("Programming thread started\n");

    printf("Init flash device\n");
    bool flash_status = init_ext_flash();
    if (!flash_status) {
        printf("ERR: flash init failed\n");
    }

    // Send the initial free buffers to free Q
    for (int ii = 0; ii < NUM_OF_PD; ii++) {
        if (!flash_status) {
            pds[ii].error = 0xFF;
        }
        tx_queue_send(get_prg_queue_free(), &pds[ii], TX_WAIT_FOREVER);
    }

    while (1) {
        // Wait for programming task
        prg_desc pd;
        tx_queue_receive(get_prg_queue_ready(), &pd, TX_WAIT_FOREVER);

        // Stop programming if there are errors
        if (pd.error) {
            while (1) {
                tx_thread_sleep(10);
            }
        }

        BOARD_LED2_Control(BOARD_LED_STATE_HIGH);
        const uint32_t sector_addr = pd.block_idx * ptrDrvFlash->GetInfo()->sector_size;
        int32_t ret = ptrDrvFlash->EraseSector(sector_addr);
        if (ret != ARM_DRIVER_OK) {
            pd.error++;
        }

        const uint32_t page_size = ptrDrvFlash->GetInfo()->page_size;
        const uint32_t data_width = ptrDrvFlash->GetCapabilities().data_width;
        const uint32_t cnt = page_size / (1 << data_width);  // data width 0 = 8 bytes, 1 = 16 bytes, 2 = 32 bytes
        const uint32_t pages = (pd.size + page_size - 1) / page_size;

        for (unsigned int ii = 0; ii < pages; ii++) {
            uint32_t addr = sector_addr + ii * page_size;
            int32_t ret = ptrDrvFlash->ProgramData(addr, &pd.buffer[ii * page_size], cnt);
            if (ret != (int32_t)cnt) {
                pd.error++;
            }
        }

        BOARD_LED2_Control(BOARD_LED_STATE_LOW);
        tx_queue_send(get_prg_queue_free(), &pd, TX_WAIT_FOREVER);
    }
}

static void toggle_ospi_flash_reset(void) {
    OSPI_GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    OSPI_GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
}

bool init_ext_flash(void) {
    toggle_ospi_flash_reset();

    int32_t ret = ptrDrvFlash->Initialize(NULL);  // Initialize ext-FLASH
    if (ret != ARM_DRIVER_OK) {
        printf("OSPI Flash: Init failed, error = %" PRIi32 "\n", ret);
        return false;
    }

    ret = ptrDrvFlash->PowerControl(ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("OSPI Flash: Power up failed, error = %" PRIi32 "\n", ret);
        return false;
    }

    return true;
}

uint32_t get_program_thread_sector_size(void) { return ptrDrvFlash->GetInfo()->sector_size; }
