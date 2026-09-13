/* Small engine entry points that supply fixed arguments or initialize a
 * compact state record before handing control to a shared implementation.
 */
#include "gba/types.h"

#define AT(x) __attribute__((section(".rom." x)))

extern void sub_08004BA4(void);
extern void RuntimeSetFieldEE8(s32 value);
extern void RuntimeSetFieldEE9(s32 value);
extern void CpuFill(void *destination, u32 size, u32 value);
extern void *sub_08070EA0(s32 index);
extern s32 sub_08080BC8(s32 arg0, s32 arg1, void *handler);

AT("00001630")
void InitializeMainRuntime(void)
{
    sub_08004BA4();
}
AT("00001630") const u8 InitializeMainRuntimeTail[2] = {0};

AT("00007678")
void ClearRuntimeStatusBytes(void)
{
    RuntimeSetFieldEE8(0);
    RuntimeSetFieldEE9(0);
}
AT("00007678") const u8 ClearRuntimeStatusBytesTail[2] = {0};

AT("00053294")
void Clear32ByteBlock(void *destination)
{
    CpuFill(destination, 32, 0);
}
AT("00053294") const u8 Clear32ByteBlockTail[2] = {0};

AT("00070730")
void ClearBattleRuntimeBuffer(void)
{
    CpuFill(sub_08070EA0(0), 1536, 0);
}
AT("00070730") const u8 ClearBattleRuntimeBufferTail[2] = {0};

AT("00079D94")
void CallRuntimeHandler(s32 arg0, s32 arg1)
{
    sub_08080BC8(arg0, arg1, *(void **)0x03005D20);
}

AT("0007F044")
void InitializePointerRecord(void **record, void *value)
{
    CpuFill(record, 8, 0);
    record[0] = value;
}
AT("0007F044") const u8 InitializePointerRecordTail[2] = {0};
