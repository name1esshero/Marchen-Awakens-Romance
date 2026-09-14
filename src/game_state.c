/* Typed access to fields in the engine's main runtime allocation. */
#include "game_state.h"

#include "rom_section.h"
extern u8 gIwramBase[];
extern u8 gMapGenerationRootOffset[];
#define GAME_STATE_BASE ({ \
    void **root = (void **)(gIwramBase + (u32)gMapGenerationRootOffset); \
    (u8 *)*root; \
})
#define FIELD(type,offset) (*(type *)(GAME_STATE_BASE + (offset)))

AT("00006C54") u32 GameStateGetField4240(void) { return FIELD(u32,0x4240); }
AT("00006C70") void GameStateSetField4240(u32 v) { FIELD(u32,0x4240)=v; }
AT("00006C8C") u32 GameStateGetField4256(void) { return FIELD(u16,0x4256); }
AT("00006CA8") void GameStateSetField4256(u32 v) { FIELD(u16,0x4256)=v; }
AT("00006CC4") s32 GameStateGetField4246(void) { return FIELD(s16,0x4246); }
AT("00006CE4") void GameStateSetField4246(s32 v) { FIELD(s16,0x4246)=v; }
AT("00006D00") s32 GameStateGetField4248(void) { return FIELD(s16,0x4248); }
AT("00006D20") void GameStateSetField4248(s32 v) { FIELD(s16,0x4248)=v; }
AT("00006D3C") void GameStateSetField4254(s32 v) { FIELD(u16,0x4254)=v; }
AT("00006D58") s32 GameStateGetField4254(void) { return FIELD(s16,0x4254); }
AT("00006D78") s32 GameStateGetField4244(void) { return FIELD(s8,0x4244); }
AT("00006D98") void GameStateSetField4244(s32 v) { FIELD(s8,0x4244)=v; }
AT("00006DB4") s32 GameStateGetField4245(void) { return FIELD(s8,0x4245); }
AT("00006DD4") void GameStateSetField4245(s32 v) { FIELD(s8,0x4245)=v; }

AT("00006E20") void GameStateGetField424C50(u32 *first,u32 *second)
{
 *first=FIELD(u32,0x424C);
 *second=FIELD(u32,0x4250);
}
AT("00006E50") u32 GameStateGetField424C(void) { return FIELD(u32,0x424C); }
AT("00006E6C") u32 GameStateGetField4250(void) { return FIELD(u32,0x4250); }

AT("00006ECC") void GameStateSetField4265(u32 i,s32 v)
{
 u8 *field=GAME_STATE_BASE+0x4265;
 field+=i;
 *(s8 *)field=v;
}
AT("00006EEC") s32 GameStateGetField4265(u32 i)
{
 u8 *field=GAME_STATE_BASE+0x4265;
 field+=i;
 return *(s8 *)field;
}
AT("00006F34") s32 GameStateGetField425A(void) { return FIELD(s8,0x425A); }
AT("00006F54") void GameStateSetField425B(s32 v) { FIELD(s8,0x425B)=v; }
AT("00006F70") s32 GameStateGetField425B(void) { return FIELD(s8,0x425B); }
AT("00006F90") void GameStateSetField4258(s32 v) { FIELD(u16,0x4258)=v; }
AT("00006FAC") s32 GameStateGetField4258(void) { return FIELD(s16,0x4258); }
AT("00006FCC") void GameStateSetField42BA(s32 v) { FIELD(u16,0x42BA)=v; }
AT("00006FE8") s32 GameStateGetField42BA(void) { return FIELD(s16,0x42BA); }
AT("00007008") void GameStateSetField42BC(s32 v) { FIELD(u16,0x42BC)=v; }
AT("00007024") s32 GameStateGetField42BC(void) { return FIELD(s16,0x42BC); }
AT("00007044") void GameStateSetField42C0(u32 v) { FIELD(u32,0x42C0)=v; }
AT("00007060") u32 GameStateGetField42C0(void) { return FIELD(u32,0x42C0); }
AT("0000707C") void GameStateSetField42C4(s32 v) { FIELD(u16,0x42C4)=v; }
AT("00007098") s32 GameStateGetField42C4(void) { return FIELD(s16,0x42C4); }
AT("000070B8") s32 GameStateGetField425C(u32 i)
{
 u8 *field=GAME_STATE_BASE+0x425C;
 field+=i;
 return *(s8 *)field;
}
AT("000070D8") void GameStateSetField425C(u32 i,s32 v)
{
 u8 *field=GAME_STATE_BASE+0x425C;
 field+=i;
 *(s8 *)field=v;
}
AT("000070F8") s32 GameStateGetField4269(void) { return FIELD(s8,0x4269); }
AT("00007118") void GameStateSetField4269(s32 v) { FIELD(s8,0x4269)=v; }
