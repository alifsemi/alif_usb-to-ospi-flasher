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
#include <stdarg.h>

#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"

int vcdc_printf(UX_SLAVE_CLASS_CDC_ACM* cdc, const char* fmt, va_list va);
int cdc_printf(UX_SLAVE_CLASS_CDC_ACM* cdc, const char* fmt, ...);
