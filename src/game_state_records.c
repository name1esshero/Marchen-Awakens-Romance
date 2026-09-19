/* Accessors for the 84-byte records at map-generation state +0x35E0 and for
 * the small s16 table at +0x3894.  sub_08055F4C resolves a record by its
 * signed 16-bit id and is still assembly, so it keeps its placeholder name.
 * The stored counters saturate at 999, which the original code expresses as
 * "greater than 998" on the signed 16-bit value. */
#include "gba/types.h"

#include "game_state.h"
#include "game_tables.h"
#include "rom_section.h"

extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
extern u8 *sub_08055F4C(s32 id);
extern u8 *sub_08056E3C(s32 a,s32 b,s32 c);
extern void CpuCopy(const void *source,void *destination,u32 size);
extern s32 sub_08056918(s32 id);
extern s32 GameStateGetEntry2768(s32 id);
extern s32 sub_080568D8(s32 entry, s32 deckId);

#define GAME_STATE_BASE ({ \
    void **root=(void **)(gIwramBase+(u32)gMapGenerationRootOffset); \
    (u8 *)*root; \
})

#define RECORD_FIELD_MAX 998
#define RECORD_FIELD_LIMIT 999

#define GAME_STATE_RESOURCE_COUNTER_OFFSET 0x38BC
#define GAME_STATE_RESOURCE_COUNTER_MAX 999999

struct GameStateResourceCounter
{
 u8 padding[GAME_STATE_RESOURCE_COUNTER_OFFSET];
 u32 value;
};

/* The runtime root is read again after updating the counter.  The union view
 * expresses that the slot and the state it points at may alias, preserving
 * the reload found in the original code without volatile or an optimizer
 * barrier. */
union GameStateRootSlot
{
 struct GameStateResourceCounter *state;
 u32 raw;
};

/** Read the six-digit resource counter, repairing an out-of-range value. */
AT("00056290") u32 GameStateGetResourceCounter(void)
{
    u32 offset;
    u8 *state;
    u8 **root;
    u32 *counter;
    u8 *iwram;
    u32 rootOffset;

    iwram = gIwramBase;
    rootOffset = GAME_STATE_ROOT_IWRAM_OFFSET;
    root = (u8 **)(iwram + rootOffset);
    state = *root;
    offset = GAME_STATE_RESOURCE_COUNTER_OFFSET;
    counter = (u32 *)(state + offset);
    if (*counter > GAME_STATE_RESOURCE_COUNTER_MAX - 1)
        *counter = GAME_STATE_RESOURCE_COUNTER_MAX;
    return *(u32 *)(*root + offset);
}

/* Record field 4 is the ceiling the field-2 accumulator clamps against;
 * field 6 is a second independent counter with the same 999 ceiling. */
#define RECORD_SET_CLAMPED(address,name,field) \
 AT(address) void name(s32 id,s32 value) \
 { \
  u8 *record; \
  s32 v; \
  u16 stored; \
  v=value; \
  id=(s16)id; \
  v=(s16)v; \
  record=sub_08055F4C(id); \
  stored=v; \
  *(u16 *)(record+(field))=stored; \
  if ((s16)stored>RECORD_FIELD_MAX) \
  { \
   stored=RECORD_FIELD_LIMIT; \
   *(u16 *)(record+(field))=stored; \
  } \
 }

RECORD_SET_CLAMPED("00055FE8",GameStateRecordSetField4,4)
/* Nothing in the decoded ROM reaches this entry point; its signature is the
 * one proved by the byte-identical sibling above. */
RECORD_SET_CLAMPED("0005601C",GameStateRecordSetField6,6)

/** Game state record add field6 using the recovered runtime layout. */
AT("00056050") void GameStateRecordAddField6(s32 id,s32 value)
{
 u8 *record;
 s32 v;
 u16 stored;
 v=value;
 id=(s16)id;
 v=(s16)v;
 record=sub_08055F4C(id);
 v+=*(u16 *)(record+6);
 *(u16 *)(record+6)=v;
 if ((s16)v>RECORD_FIELD_MAX)
 {
  stored=RECORD_FIELD_LIMIT;
  *(u16 *)(record+6)=stored;
 }
}

/** Add to the game-state resource counter and saturate it at six digits. */
AT("000562C8") void GameStateAddResourceCounter(u32 value)
{
 union GameStateRootSlot *root;
 struct GameStateResourceCounter *state;

 root=(union GameStateRootSlot *)(gIwramBase+(u32)gMapGenerationRootOffset);
 state=root->state;
 state->value+=value;
 if (root->state->value>GAME_STATE_RESOURCE_COUNTER_MAX-1)
  root->state->value=GAME_STATE_RESOURCE_COUNTER_MAX;
}

/* state+0x3894 holds s16 entries in 12-byte rows of six. */
#define TABLE_3894_ROW_SIZE 12
#define TABLE_3894_GROUP 3

/** Game state set entry3894 using the recovered runtime layout. */
AT("000560C4") void GameStateSetEntry3894(s32 row,s32 group,s32 slot,s32 value)
{
 u8 *iwram;
 u32 offset;
 u8 *base;
 s32 index;
 iwram=gIwramBase;
 offset=(u32)gMapGenerationRootOffset;
 base=*(u8 **)(iwram+offset);
 index=(group*TABLE_3894_GROUP+slot)*2;
 index+=row*TABLE_3894_ROW_SIZE;
 base+=0x3894;
 base+=index;
 *(u16 *)base=value;
}

/** Game state get entry3894 using the recovered runtime layout. */
AT("000560F8") s32 GameStateGetEntry3894(s32 row,s32 group,s32 slot)
{
 u8 *iwram;
 u32 offset;
 u8 *base;
 s32 index;
 iwram=gIwramBase;
 offset=(u32)gMapGenerationRootOffset;
 base=*(u8 **)(iwram+offset);
 index=(group*TABLE_3894_GROUP+slot)*2;
 index+=row*TABLE_3894_ROW_SIZE;
 base+=0x3894;
 base+=index;
 return *(s16 *)base;
}

/** Copies one 40-byte entry out of the record table reached by sub_08056E3C. */
AT("00056CD0") void GameStateCopyRecord(s32 a,s32 b,void *destination)
{
 u8 *source;
 s32 x;
 s32 y;
 x=a;
 y=b;
 x=(s16)x;
 y=(s16)y;
 source=sub_08056E3C(0,x,y);
 CpuCopy(source,destination,40);
}

/** Sums the two per-id contributions and saturates at 99. */
AT("00056984") s32 GameStateGetEntry2768Total(s32 id)
{
 s32 v;
 s32 total;
 v=id;
 v=(s16)v;
 total=(s16)sub_08056918(v);
 total=(s16)(total+GameStateGetEntry2768(v));
 if (total>98)
  total=99;
 return total;
}

#define PMB_DECK_ENTRY_COUNT_LIMIT 98
#define PMB_DECK_ENTRY_COUNT_OVERFLOW 99
#define FIXED_POINT_ONE (1 << 16)

/**
 * @brief Count one PMB entry across the saved total and all default decks.
 *
 * A result above 98 is represented by the sentinel 99. The fixed-point loop
 * counter is the same 16.16 counter convention used by adjacent game-state
 * routines; the deck pointer itself advances by one signed halfword.
 */
AT("000569B0") s32 CountPmbDeckEntryCopies(s32 id)
{
 s32 entry;
 s32 total;
 s32 fixedIndex;
 const s16 *deck;
 s32 index;

 entry=(s16)id;
 total=(s16)GameStateGetEntry2768(entry);
 fixedIndex=FIXED_POINT_ONE;
 deck=gBattlePartyDefaults;
 do
 {
  total=(s16)(total+sub_080568D8(entry,*deck));
  index=fixedIndex;
  fixedIndex+=FIXED_POINT_ONE;
  deck++;
 } while ((index>>16)<=(s32)ARRAY_COUNT(gBattlePartyDefaults)-1);
 if (total>PMB_DECK_ENTRY_COUNT_LIMIT)
  total=PMB_DECK_ENTRY_COUNT_OVERFLOW;
 return total;
}

/* state+0x423C points at the actor/part array the runtime also reaches
 * through gSecondaryRuntime; the strides are the same 1672/104 pair. */
#define ACTOR_RECORD_SIZE 1672
#define PART_RECORD_SIZE 104

/** Game state get part field64 c using the recovered runtime layout. */
AT("00056DFC") s32 GameStateGetPartField64C(s16 actorArg,s16 partArg)
{
 u8 *base;
 s32 actor;
 s32 part;
 actor=actorArg;
 part=partArg;
 base=*(u8 **)(GAME_STATE_BASE+0x423C);
 part*=PART_RECORD_SIZE;
 actor*=ACTOR_RECORD_SIZE;
 part+=actor;
 base+=part;
 base+=0x64C;
 return *(s16 *)base;
}
