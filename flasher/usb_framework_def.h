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

/* Change the endpoint packet size later */
#define DEVICE_FRAMEWORK_LENGTH_FULL_SPEED 93
UCHAR device_framework_full_speed[] = {

    /*
    Device descriptor 18 bytes
    0x02 bDeviceClass: CDC class code
    0x00 bDeviceSubclass: CDC class sub code 0x00 bDeviceProtocol: CDC Device protocol
    idVendor & idProduct - https://www.linux-usb.org/usb.ids
    */

    0x12, 0x01, 0x00, 0x02,
    0x02, 0x00, 0x01, 0x40,
    0x25, 0x05, 0xa7, 0xa4,
    0x00, 0x01, 0x01, 0x02,
    0x03, 0x01,

    /* Configuration  descriptor 9 bytes  */
    0x09, 0x02, 0x4b, 0x00, 0x02, 0x01, 0x00, 0xC0, 0x01,

    /* Interface association descriptor  */
    0x08, 0x0b, 0x00,
    0x02, 0x02, 0x02, 0x00, 0x00,

    /* Communication Class Interface Descriptor */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,

    /* Header Functional Descriptor 5 bytes */
    0x05, 0x24, 0x00, 0x10, 0x01,

    /* ACM Functional Descriptor */
    0x04, 0x24, 0x02, 0x0f,
    /* Union Functional Descriptor 5 bytes */
    0x05, 0x24, 0x06, 0x00, 0x01,

    /* Call Management Functional Descriptor */
    0x05, 0x24, 0x01, 0x03, 0x01, /* Data interface */

    /* High Speed Notify  descriptor 7 bytes */
    0x07, 0x05, 0x83, 0x03, 0x0a, 0x00, 0x09,

    /* Data Interface Descriptor  */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,

    /* ACM HS IN Endpoint descriptor 7 bytes*/
    0x07, 0x05, 0x82, 0x02, 0x00, 0x02, 0x00,

    /* ACM HS OUT Endpoint descriptor 7 bytes */
        0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00

};

#define DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED 93

UCHAR device_framework_high_speed[] = {

    /*
    Device descriptor 18 bytes
    0x02 bDeviceClass: CDC class code
    0x00 bDeviceSubclass: CDC class sub code 0x00 bDeviceProtocol: CDC Device protocol
    idVendor & idProduct - https://www.linux-usb.org/usb.ids
    */

    0x12, 0x01, 0x00, 0x02,
    0x02, 0x00, 0x01, 0x40,
    0x25, 0x05, 0xa7, 0xa4,
    0x00, 0x01, 0x01, 0x02,
    0x03, 0x01,

    /* Configuration  descriptor 9 bytes  */
    0x09, 0x02, 0x4b, 0x00, 0x02, 0x01, 0x00, 0xC0, 0x01,

    /* Interface association descriptor */
    0x08, 0x0b, 0x00,
    0x02, 0x02, 0x02, 0x00, 0x00,

    /* Communication Class Interface Descriptor */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,

    /* Header Functional Descriptor 5 bytes */
    0x05, 0x24, 0x00, 0x10, 0x01,

    /* ACM Functional Descriptor */
    0x04, 0x24, 0x02, 0x0f,

    /* Union Functional Descriptor 5 bytes */
    0x05, 0x24, 0x06, 0x00, 0x01,

    /* Call Management Functional Descriptor */
    0x05, 0x24, 0x01, 0x03, 0x01, /* Data interface */

    /* High Speed Notify  descriptor 7 bytes  */
    0x07, 0x05, 0x83, 0x03, 0x0a, 0x00, 0x09,

    /* Data Interface Descriptor Requirement  */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,

    /* ACM HS IN Endpoint descriptor 7 bytes*/
    0x07, 0x05, 0x82, 0x02, 0x00, 0x02, 0x00,

    /* ACM HS OUT Endpoint descriptor 7 bytes */
        0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00

};

    /* String Device Framework :
     Byte 0 and 1 : Word containing the language ID : 0x0904 for US
     Byte 2       : Byte containing the index of the descriptor
     Byte 3       : Byte containing the length of the descriptor string
    */

#define STRING_FRAMEWORK_LENGTH 42
UCHAR string_framework[] = {

  (UCHAR) (0x0409), /* 0 Supported Language Code */
  (UCHAR) (0x0409 >> 8), /* 1 Supported Language Code */
  0x01, /* 2 Index */
  17, /* 3 bLength */
  'A', 'l', 'i', 'f', 'S', 'e', 'm', 'i', 'c', 'o', 'n', 'd', 'u', 'c', 't', 'o', 'r',

  (UCHAR) (0x0409), /* 0 Supported Language Code */
  (UCHAR) (0x0409 >> 8), /* 1 Supported Language Code */
  0x02, /* 2 Index */
  6, /* 3 bLength */
  'D', 'e', 'v', 'k', 'i', 't',

  (UCHAR) (0x0409), /* 0 Supported Language Code */
  (UCHAR) (0x0409 >> 8), /* 1 Supported Language Code */
  0x03, /* 2 Index */
  4, /* 3 bLength */
  '1', '2', '0', '0'

};

    /* Multiple languages are supported on the device, to add
       a language besides english, the unicode language code must
       be appended to the language_id_framework array and the length
     adjusted accordingly. */
#define LANGUAGE_ID_FRAMEWORK_LENGTH 2
UCHAR language_id_framework[] = {

    /* English. */
        0x09, 0x04
    };
