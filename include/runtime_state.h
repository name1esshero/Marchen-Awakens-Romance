#ifndef RUNTIME_STATE_H
#define RUNTIME_STATE_H
#include "gba/types.h"

/* Fixed IWRAM pointer to the secondary runtime overlay: a large state blob
 * used by battle tasks, encounter tasks, the object-command task, and the
 * per-actor buffer table. Backed by asm/iwram_symbols.s's
 * ".set gSecondaryRuntime, 0x03004020", so this is a real global whose
 * address is that fixed IWRAM slot, not a macro over a raw address. */
extern u8 *gSecondaryRuntime;

/* Fixed IWRAM pointer to the primary runtime allocation used by the link and
 * early-runtime accessors. */
extern u8 *gPrimaryRuntime;

#define RUNTIME_HISTORY_ENTRY_COUNT 10
#define RUNTIME_HISTORY_ENTRY_WORD_COUNT 6
#define RUNTIME_HISTORY_PADDING_SIZE 10

/** A 24-byte snapshot stored in the primary runtime's circular history. */
struct RuntimeHistoryEntry
{
    u32 words[RUNTIME_HISTORY_ENTRY_WORD_COUNT];
};

/** Ten runtime snapshots followed by the signed current/previous cursors. */
struct RuntimeHistory
{
    struct RuntimeHistoryEntry entries[RUNTIME_HISTORY_ENTRY_COUNT];
    u8 unknownF0[RUNTIME_HISTORY_PADDING_SIZE];
    s8 currentIndex;
    s8 previousIndex;
};

s32 RuntimeGetLinkActivityState(void);
void *RuntimeGetOptionalField130(void);
void RuntimeInitialize(u8 *state);
void RuntimeClear(void);
void RuntimeReleaseField17C(void);
s32 RuntimeHistoryPush(const struct RuntimeHistoryEntry *entry,
                       struct RuntimeHistory *history);
void RuntimeStart(void);
void RuntimeStop(void);
void *RuntimeGetCurrentRecord14C(void);
void RuntimeAdvanceWord4(void);
void *RuntimeGetRecord17C(s16 index);
void RuntimeSetModeE4B(s32 value);
s32 RuntimeGetSignedByteE4B(void);
s32 RuntimeGetSignedByteE4C(void);

#endif /* RUNTIME_STATE_H */
