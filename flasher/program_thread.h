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

#include <inttypes.h>

#include "tx_api.h"

UINT program_thread_init(TX_BYTE_POOL *pool);
VOID program_thread_entry(ULONG arg);

uint32_t get_program_thread_sector_size(void);
