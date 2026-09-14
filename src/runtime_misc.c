/* Small helpers from the map renderer, script VM, and scene runtimes. */
#include "runtime_misc.h"

#include "rom_section.h"
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
#define GAME_STATE_BASE ({ \
    void **root=(void **)(gIwramBase+(u32)gMapGenerationRootOffset); \
    (u8 *)*root; \
})
/* This spelling keeps the IWRAM base and root-offset literals in the order
 * used by the original compiler for the following older helper family. */
#define ORDERED_GAME_STATE_BASE ({ \
    u8 *iwram=gIwramBase; \
    u32 offset=(u32)gMapGenerationRootOffset; \
    *(u8 **)(iwram+offset); \
})
extern s32 BitTest(const void *bits,u32 bit);
extern void CpuFill(void *destination,u32 size,u32 value);
extern void CpuCopy(const void *source,void *destination,u32 size);

AT("00070090") void *GameStateGetBuffer38C0(void)
{
 return GAME_STATE_BASE+0x38C0;
}

AT("0007E964") void VmAddToField220(u32 value)
{
 u8 *vm=*(u8 **)0x0300611C;
 u32 *field=(u32 *)(*(u8 **)(vm+0x0C)+0x220);
 *field+=value;
}

AT("0007F274") u32 VmGetField10(void)
{
 u8 *vm=*(u8 **)0x0300611C;
 return *(u16 *)(*(u8 **)(vm+0x0C)+0x10);
}

AT("0007F284") u32 VmGetField12(void)
{
 u8 *vm=*(u8 **)0x0300611C;
 return *(u16 *)(*(u8 **)(vm+0x0C)+0x12);
}

AT("00080600") void *RuntimeGetPointer6120Field800(void)
{
 return *(void **)(*(u8 **)0x03006120+0x800);
}

AT("0002ADC8") void InputRepeatInit(struct InputRepeatState *state,u32 mask)
{
 state->counter=0;
 state->previous=0xFFFF;
 state->mask=mask;
 state->active=0;
 state->initialDelay=12;
 state->repeatDelay=24;
}

AT("0006C7A0") void MapObjectResetMotion(struct MapObjectMotion *motion)
{
 motion->field04=0;
 motion->field14=0;
 motion->field12=0;
 motion->field16=0;
}

AT("0000F0CC") u8 *GameStateGetRecord1190(u32 index)
{
 return GAME_STATE_BASE+0x1190+index*20;
}

AT("0000503C") u32 RuntimeReturnZero(void) { return 0; }
AT("00011674") const struct BattleCharacterDefinition *RuntimeGetBattleCharacterDefinition(u32 index)
{
 return &gBattleCharacterDefinitions[index];
}
AT("000804EC") u8 *RuntimeGetBlock6120(u32 slot,u32 group)
{
 u32 slotBits=slot<<24;
 u8 **root=(u8 **)0x03006120;
 group<<=10;
 {
  u8 *base=*root;
  base+=group;
  slotBits>>=19;
  return base+slotBits;
 }
}
extern char *strcpy(char *destination,const char *source);
AT("000069F8") void GameStateCopyString12F4(char *destination)
{
 strcpy(destination,(const char *)(GAME_STATE_BASE+0x12F4));
}
AT("000106C8") void *GameStateGetRecord0B90(u32 index)
{
 return GAME_STATE_BASE+0x0B90+index*40;
}

AT("00006784") s32 GameStateTestFlagsAC(u32 bit)
{
 return BitTest(ORDERED_GAME_STATE_BASE+0xAC,bit);
}

AT("00006834") s32 GameStateTestFlags12C(u32 bit)
{
 return BitTest(ORDERED_GAME_STATE_BASE+0x12C,bit);
}

/* Copy a caller-owned string into the fixed game-state text buffer. */
AT("00006A1C") void GameStateSetString12F4(const char *source)
{
 strcpy((char *)(ORDERED_GAME_STATE_BASE+0x12F4),source);
}

AT("0000F96C") void GameStateClearBlock413C(void)
{
 CpuFill(ORDERED_GAME_STATE_BASE+0x413C,256,0);
}

AT("00057218") void GameStateCopyMapBuffer(void)
{
 u8 *state=ORDERED_GAME_STATE_BASE;
 CpuCopy(state+0x31D0,state+0x33D0,512);
}

extern u8 *RuntimeGetActorRecord(u32 actor,u32 part);
AT("00006A40") void GameStateSaveActorRecord0(void)
{
 u8 *record=RuntimeGetActorRecord(0,0);
 CpuCopy(record,ORDERED_GAME_STATE_BASE+0x1244,168);
}
AT("00006A6C") void GameStateLoadActorRecord0(void)
{
 u8 *saved=ORDERED_GAME_STATE_BASE+0x1244;
 u8 *record=RuntimeGetActorRecord(0,0);
 CpuCopy(saved,record,168);
}

AT("0000F0A0") void GameStateClearRecord1190IfZero(u32 value)
{
 if (value==0)
  CpuFill(ORDERED_GAME_STATE_BASE+0x1190,180,0);
}
AT("00011438") void GameStateClearRecord1090IfZero(u32 value)
{
 if (value==0)
  CpuFill(ORDERED_GAME_STATE_BASE+0x1090,256,0);
}

AT("000805D0") void SpriteRuntimeSetAllFlags800(u32 enabled)
{
 u8 *flags;
 if (enabled) {
  flags=*(u8 **)0x03006120+0x800;
  enabled=-1;
 } else {
  flags=*(u8 **)0x03006120+0x800;
 }
 *(u32 *)flags=enabled;
}
