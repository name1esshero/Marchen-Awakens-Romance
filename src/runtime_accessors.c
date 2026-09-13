/* Small, typed accessors shared by the map, scene, and link runtimes. */
#include "runtime_accessors.h"

#define AT(x) __attribute__((section(".rom." x)))
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
extern u8 gIwramField3FD5Offset[];
extern u8 gIwramPointer2860Offset[];
extern u8 gIwramField0810Offset[];

#define GAME_STATE_BASE ({ \
    void **root = (void **)(gIwramBase + (u32)gMapGenerationRootOffset); \
    (u8 *)*root; \
})

AT("00004CC0") u32 SioGetPlayerId(void)
{
 return (*(volatile u32 *)0x04000128<<26)>>30;
}
AT("00004CD0") u32 RuntimeGetByte4014(void)
{
 return **(u8 **)0x03004014;
}
AT("000067A4") void GameStateSetPointer2C(u32 index,void *value)
{
 u8 *base=GAME_STATE_BASE;
 index*=4;
 base+=0x2C;
 base+=index;
 *(void **)base=value;
}
AT("000067C0") void *GameStateGetPointer2C(u32 index)
{
 u8 *base=GAME_STATE_BASE;
 index*=4;
 base+=0x2C;
 base+=index;
 return *(void **)base;
}
AT("00006898") void GameStateSetField60E(u32 value)
{
 *(u16 *)(GAME_STATE_BASE+0x60E)=value;
}
AT("000068B4") u32 GameStateGetField60E(void)
{
 return *(u16 *)(GAME_STATE_BASE+0x60E);
}
AT("000068D0") void GameStateSetField12EC(s32 value)
{
 *(s8 *)(GAME_STATE_BASE+0x12EC)=value;
}
AT("000068EC") s32 GameStateGetField12EC(void)
{
 return *(s8 *)(GAME_STATE_BASE+0x12EC);
}
AT("00006AA0") s32 GameStateGetField12EE(void)
{
 return *(s16 *)(GAME_STATE_BASE+0x12EE);
}
AT("00006AC0") void GameStateSetField12EE(s32 value)
{
 *(u16 *)(GAME_STATE_BASE+0x12EE)=value;
}
AT("00006BD8") u32 IwramGetField0810(void)
{
 u8 *base=gIwramBase;
 u32 offset=0x81;
 offset<<=4;
 base+=offset;
 return *(u16 *)base;
}
AT("00008658") void *RuntimeGetBufferE50(void)
{
 return *(u8 **)0x03004020+0xE50;
}
AT("0000D628") void *GameStateGetRecord610(u32 index)
{
 return GAME_STATE_BASE+0x610+index*44;
}
