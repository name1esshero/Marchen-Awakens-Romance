/* Small, typed accessors shared by the map, scene, and link runtimes. */
#include "runtime_accessors.h"
#include "runtime_state.h"
#include "bitset.h"

#include "rom_section.h"
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
extern u8 gIwramField3FD5Offset[];
extern u8 gIwramPointer2860Offset[];
extern u8 gIwramField0810Offset[];
extern s32 GameStateGetEntry3894(s32 row,s32 group,s32 slot);
extern void sub_08056F90(void);

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
 return gSecondaryRuntime+0xE50;
}
AT("0000D628") void *GameStateGetRecord610(u32 index)
{
 return GAME_STATE_BASE+0x610+index*44;
}
AT("00001A34") void IwramSetField3FD5(u32 value)
{
 gIwramBase[(u32)gIwramField3FD5Offset]=value;
}
AT("00001B34") u32 IwramGetPointer2860(u32 index0)
{
 u32 index=index0;
 u32 base=(u32)gIwramBase;
 index*=4;
 base+=(u32)gIwramPointer2860Offset;
 index+=base;
 return *(u32 *)index;
}
AT("00001B4C") void IwramSetPointer2860(u32 index0,u32 value)
{
 u32 index=index0;
 u32 base=(u32)gIwramBase;
 index*=4;
 base+=(u32)gIwramPointer2860Offset;
 index+=base;
 *(u32 *)index=value;
}
AT("00005360") void *GameStateGetBuffer3F38(void)
{
 void **root=(void **)(gIwramBase+(u32)gMapGenerationRootOffset);
 u8 *base=*(u8 **)root;
 u32 offset=(u32)gMapGenerationRootOffset;
 offset-=164;
 return base+offset;
}

/* The map-generation state block also holds three parallel s16 tables that
 * the map and scene runtimes index by a signed 16-bit entry number.  The
 * original source narrows the index first and then walks the base pointer
 * one step at a time, which is what keeps the root offset live in a
 * register for the second add. */
#define GAME_STATE_TABLE_GET_S16(address,name,field) \
 AT(address) s32 name(s32 index) \
 { \
  u8 *iwram; \
  u32 offset; \
  u8 *base; \
  index=(s16)index; \
  iwram=gIwramBase; \
  offset=(u32)gMapGenerationRootOffset; \
  base=*(u8 **)(iwram+offset); \
  index*=2; \
  base+=(field); \
  base+=index; \
  return *(s16 *)base; \
 }

GAME_STATE_TABLE_GET_S16("000568B4",GameStateGetEntry2768,0x2768)
GAME_STATE_TABLE_GET_S16("00056EE0",GameStateGetEntry2AE0,0x2AE0)
GAME_STATE_TABLE_GET_S16("00057138",GameStateGetEntry31D0,0x31D0)

AT("000577E4") s32 GameStateGetEncounterValue(void)
{
 u8 *iwram=gIwramBase;
 u32 offset=(u32)gMapGenerationRootOffset;
 u8 *base=*(u8 **)(iwram+offset);
 offset-=180;
 return *(s16 *)(base+offset);
}
AT("00057844") s32 GameStateGetEncounterMode(void)
{
 u8 *iwram=gIwramBase;
 u32 offset=(u32)gMapGenerationRootOffset;
 u8 *base=*(u8 **)(iwram+offset);
 offset-=178;
 return *(s8 *)(base+offset);
}

/* Packed flag banks inside the map-generation state.  The index is narrowed
 * to 16 bits by the callers' script ABI before it reaches BitSet/BitTest. */
#define GAME_STATE_FLAG_SET(address,name,bank) \
 AT(address) void name(s32 index) \
 { \
  u8 *iwram; \
  u32 offset; \
  u8 *base; \
  s32 bit; \
  bit=index; \
  bit=(s16)bit; \
  iwram=gIwramBase; \
  offset=(u32)gMapGenerationRootOffset; \
  base=*(u8 **)(iwram+offset); \
  base+=(bank); \
  BitSet(base,bit,1); \
 }
#define GAME_STATE_FLAG_TEST(address,name,bank) \
 AT(address) s32 name(s32 index) \
 { \
  u8 *iwram; \
  u32 offset; \
  u8 *base; \
  s32 bit; \
  bit=index; \
  bit=(s16)bit; \
  iwram=gIwramBase; \
  offset=(u32)gMapGenerationRootOffset; \
  base=*(u8 **)(iwram+offset); \
  base+=(bank); \
  return (s16)BitTest(base,bit); \
 }

GAME_STATE_FLAG_SET("00056A34",GameStateSetFlag2730,0x2730)
GAME_STATE_FLAG_TEST("00056A60",GameStateTestFlag2730,0x2730)
GAME_STATE_FLAG_TEST("00056B2C",GameStateTestFlag26F8,0x26F8)

/** state+0x38B8 selects which row of the 0x3894 table is current. */
AT("00056130") s32 GameStateGetCurrentEntry3894(void)
{
 u8 *iwram;
 u32 offset;
 u8 *base;
 iwram=gIwramBase;
 offset=(u32)gMapGenerationRootOffset;
 base=*(u8 **)(iwram+offset);
 base+=0x38B8;
 return (s16)GameStateGetEntry3894(*base,0,0);
}

AT("00057174") void GameStateClearEntry31D0(s32 index)
{
 u8 *iwram;
 u32 offset;
 u8 *base;
 index=(s16)index;
 iwram=gIwramBase;
 offset=(u32)gMapGenerationRootOffset;
 base=*(u8 **)(iwram+offset);
 index*=2;
 base+=0x31D0;
 base+=index;
 *(u16 *)base=0;
 sub_08056F90();
}
