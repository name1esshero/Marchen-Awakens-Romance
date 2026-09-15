/* Mount names are uppercased on assignment. Unmount only clears the active
 * flag; callers retain ownership of archive data. */
#include "nfp.h"
#include "rom_section.h"
extern char *strcpy(char *,const char *);
extern char *strupr(char *);
/** @return The active mount's name, or NULL if handle is not mounted. */
AT("0007AA48")
const char *NfpGetMountName(s32 handle)
{
 if(NfpMountIsActive(handle))
  return gNfpState->mounts[handle].name;
 else return 0;
}
/** Copy name into a mount slot and uppercase it in place. Does not itself
 * mark the mount active. */
AT("0007AA74")
void NfpSetMountName(s32 handle,const char *name)
{
 strcpy(gNfpState->mounts[handle].name,name);
 strupr(gNfpState->mounts[handle].name);
}
/** Clear a mount's active flag. The archive data itself is left untouched;
 * the caller still owns it. */
AT("0007AB70")
void NfpUnmount(s32 handle)
{
 NfpSetMountActive(handle,0);
}
/** @return How many of the archive's mount slots are currently active. */
AT("0007ABBC")
s32 NfpCountMounted(void)
{
 s32 count=0,i;
 for(i=0;i<gNfpState->mount_count;i++)
  if(NfpMountIsActive(i)) count++;
 return count;
}
