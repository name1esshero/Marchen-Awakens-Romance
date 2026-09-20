/* Lifecycle and record-selection helpers for the main runtime allocation. */
#include "gba/types.h"
#include "runtime_accessors.h"
#include "runtime_state.h"

#include "rom_section.h"
#define RUNTIME_ROOT gRuntimeState

#define RUNTIME_RECORD_A_OFFSET 0x14C
#define RUNTIME_RECORD_B_OFFSET 0x164
#define RUNTIME_RECEIVE_RECORDS_OFFSET 0x17C
#define RUNTIME_RECORD_SELECTOR_OFFSET 0x1AC
#define RUNTIME_TRANSFER_RECORD_COUNT 2
#define RUNTIME_GROUP_STRIDE 0x100
#define RUNTIME_GROUP_RECORDS_OFFSET 0x1B0
#define RUNTIME_GROUP_SELECTOR_OFFSET 0x2AA

struct RuntimeHeader
{
    u16 isRunning;
    u16 padding02;
    u32 frameCounter;
};

extern void CpuFill(void *destination, u32 size, u32 value);
extern void sub_080046B0(void);
extern void sub_08004758(void);
extern void sub_08004B08(void);
extern void sub_080048C0(void *state);
extern void sub_080047AC(void);
extern void sub_08004840(void);
extern u32 sub_08004CDC(void);

#define LINK_STATE_FIELD_OFFSET 0x130
#define LINK_STATE_CONNECTION_MASK 0x180
#define LINK_STATE_ACTIVITY_MASK 0xE

/**
 * @brief Classify the current link-play activity state.
 * @return 0 when disconnected as player zero, 2 when this player's activity
 * bit is set, and 1 for the remaining connected or idle states.
 */
AT("00004DA8") s32 RuntimeGetLinkActivityState(void)
{
    u8 **root;
    u8 *runtime;
    u32 offset;

    root = &gLinkRuntime;
    runtime = *root;
    offset = LINK_STATE_FIELD_OFFSET;

    if ((*(u32 *)(runtime + offset) & LINK_STATE_CONNECTION_MASK) == 0)
    {
        if ((s8)SioGetPlayerId() == 0)
            goto inactive;
    }

    if ((*(u32 *)(*root + offset) & LINK_STATE_ACTIVITY_MASK) == 0)
        goto active;

    if (*(u32 *)(*root + offset) & (1 << (u8)SioGetPlayerId()))
        return 2;

active:
    return 1;

inactive:
    return 0;
}

/** A secondary allocation is optional during early startup and teardown. */
AT("00004E04") void *RuntimeGetOptionalField130(void)
{
    u8 *state = gLinkRuntime;
    void *result;
    if (state != 0)
        result = *(void **)(state + 0x130);
    else
        result = 0;
    return result;
}
AT("00004E04") const u8 RuntimeGetOptionalField130Tail[2] = {0, 0};

/** One of the 24-byte records based at +0x17C in the main allocation. The
 * index arrives narrowed to 16 bits by the callers' ABI. */
AT("00004FF0") void *RuntimeGetRecord17C(s16 index)
{
    s32 narrowed = index;
    u8 **root;
    u32 offset;

    root = &gRuntimeState;
    offset = narrowed * 24 + 380;
    return *root + offset;
}

/** Return the selected 24-byte record in one 0x100-byte runtime group. */
AT("0000500C") void *RuntimeGetSelectedGroupRecord(s32 group)
{
    s32 groupOffset = group;
    u8 **root;
    s32 recordOffset;
    u8 *state;
    u8 *records;
    u8 *selector;
    s32 record;

    groupOffset <<= 16;
    root = &gRuntimeState;
    groupOffset >>= 8;
    recordOffset = groupOffset + RUNTIME_GROUP_RECORDS_OFFSET;
    state = *root;
    records = state + recordOffset;
    selector = state + groupOffset;
    record = *(s8 *)(selector + RUNTIME_GROUP_SELECTOR_OFFSET);

    return records + record * sizeof(struct RuntimeHistoryEntry);
}

/** Install the main runtime allocation, zero its 0x3B0-byte block, wire up
 * its +0x148 pointer, and run the shared startup routine.
 * @param state Freshly allocated runtime block to install as the root.
 * @return Nothing. */
AT("00004E24") void RuntimeInitialize(u8 *state)
{
    u8 **root = &gRuntimeState;
    *root = state;
    CpuFill(state, 0x3B0, 0);
    state = *root;
    *(u8 **)(state + 0x148) = state + 8;
    sub_080046B0();
}

/** Run the shared teardown routine, then zero the whole 0x3B0-byte runtime
 * block in place (the allocation itself is kept, unlike RuntimeInitialize()
 * which installs a new one). */
AT("00004E50") void RuntimeClear(void)
{
    sub_08004758();
    CpuFill(RUNTIME_ROOT, 0x3B0, 0);
}

/** Release the +0x17C record table's resources. */
AT("00004E6C") void RuntimeReleaseField17C(void)
{
    sub_08004B08();
    sub_080048C0(RUNTIME_ROOT + 0x17C);
}

/**
 * @brief Advance a ten-entry circular history and copy one snapshot into it.
 *
 * The entry is copied one word at a time. Keeping the two cursors explicit
 * documents the fixed 24-byte record ABI and reproduces the original copy
 * without relying on an untyped block operation.
 *
 * @param entry Snapshot to append.
 * @param history Circular history to update.
 * @return TRUE.
 */
AT("00004E88")
s32 RuntimeHistoryPush(const struct RuntimeHistoryEntry *entry,
                       struct RuntimeHistory *history)
{
    const u32 *source;
    u32 *destination;

    history->previousIndex = history->currentIndex;
    history->currentIndex++;
    history->currentIndex %= RUNTIME_HISTORY_ENTRY_COUNT;

    source = entry->words;
    destination = history->entries[history->currentIndex].words;
    *destination++ = *source++;
    *destination++ = *source++;
    *destination++ = *source++;
    *destination++ = *source++;
    *destination++ = *source++;
    *destination = *source;
    return TRUE;
}
AT("00004E88") const u8 RuntimeHistoryPushTail[2] = {0, 0};

/** Run the shared startup routine, mark the runtime active, and reset its
 * frame counter. */
AT("00004EDC") void RuntimeStart(void)
{
    struct RuntimeHeader *state;

    sub_080047AC();
    state = (struct RuntimeHeader *)RUNTIME_ROOT;
    state->isRunning = TRUE;
    state->frameCounter = 0;
}

/** Run the shared stop routine and clear the runtime's running flag. */
AT("00004EF8") void RuntimeStop(void)
{
    sub_08004840();
    *(u16 *)RUNTIME_ROOT = 0;
}

/** Clear both receive records and both send records used by link transfer. */
AT("00004F10") void RuntimeClearTransferRecords(void)
{
    u8 **root = &gRuntimeState;
    s32 fixedIndex = 1 << 16;
    u32 offset = RUNTIME_RECEIVE_RECORDS_OFFSET;
    s32 index;

    do
    {
        CpuFill(*root + offset, sizeof(struct RuntimeHistoryEntry), 0);
        index = fixedIndex;
        fixedIndex += 1 << 16;
        offset += sizeof(struct RuntimeHistoryEntry);
    } while ((index >> 16) <= RUNTIME_TRANSFER_RECORD_COUNT - 1);

    CpuFill(RUNTIME_ROOT + RUNTIME_RECORD_A_OFFSET,
            sizeof(struct RuntimeHistoryEntry), 0);
    CpuFill(RUNTIME_ROOT + RUNTIME_RECORD_B_OFFSET,
            sizeof(struct RuntimeHistoryEntry), 0);
}

/** Clear the two records used as outgoing link-transfer payloads. */
AT("00004F64") void RuntimeClearSendRecords(void)
{
    CpuFill(RUNTIME_ROOT + RUNTIME_RECORD_A_OFFSET,
            sizeof(struct RuntimeHistoryEntry), 0);
    CpuFill(RUNTIME_ROOT + RUNTIME_RECORD_B_OFFSET,
            sizeof(struct RuntimeHistoryEntry), 0);
}

/** @return The 24-byte 0x14C record selected by the index stored at +0x1AC. */
AT("00004F94") void *RuntimeGetCurrentRecord14C(void)
{
    u8 *state = RUNTIME_ROOT;
    u32 index = *(u16 *)(state + 0x1AC);
    return state + 0x14C + index * 24;
}

/** Increment the runtime's second 32-bit word (offset +4). Likely a frame
 * or tick counter; exact meaning not yet recovered. */
AT("00004FB4") void RuntimeAdvanceWord4(void)
{
    u32 *state = (u32 *)RUNTIME_ROOT;
    state[1]++;
}

/** Build a packet from the 24-byte record that is not currently selected. */
AT("00004FC4") void RuntimeBuildInactiveRecordPacket(void)
{
    u8 *state = RUNTIME_ROOT;
    u16 selector = *(u16 *)(state + RUNTIME_RECORD_SELECTOR_OFFSET);
    struct RuntimeHistoryEntry *entry =
        (struct RuntimeHistoryEntry *)(state + RUNTIME_RECORD_A_OFFSET);

    if (selector == 0)
        entry = (struct RuntimeHistoryEntry *)(state + RUNTIME_RECORD_B_OFFSET);

    LinkBuildSendPacket(entry);
}

/** Does nothing; kept as a callable no-op handler. */
AT("00005040") void RuntimeNoOp(void) {}
AT("00005040") const u8 RuntimeNoOpTail[2] = {0, 0};

/** @return This console's link-play player id if a link session with more
 * than one player is active, otherwise 0. */
AT("00005044") u32 RuntimeGetLinkPlayerIfActive(void)
{
    u32 player;
    if ((s8)RuntimeGetLinkActivityState() <= 1)
        player = 0;
    else
        player = (u8)SioGetPlayerId();
    return player;
}
AT("00005044") const u8 RuntimeGetLinkPlayerIfActiveTail[2] = {0, 0};

/** Narrow-return wrapper around RuntimeGetByte4014(). */
AT("00005064") u32 RuntimeGetByte4014U8(void)
{
    return (u8)RuntimeGetByte4014();
}
AT("00005064") const u8 RuntimeGetByte4014U8Tail[2] = {0, 0};

/** Narrow-return wrapper around sub_08004CDC(). */
AT("00005074") u32 RuntimeGetByte4CDCU8(void)
{
    return (u8)sub_08004CDC();
}
AT("00005074") const u8 RuntimeGetByte4CDCU8Tail[2] = {0, 0};

extern u32 RuntimeReturnZero(void);
extern void CpuCopy(void *destination, const void *source, u32 size);

/* Bytes of the current 0x14C record that the caller-supplied header owns. */
#define RUNTIME_RECORD_14C_HEADER_SIZE 8

/** Overwrite the header of the currently selected 0x14C record. */
AT("0006C6F8") void RuntimeStoreCurrentRecord14C(const void *source)
{
    void *record = RuntimeGetCurrentRecord14C();

    if ((u8)RuntimeReturnZero() == 0)
        CpuCopy(record, source, RUNTIME_RECORD_14C_HEADER_SIZE);
}
