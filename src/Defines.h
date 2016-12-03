
#include <stdint.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#ifndef __DEFINES_H
#define __DEFINES_H

#define LOG_ERROR(fmt, ...)                                 \
    do {                                                    \
        fprintf(stderr, "%s:%d:%s(): " fmt "\n",            \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);   \
    } while (0)

#define MIN(a, b) (((a) < (b))? (a) : (b))
#define MAX(a, b) (((a) > (b))? (a) : (b))

#define nullptr NULL

#endif // __DEFINES_H
