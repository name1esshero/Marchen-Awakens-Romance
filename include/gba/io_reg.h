#ifndef GBA_IO_REG_H
#define GBA_IO_REG_H

#include "gba/types.h"

#define REG_BASE 0x04000000

#define REG_ADDR_DISPCNT   (REG_BASE + 0x00)
#define REG_ADDR_DISPSTAT  (REG_BASE + 0x04)
#define REG_ADDR_VCOUNT    (REG_BASE + 0x06)
#define REG_ADDR_BG0CNT    (REG_BASE + 0x08)
#define REG_ADDR_KEYINPUT  (REG_BASE + 0x130)
#define REG_ADDR_IE        (REG_BASE + 0x200)
#define REG_ADDR_IF        (REG_BASE + 0x202)
#define REG_ADDR_WAITCNT   (REG_BASE + 0x204)
#define REG_ADDR_IME       (REG_BASE + 0x208)

#define REG_DISPCNT  (*(vu16 *)REG_ADDR_DISPCNT)
#define REG_DISPSTAT (*(vu16 *)REG_ADDR_DISPSTAT)
#define REG_VCOUNT   (*(vu16 *)REG_ADDR_VCOUNT)
#define REG_KEYINPUT (*(vu16 *)REG_ADDR_KEYINPUT)
#define REG_IE       (*(vu16 *)REG_ADDR_IE)
#define REG_IF       (*(vu16 *)REG_ADDR_IF)
#define REG_IME      (*(vu16 *)REG_ADDR_IME)

/* memory map */
#define EWRAM_START 0x02000000
#define IWRAM_START 0x03000000
#define PLTT        0x05000000
#define VRAM        0x06000000
#define OAM         0x07000000

#define BG_CHAR_ADDR(n) ((void *)(VRAM + (n) * 0x4000))

/* key bits, as read from REG_KEYINPUT (active low) */
#define KEY_A      0x0001
#define KEY_B      0x0002
#define KEY_SELECT 0x0004
#define KEY_START  0x0008
#define KEY_RIGHT  0x0010
#define KEY_LEFT   0x0020
#define KEY_UP     0x0040
#define KEY_DOWN   0x0080
#define KEY_R      0x0100
#define KEY_L      0x0200

#endif /* GBA_IO_REG_H */
