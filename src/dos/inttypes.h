
#ifndef INT_TYPES_H_
#define INT_TYPES_H_

#if defined(__WATCOMC__) && __WATCOMC__ < 1200
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef long intptr_t;
typedef unsigned long uintptr_t;
#else
#include <stdint.h>
#endif

#endif	/* INT_TYPES_H_ */
