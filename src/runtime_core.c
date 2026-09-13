/* Lifecycle and record-selection helpers for the main runtime allocation. */
#include "gba/types.h"
#include "runtime_accessors.h"

#define AT(x) __attribute__((section(".rom." x)))
#define RUNTIME_ROOT (*(u8 **)0x0300401C)

extern void CpuFill(void *destination, u32 size, u32 value);
extern void sub_080046B0(void);
extern void sub_08004758(void);
extern void sub_08004B08(void);
extern void sub_080048C0(void *state);
extern void sub_080047AC(void);
extern void sub_08004840(void);
extern s32 sub_08004DA8(void);
extern u32 sub_08004CDC(void);

/* A secondary allocation is optional during early startup and teardown. */
AT("00004E04") void *RuntimeGetOptionalField130(void)
{
    u8 *state = *(u8 **)0x03004014;
    void *result;
    if (state != 0)
        result = *(void **)(state + 0x130);
    else
        result = 0;
    return result;
}
AT("00004E04") const u8 RuntimeGetOptionalField130Tail[2] = {0, 0};

AT("00004E24") void RuntimeInitialize(u8 *state)
{
    u8 **root = (u8 **)0x0300401C;
    *root = state;
    CpuFill(state, 0x3B0, 0);
    state = *root;
    *(u8 **)(state + 0x148) = state + 8;
    sub_080046B0();
}

AT("00004E50") void RuntimeClear(void)
{
    sub_08004758();
    CpuFill(RUNTIME_ROOT, 0x3B0, 0);
}

AT("00004E6C") void RuntimeReleaseField17C(void)
{
    sub_08004B08();
    sub_080048C0(RUNTIME_ROOT + 0x17C);
}

#ifdef NONMATCHING
AT("00004EDC") void RuntimeStart(void)
{
    u8 *state;
    u32 stopped = 0;
    u32 running = 1;
    sub_080047AC();
    state = RUNTIME_ROOT;
    *(u16 *)state = running;
    *(u32 *)(state + 4) = stopped;
}
#endif

AT("00004EF8") void RuntimeStop(void)
{
    sub_08004840();
    *(u16 *)RUNTIME_ROOT = 0;
}

AT("00004F94") void *RuntimeGetCurrentRecord14C(void)
{
    u8 *state = RUNTIME_ROOT;
    u32 index = *(u16 *)(state + 0x1AC);
    return state + 0x14C + index * 24;
}

AT("00004FB4") void RuntimeAdvanceWord4(void)
{
    u32 *state = (u32 *)RUNTIME_ROOT;
    state[1]++;
}

#ifdef NONMATCHING
AT("00004FF0") void *RuntimeGetRecord17C(s32 index)
{
    s32 offset = (s16)index * 24;
    return RUNTIME_ROOT + 0x17C + offset;
}
#endif

AT("00005040") void RuntimeNoOp(void) {}
AT("00005040") const u8 RuntimeNoOpTail[2] = {0, 0};

AT("00005044") u32 RuntimeGetLinkPlayerIfActive(void)
{
    u32 player;
    if ((s8)sub_08004DA8() <= 1)
        player = 0;
    else
        player = (u8)SioGetPlayerId();
    return player;
}
AT("00005044") const u8 RuntimeGetLinkPlayerIfActiveTail[2] = {0, 0};

AT("00005064") u32 RuntimeGetByte4014U8(void)
{
    return (u8)RuntimeGetByte4014();
}
AT("00005064") const u8 RuntimeGetByte4014U8Tail[2] = {0, 0};

AT("00005074") u32 RuntimeGetByte4CDCU8(void)
{
    return (u8)sub_08004CDC();
}
AT("00005074") const u8 RuntimeGetByte4CDCU8Tail[2] = {0, 0};
