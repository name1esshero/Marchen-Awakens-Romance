#ifndef RUNTIME_OBJECTS_H
#define RUNTIME_OBJECTS_H
#include "gba/types.h"

void *RuntimeGetPointerEA0(void);
void RuntimeSetPointerEA0(void *value);
void RuntimeSetFieldsE48ToE4A(s32 first,s32 second,s32 third);
void RuntimeSetFieldE48(s32 value);
s32 RuntimeGetFieldE48(void);
void RuntimeSetFieldE49(s32 value);
s32 RuntimeGetFieldE49(void);
void RuntimeSetFieldE4A(s32 value);
s32 RuntimeGetFieldE4A(void);
void *RuntimeGetActorRecord(u32 actor,u32 part);

s32 RuntimeObjectGetField00(u32 group,u32 slot);
s32 RuntimeObjectGetField01(u32 group,u32 slot);
s32 RuntimeObjectGetField02(u32 group,u32 slot);
s32 RuntimeObjectGetField03(u32 group,u32 slot);
s32 RuntimeObjectGetField04(u32 group,u32 slot);
s32 RuntimeObjectGetField05(u32 group,u32 slot);
s32 RuntimeObjectGetField0A(u32 group,u32 slot);
s32 RuntimeObjectGetField09(u32 group,u32 slot);
void RuntimeObjectSetFlag00(u32 group,u32 slot,s32 enabled);
void RuntimeObjectSetFlag01(u32 group,u32 slot,s32 enabled);
void RuntimeObjectSetFlag02(u32 group,u32 slot,s32 enabled);
void RuntimeObjectSetFlag03(u32 group,u32 slot,s32 enabled);
void RuntimeObjectSetFlag04(u32 group,u32 slot,s32 enabled);
void RuntimeObjectSetFlag05(u32 group,u32 slot,s32 enabled);
void RuntimeObjectSetFlag0A(u32 group,u32 slot,s32 enabled);
void RuntimeObjectSetFlag09(u32 group,u32 slot,s32 enabled);
s32 RuntimeObjectGetField48(u32 group,u32 slot);
void RuntimeObjectSetField48(u32 group,u32 slot,s32 value);
void RuntimeObjectSetFixed18(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField1A(u32 group,u32 slot);
void RuntimeObjectSetFixed1C(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField1E(u32 group,u32 slot);
void RuntimeObjectSetFixed20(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField22(u32 group,u32 slot);
void RuntimeObjectSetFixed24(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField26(u32 group,u32 slot);
void RuntimeObjectSetWord18(u32 group,u32 slot,u32 value);
u32 RuntimeObjectGetWord18(u32 group,u32 slot);
void RuntimeObjectSetWord1C(u32 group,u32 slot,u32 value);
u32 RuntimeObjectGetWord1C(u32 group,u32 slot);
void RuntimeObjectSetWord20(u32 group,u32 slot,u32 value);
u32 RuntimeObjectGetWord20(u32 group,u32 slot);
void RuntimeObjectSetWord24(u32 group,u32 slot,u32 value);
u32 RuntimeObjectGetWord24(u32 group,u32 slot);
void RuntimeObjectSetField36(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField36(u32 group,u32 slot);
s32 RuntimeObjectGetField3A(u32 group,u32 slot);
void RuntimeObjectSetField3A(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField3C(u32 group,u32 slot);
void RuntimeObjectSetField3C(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField3E(u32 group,u32 slot);
void RuntimeObjectSetField3E(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField40(u32 group,u32 slot);
void RuntimeObjectSetField40(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField39(u32 group,u32 slot);
void RuntimeObjectSetField39(u32 group,u32 slot,s32 value);
void RuntimeObjectSetField4A(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField4A(u32 group,u32 slot);
void RuntimeObjectSetField4C(u32 group,u32 slot,s32 value);
s32 RuntimeObjectGetField4C(u32 group,u32 slot);
#endif
