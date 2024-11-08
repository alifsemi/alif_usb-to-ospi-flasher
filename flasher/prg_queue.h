/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#pragma once

#include "tx_api.h"
#include "tx_queue.h"

UINT prg_queue_init(TX_BYTE_POOL *pool);
TX_QUEUE *get_prg_queue_free(void);
TX_QUEUE *get_prg_queue_ready(void);
