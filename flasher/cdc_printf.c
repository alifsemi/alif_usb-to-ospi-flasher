/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include "cdc_printf.h"

#include <stdarg.h>

int vcdc_printf(UX_SLAVE_CLASS_CDC_ACM *cdc, const char *fmt, va_list va) {
    char pr_buffer[80];
    int n = vsnprintf(pr_buffer, sizeof(pr_buffer), fmt, va);
    if (n > sizeof(pr_buffer)) n = sizeof(pr_buffer);

    ULONG actual_length = 0;
    UINT status = ux_device_class_cdc_acm_write(cdc, pr_buffer, n, (ULONG *)&actual_length);
    if (status != UX_SUCCESS) {
        return -1;
    }

    return n;
}

int cdc_printf(UX_SLAVE_CLASS_CDC_ACM *cdc, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vcdc_printf(cdc, fmt, args);
    va_end(args);
    return ret;
}
