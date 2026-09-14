/* Mount names are uppercased on assignment. Unmount only clears the active
 * flag; callers retain ownership of archive data. */
#include "nfp.h"
#include "rom_section.h"
extern char *strcpy(char *,const char *);
extern char *strupr(char *);
AT("0007AA48")
const char *NfpGetMountName(s32 handle)
{
 if(NfpMountIsActive(handle))
  return gNfpState->mounts[handle].name;
 else return 0;
}
AT("0007AA74")
void NfpSetMountName(s32 handle,const char *name)
{
 strcpy(gNfpState->mounts[handle].name,name);
 strupr(gNfpState->mounts[handle].name);
}
AT("0007AB70")
void NfpUnmount(s32 handle)
{
 NfpSetMountActive(handle,0);
}
AT("0007ABBC")
s32 NfpCountMounted(void)
{
 s32 count=0,i;
 for(i=0;i<gNfpState->mount_count;i++)
  if(NfpMountIsActive(i)) count++;
 return count;
}
