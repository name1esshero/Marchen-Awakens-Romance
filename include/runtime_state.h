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

void *RuntimeGetOptionalField130(void);
void RuntimeInitialize(u8 *state);
void RuntimeClear(void);
void RuntimeReleaseField17C(void);
void RuntimeStart(void);
void RuntimeStop(void);
void *RuntimeGetCurrentRecord14C(void);
void RuntimeAdvanceWord4(void);
void *RuntimeGetRecord17C(s16 index);

#endif /* RUNTIME_STATE_H */
