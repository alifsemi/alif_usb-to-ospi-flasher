/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "prg_queue.h"

#include "prg_desc.h"

#define PRG_QUEUE_SIZE (4)
static TX_QUEUE free_prg_queue;
static TX_QUEUE ready_prg_queue;

UINT prg_queue_init(TX_BYTE_POOL *pool) {
    UCHAR *pointer = TX_NULL;

    // Q for free buffer "descriptors"
    UINT status = tx_byte_allocate(pool, (VOID **)&pointer, PRG_QUEUE_SIZE * sizeof(prg_desc), TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = tx_queue_create(&free_prg_queue, "free prg task", PRG_DESC_WORDS, pointer, PRG_QUEUE_SIZE * sizeof(prg_desc));
    if (status != TX_SUCCESS) {
        return status;
    }

    // Q for ready to program "descriptors"
    status = tx_byte_allocate(pool, (VOID **)&pointer, PRG_QUEUE_SIZE * sizeof(prg_desc), TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        return status;
    }

    status = tx_queue_create(&ready_prg_queue, "ready prg task", PRG_DESC_WORDS, pointer, PRG_QUEUE_SIZE * sizeof(prg_desc));
    return status;
}

TX_QUEUE *get_prg_queue_free(void) { return &free_prg_queue; }

TX_QUEUE *get_prg_queue_ready(void) { return &ready_prg_queue; }
