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

typedef struct prg_desc_ {
    uint32_t block_idx;
    uint32_t size;
    uint32_t error;
    uint8_t* buffer;
    uint32_t buffer_size;
} prg_desc;

#define PRG_DESC_WORDS (sizeof(prg_desc) / sizeof(uint32_t))
