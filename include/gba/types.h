#ifndef GBA_TYPES_H
#define GBA_TYPES_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long long u64;
typedef signed char    s8;
typedef signed short   s16;
typedef signed int     s32;
typedef signed long long s64;

typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;

typedef u8  bool8;
typedef u32 bool32;

#define TRUE  1
#define FALSE 0
#define NULL ((void *)0)
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#endif /* GBA_TYPES_H */
