#ifndef BMP_H
#define BMP_H

#include <stdint.h>

#define BMP_ID 0x4D42

typedef struct __attribute__((packed)) {
    uint16_t id;
    uint32_t size;
    uint32_t reserved;
    uint32_t offBits;
} BmpFileInfo;


#endif
