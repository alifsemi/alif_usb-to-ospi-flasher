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

int xmodemReceivedBlock(uint32_t block_idx, uint8_t *ptr, int len);

int xmodemReceive(unsigned char *dest, int destsz, int block_size);

int _inbyte(unsigned short t);
void _outbyte(int c);