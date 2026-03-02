// include/utils.h

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h> // For size_t

#define CRC16_POLY 0x1021 
#define CRC16_INITIAL 0xFFFF

uint16_t crc16_ccitt(const uint8_t *data, size_t length);

#endif // UTILS_H