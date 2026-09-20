/* Packed game-state flag banks used by scripts, maps, and inventory data. */
#include "flags.h"

#include "bitset.h"
#include "runtime_accessors.h"
#include "rom_section.h"

extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
extern void CpuFill(void *destination, u32 size, u32 value);

/* This spelling preserves the original literal and load order under agbcc. */
#define ORDERED_GAME_STATE_BASE ({ \
    u8 *iwram = gIwramBase; \
    u32 offset = (u32)gMapGenerationRootOffset; \
    *(u8 **)(iwram + offset); \
})

/** Enable or disable a script-visible game flag. */
AT("00006760") void GameStateSetFlagsAC(u32 bit, s32 enabled)
{
    u8 *iwram = gIwramBase;
    u32 offset = (u32)gMapGenerationRootOffset;
    u8 *state = *(u8 **)(iwram + offset);

    BitSet(state + GAME_STATE_FLAGS_OFFSET, bit, enabled);
}

/** @return Whether a script-visible game flag is set. */
AT("00006784") s32 GameStateTestFlagsAC(u32 bit)
{
    return BitTest(ORDERED_GAME_STATE_BASE + GAME_STATE_FLAGS_OFFSET, bit);
}

/** Enable every background attribute and reset its selection field. */
AT("000067DC") void GameStateInitializeAttributeFlags(void)
{
    CpuFill(ORDERED_GAME_STATE_BASE + GAME_STATE_ATTRIBUTE_FLAGS_OFFSET,
            GAME_STATE_ATTRIBUTE_FLAGS_SIZE, -1);
    GameStateSetField60E(0);
}

/** Enable or disable one background attribute. */
AT("0000680C") void GameStateSetAttributeFlag(s32 index, s32 enabled)
{
    BitSet(ORDERED_GAME_STATE_BASE + GAME_STATE_ATTRIBUTE_FLAGS_OFFSET,
           index, enabled);
}

/** @return Whether one background attribute is enabled. */
AT("00006834") s32 GameStateTestAttributeFlag(u32 bit)
{
    return BitTest(ORDERED_GAME_STATE_BASE + GAME_STATE_ATTRIBUTE_FLAGS_OFFSET,
                   bit);
}

/** Set an inclusive range of background-attribute enable bits. */
AT("00006858") void GameStateSetAttributeFlagRange(s32 first, s32 last,
                                                    s32 enabled)
{
    u8 *flags;
    s32 index;
    s32 value;

    if (enabled != 0)
        enabled = 1;
    value = enabled;
    flags = ORDERED_GAME_STATE_BASE + GAME_STATE_ATTRIBUTE_FLAGS_OFFSET;
    for (index = first; index <= last; index++)
        BitSet(flags, index, value);
}

/* The script ABI narrows these indices to signed 16 bits. */
#define GAME_STATE_FLAG_SET(address, name, bank) \
AT(address) void name(s32 index) \
{ \
    u8 *iwram; \
    u32 offset; \
    u8 *base; \
    s32 bit; \
    bit = index; \
    bit = (s16)bit; \
    iwram = gIwramBase; \
    offset = (u32)gMapGenerationRootOffset; \
    base = *(u8 **)(iwram + offset); \
    base += (bank); \
    BitSet(base, bit, 1); \
}

#define GAME_STATE_FLAG_TEST(address, name, bank) \
AT(address) s32 name(s32 index) \
{ \
    u8 *iwram; \
    u32 offset; \
    u8 *base; \
    s32 bit; \
    bit = index; \
    bit = (s16)bit; \
    iwram = gIwramBase; \
    offset = (u32)gMapGenerationRootOffset; \
    base = *(u8 **)(iwram + offset); \
    base += (bank); \
    return (s16)BitTest(base, bit); \
}

GAME_STATE_FLAG_SET("00056A34", GameStateSetFlag2730,
                    GAME_STATE_FLAGS_2730_OFFSET)
GAME_STATE_FLAG_TEST("00056A60", GameStateTestFlag2730,
                     GAME_STATE_FLAGS_2730_OFFSET)
GAME_STATE_FLAG_TEST("00056B2C", GameStateTestFlag26F8,
                     GAME_STATE_DECK_FLAGS_OFFSET)
