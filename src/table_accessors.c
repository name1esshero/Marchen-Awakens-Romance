/* Typed views over fixed runtime and ROM tables. */
#include "gba/types.h"

#define AT(x) __attribute__((section(".rom." x)))
#define RUNTIME (*(u8 **)0x03004020)

#ifdef NONMATCHING
AT("00009508")
void *RuntimeGetPointerTableEntry(s32 index)
{
    return *(void **)(RUNTIME + 0xE3C + index * 4);
}
#endif

AT("0000AF88")
s32 RuntimeGetSignedByteE4B(void)
{
    return *(s8 *)(RUNTIME + 0xE4B);
}

AT("0000AFCC")
s32 RuntimeGetSignedByteE4C(void)
{
    return *(s8 *)(RUNTIME + 0xE4C);
}

#ifdef NONMATCHING
AT("00056F68")
void *GetItemDefinitionPointer(s32 itemId)
{
    return *(void **)(0x081B09E8 + (s16)itemId * 128 - 4);
}

AT("00056F7C")
u32 GetItemDefinitionByte124(s32 itemId)
{
    return *(u8 *)(0x081B096C + (s16)itemId * 128 + 124);
}

AT("00072C04")
void *GetBattleDefinition(s32 index)
{
    return ((void **)0x081C090C)[index];
}
#endif

AT("000868E8")
void *GetNewlibReentrancyState(void)
{
    return *(void **)0x08F2AFEC;
}
