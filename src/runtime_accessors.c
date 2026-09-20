/* Small, typed accessors shared by the map, scene, and link runtimes. */
#include "runtime_accessors.h"
#include "runtime_state.h"
#include "dialogue.h"
#include "game_state.h"

#include "rom_section.h"
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
extern u8 gIwramField3FD5Offset[];
extern u8 gSoundIrqModeOffset[];
extern u8 gIwramPointer2860Offset[];
extern u8 gIwramField0810Offset[];
extern s32 GameStateGetEntry3894(s32 row,s32 group,s32 slot);
extern void sub_08056F90(void);

#define GAME_STATE_BASE ({ \
    void **root = (void **)(gIwramBase + (u32)gMapGenerationRootOffset); \
    (u8 *)*root; \
})

#define IWRAM_FIELD_2870_OFFSET 0x2870
#define IWRAM_FIELD_2871_OFFSET 0x2871
#define IWRAM_FLAGS_0810_OFFSET 0x0810

#define IWRAM_FLAG_0810_MODE_0 (1 << 8)
#define IWRAM_FLAG_0810_MODE_1 (1 << 9)
#define IWRAM_FLAG_0810_MODE_2 (1 << 10)
#define IWRAM_FLAG_0810_MODE_3 (1 << 11)

#define DIALOGUE_TILE_BASE_DEFAULT  0x1F43
#define DIALOGUE_TILE_LIMIT_DEFAULT 0x1E42
#define DIALOGUE_MAP_BASE_DEFAULT   0x1D49
#define DIALOGUE_MAP_LIMIT_DEFAULT  0x1C4C
#define DIALOGUE_FLAGS_DEFAULT      0x1040

enum DialogueFlagMode
{
    DIALOGUE_FLAG_MODE_0,
    DIALOGUE_FLAG_MODE_1,
    DIALOGUE_FLAG_MODE_2,
    DIALOGUE_FLAG_MODE_3,
};

struct IwramFlags0810
{
    u16 value;
};

struct DialogueRuntimeConfig
{
    u16 flags;
    u16 tileBase;
    u16 tileLimit;
    u16 mapBase;
    u16 mapLimit;
    u8 unknown0A[18];
    u16 activeWindow;
};

extern struct DialogueRuntimeConfig gIwramField0810;

/** Initialize the shared dialogue/window buffers and their display defaults. */
AT("00008358") void InitializeDialogueRuntime(void)
{
    struct DialogueRuntimeConfig *config = &gIwramField0810;
    s32 zero;

    InitBufferTable2050((u8 *)config);
    zero = 0;
    config->tileBase = DIALOGUE_TILE_BASE_DEFAULT;
    config->tileLimit = DIALOGUE_TILE_LIMIT_DEFAULT;
    config->mapBase = DIALOGUE_MAP_BASE_DEFAULT;
    config->mapLimit = DIALOGUE_MAP_LIMIT_DEFAULT;
    config->flags = DIALOGUE_FLAGS_DEFAULT;
    IwramSetFlags0810(DIALOGUE_FLAG_MODE_0, TRUE);
    IwramSetFlags0810(DIALOGUE_FLAG_MODE_1, TRUE);
    IwramSetFlags0810(DIALOGUE_FLAG_MODE_2, FALSE);
    IwramSetFlags0810(DIALOGUE_FLAG_MODE_3, FALSE);
    config->activeWindow = zero;
    DialogueLoadWindowGraphics(1, TRUE);
}

/** @return This console's multiplayer id, bits 4-5 of REG_SIOCNT (the
 * hardware multi-play ID field). */
AT("00004CC0") u32 SioGetPlayerId(void)
{
 return (*(volatile u32 *)0x04000128<<26)>>30;
}
/** @return The byte pointed to by the pointer stored at fixed IWRAM slot
 * gLinkRuntime. See RuntimeGetByte4014U8() for the narrowed wrapper. */
AT("00004CD0") u32 RuntimeGetByte4014(void)
{
 return *gLinkRuntime;
}
/** Return the selected bit mask from the primary runtime's flag byte. */
AT("00004D90") u32 RuntimeTestFlag(u32 bit)
{
 u32 index=(u8)bit;
 u8 *runtime=gLinkRuntime;
 u32 result=1;
 result<<=index;
 result&=runtime[3];
 return result;
}
/** Store an indexed pointer into the game state's +0x2C table. */
AT("000067A4") void GameStateSetPointer2C(u32 index,void *value)
{
 u8 *base=GAME_STATE_BASE;
 index*=4;
 base+=0x2C;
 base+=index;
 *(void **)base=value;
}
/** @return An indexed pointer from the game state's +0x2C table. */
AT("000067C0") void *GameStateGetPointer2C(u32 index)
{
 u8 *base=GAME_STATE_BASE;
 index*=4;
 base+=0x2C;
 base+=index;
 return *(void **)base;
}
/** Set the game state's +0x60E u16 field. Meaning not yet recovered. */
AT("00006898") void GameStateSetField60E(u32 value)
{
 *(u16 *)(GAME_STATE_BASE+0x60E)=value;
}
/** @return The game state's +0x60E u16 field. */
AT("000068B4") u32 GameStateGetField60E(void)
{
 return *(u16 *)(GAME_STATE_BASE+0x60E);
}
/** Set the game state's +0x12EC signed byte field. Meaning not yet
 * recovered. */
AT("000068D0") void GameStateSetField12EC(s32 value)
{
 *(s8 *)(GAME_STATE_BASE+0x12EC)=value;
}
/** @return The game state's +0x12EC signed byte field. */
AT("000068EC") s32 GameStateGetField12EC(void)
{
 return *(s8 *)(GAME_STATE_BASE+0x12EC);
}
/** @return The current menu selection field at +0x12EE, or -1 for "nothing
 * selected" (see RuntimeResetSelection() in runtime_leaf.c). */
AT("00006AA0") s32 GameStateGetField12EE(void)
{
 return *(s16 *)(GAME_STATE_BASE+0x12EE);
}
/** Set the current menu selection field at +0x12EE. Pass -1 to clear it;
 * see GameStateGetField12EE(). */
AT("00006AC0") void GameStateSetField12EE(s32 value)
{
 *(u16 *)(GAME_STATE_BASE+0x12EE)=value;
}
/**
 * @brief Set or clear one of the four mode flags in the IWRAM +0x810 word.
 * @param mode Flag index from 0 through 3. Other values leave the word alone.
 * @param enabled Nonzero to set the flag, zero to clear it.
 */
AT("00006ADC")
void IwramSetFlags0810(s32 mode, s32 enabled)
{
    if (enabled)
    {
        switch (mode)
        {
        case 0:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value |=
                IWRAM_FLAG_0810_MODE_0;
            break;
        }
        case 1:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value |=
                IWRAM_FLAG_0810_MODE_1;
            break;
        }
        case 2:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value |=
                IWRAM_FLAG_0810_MODE_2;
            break;
        }
        case 3:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value |=
                IWRAM_FLAG_0810_MODE_3;
            break;
        }
        }
    }
    else
    {
        switch (mode)
        {
        case 0:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value &=
                ~IWRAM_FLAG_0810_MODE_0;
            break;
        }
        case 1:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value &=
                ~IWRAM_FLAG_0810_MODE_1;
            break;
        }
        case 2:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value &=
                ~IWRAM_FLAG_0810_MODE_2;
            break;
        }
        case 3:
        {
            u32 offset = IWRAM_FLAGS_0810_OFFSET >> 4;
            offset <<= 4;
            ((struct IwramFlags0810 *)(gIwramBase + offset))->value &=
                ~IWRAM_FLAG_0810_MODE_3;
            break;
        }
        }
    }
}
/** @return The u16 field at fixed IWRAM offset 0x810. Meaning not yet
 * recovered. */
AT("00006BD8") u32 IwramGetField0810(void)
{
 u8 *base=gIwramBase;
 u32 offset=0x81;
 offset<<=4;
 base+=offset;
 return *(u16 *)base;
}
/** @return The secondary runtime's +0xE50 buffer. */
AT("00008658") void *RuntimeGetBufferE50(void)
{
 return gSecondaryRuntime+0xE50;
}
/** @return One of the 44-byte records based at the game state's +0x610. */
AT("0000D628") void *GameStateGetRecord610(u32 index)
{
 return GAME_STATE_BASE+0x610+index*44;
}

/** @return One of the eight 32-byte effect slots in the main game state. */
AT("0000F994") void *GameStateGetEffectSlot(u32 index)
{
    struct IwramGameStateRootLayout *iwram =
        (struct IwramGameStateRootLayout *)gIwramBase;

    return iwram->gameState + GAME_STATE_EFFECT_SLOTS_OFFSET
           + index * GAME_STATE_EFFECT_SLOT_SIZE;
}
/** Set the byte field at the fixed IWRAM offset named gIwramField3FD5Offset.
 * See IwramGetField3FD5() for the signed reader. */
AT("00001A34") void IwramSetField3FD5(u32 value)
{
 gIwramBase[(u32)gIwramField3FD5Offset]=value;
}
/** The block this initialises is laid out as a 0x50-byte header, four 2 KiB
 * buffers, and then the four-entry table of pointers to them at +0x2050.
 * Named for that table's offset until a caller explains what it holds. */
AT("00001AF0") void InitBufferTable2050(u8 *base)
{
 *(u8 **)(base+0x2050)=base+0x50;
 *(u8 **)(base+0x2054)=base+0x850;
 *(u8 **)(base+0x2058)=base+0x1050;
 *(u8 **)(base+0x205C)=base+0x1850;
}
/** @return The byte field at gIwramField3FD5Offset, sign-extended. See
 * IwramSetField3FD5(). */
AT("00001A48") s32 IwramGetField3FD5(void)
{
 return (s8)gIwramBase[(u32)gIwramField3FD5Offset];
}
/** @return The signed sound IRQ mode stored in IWRAM. */
AT("00001AD8") s32 SoundGetIrqMode(void)
{
 u8 *base=gIwramBase;
 u32 offset=(u32)gSoundIrqModeOffset;
 base+=offset;
 return *(s8 *)base;
}
/** @return An indexed pointer from the fixed-IWRAM table at
 * gIwramPointer2860Offset. See IwramSetPointer2860(). */
AT("00001B34") u32 IwramGetPointer2860(u32 index0)
{
 u32 index=index0;
 u32 base=(u32)gIwramBase;
 index*=4;
 base+=(u32)gIwramPointer2860Offset;
 index+=base;
 return *(u32 *)index;
}
/** Store an indexed pointer into the fixed-IWRAM table at
 * gIwramPointer2860Offset. See IwramGetPointer2860(). */
AT("00001B4C") void IwramSetPointer2860(u32 index0,u32 value)
{
 u32 index=index0;
 u32 base=(u32)gIwramBase;
 index*=4;
 base+=(u32)gIwramPointer2860Offset;
 index+=base;
 *(u32 *)index=value;
}
/** Set the byte control field at fixed IWRAM offset 0x2870. */
AT("00001BB0") void IwramEnableField2870(void)
{
 u8 *base=gIwramBase;
 u32 offset=IWRAM_FIELD_2870_OFFSET;
 base+=offset;
 *base=1;
}
/** Clear the byte control field at fixed IWRAM offset 0x2870. */
AT("00001BC4") void IwramClearField2870(void)
{
 u8 *base=gIwramBase;
 u32 offset=IWRAM_FIELD_2870_OFFSET;
 base+=offset;
 *base=0;
}
/** @return The byte field at fixed IWRAM offset 0x2871. */
AT("00001BD8") u32 IwramGetField2871(void)
{
 u8 *base=gIwramBase;
 u32 offset=IWRAM_FIELD_2871_OFFSET;
 base+=offset;
 return *base;
}
/** @return The byte control field at fixed IWRAM offset 0x2870. */
AT("00001BEC") u32 IwramGetField2870(void)
{
 u8 *base=gIwramBase;
 u32 offset=IWRAM_FIELD_2870_OFFSET;
 base+=offset;
 return *base;
}
/** Write the byte control field at fixed IWRAM offset 0x2870. */
AT("00001C00") void IwramSetField2870(u32 value)
{
 u8 *base=gIwramBase;
 u32 offset=IWRAM_FIELD_2870_OFFSET;
 base+=offset;
 *base=value;
}
/** @return A buffer 164 bytes before the map-generation root offset's own
 * address value. Relationship to the game state root pointer unresolved. */
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
GAME_STATE_TABLE_GET_S16("00057138",ConsumableInventoryGetSlot,0x31D0)

/** @return A signed 16-bit encounter-related field stored 180 bytes before
 * the game state root. Exact meaning not yet recovered. */
AT("000577E4") s32 GameStateGetEncounterValue(void)
{
 u8 *iwram=gIwramBase;
 u32 offset=(u32)gMapGenerationRootOffset;
 u8 *base=*(u8 **)(iwram+offset);
 offset-=180;
 return *(s16 *)(base+offset);
}
/** @return A signed byte encounter-related field stored 178 bytes before
 * the game state root. Exact meaning not yet recovered. */
AT("00057844") s32 GameStateGetEncounterMode(void)
{
 u8 *iwram=gIwramBase;
 u32 offset=(u32)gMapGenerationRootOffset;
 u8 *base=*(u8 **)(iwram+offset);
 offset-=178;
 return *(s8 *)(base+offset);
}

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

/** Clear one slot of the game-state consumable inventory and rebuild its
 * compacted view.
 * @param index Inventory slot to clear, narrowed to 16 bits.
 * @return Nothing. */
AT("00057174") void ConsumableInventoryClearSlot(s32 index)
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
