/* FldSet handler 080122F8 passes the field basename and signed pixel
 * coordinates here. 08006A1C stores that basename at engine state +0x12F4.
 * The first load enables palette+tile transfer (flags 3); the second reuses
 * those resources (flags 0). Viewport slots have a 0xFC-byte stride.
 * The original 16-byte filename buffer and unchecked copies are preserved.
 */
#include "kmp.h"
#define AT(x) __attribute__((section(".rom." x)))
extern void sub_08006A1C(const char *);
extern char *strcpy(char *,const char *);
extern char *strcat(char *,const char *);
extern char *strupr(char *);
extern void sub_08003178(char *,void *,s32,s32,s32,s32,s32);
AT("000032B8") void KmpLoadField(const char *name,s16 x,s16 y)
{
 char resource[16]; /* basename + .KMP + terminator; original has no length check */
 struct KmpViewport *view;
 s32 px=x,py=y;
 sub_08006A1C(name);
 strcpy(resource,name);
 strcat(resource,(const char *)0x08086A5C);
 strupr(resource);
 sub_08003178(resource,(void *)0x06000000,0,0,0,0,3);
 sub_08003178(resource,(void *)0x06000000,1,1,0,0,0);
 view=(struct KmpViewport *)0x03003BC4;
 px*=65536;py*=65536;
 KmpRenderViewport(view,px,py);
 view=(struct KmpViewport *)((u8 *)view+0xFC);
 KmpRenderViewport(view,px,py);
}
AT("00003280") void KmpSetClip(struct KmpViewport *view,u32 x,u32 y,u32 width,u32 height)
{view->clipX=x;view->clipY=y;view->clipWidth=width;view->clipHeight=height;}
AT("00003280") const u8 KmpSetClipTail[2]={0,0};
AT("00003294") void KmpResetClip(struct KmpViewport *view)
{view->clipX=0;view->clipY=0;view->clipWidth=view->data->widthTiles;view->clipHeight=view->data->heightTiles;}
AT("00003294") const u8 KmpResetClipTail[2]={0,0};
