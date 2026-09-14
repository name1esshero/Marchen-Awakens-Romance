#ifndef RUNTIME_STATE_H
#define RUNTIME_STATE_H
#include "gba/types.h"

/* Fixed IWRAM pointer to the secondary runtime overlay: a large state blob
 * used by battle tasks, encounter tasks, the object-command task, and the
 * per-actor buffer table. Backed by asm/iwram_symbols.s's
 * ".set gSecondaryRuntime, 0x03004020", so this is a real global whose
 * address is that fixed IWRAM slot, not a macro over a raw address. */
extern u8 *gSecondaryRuntime;

#endif /* RUNTIME_STATE_H */
