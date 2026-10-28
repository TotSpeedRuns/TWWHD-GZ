/* Portable execution of the two exact authored selectors. This uses the host
 * compiler's named bitfields; it does not test PPC bitfield wire layout. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../native-heap-v147/lifetime.h"
#include "../native-heap-v147/lifetime_storage.h"
HlStoredAllocation _gz_hl_allocations[HL_ALLOCS];
#include "hl_lookup.inc"
static unsigned checks;
static void check(int value){assert(value);++checks;}
static u32 hash(u32 p){return ((p>>2)*0x9e3779b1u)>>14;}
static void put(u32 i,u32 p,u32 heap,u32 kind,u32 block,u32 state){
 HlStoredAllocation *r=_gz_hl_allocations+i;
 memset(r,0,sizeof(*r));r->pointer=p;r->heap=heap;r->kind=kind;r->block=block;r->state=state;
}
int main(void){
 const u32 p=0x12345678u,h=0x11000000u,start=hash(p),mask=HL_ALLOCS-1u;
 u32 i;HlAllocation a,b;HlStoredAllocation stored;
 memset(_gz_hl_allocations,0,sizeof(_gz_hl_allocations));
 check(allocation_find(p,h,0)==0);
 check(allocation_find(p,h,1)==_gz_hl_allocations+start);
 put(start,p,h,HL_EXP,0,HL_LIVE);
 check(allocation_find(p,h,0)==_gz_hl_allocations+start);
 check(allocation_find(p,h,1)==_gz_hl_allocations+start);
 /* Same pointer, distinct heap and Custom descriptors are separate keys. */
 put((start+1)&mask,p,h+4,HL_EXP,0,HL_LIVE);
 put((start+2)&mask,p,h+8,HL_CUSTOM,0x12000000u,HL_LIVE);
 put((start+3)&mask,p,h+8,HL_CUSTOM,0x12000020u,HL_LIVE);
 check(allocation_find(p,h+4,0)==_gz_hl_allocations+((start+1)&mask));
 check(custom_find(p,h+8,0x12000000u,0)==_gz_hl_allocations+((start+2)&mask));
 check(custom_find(p,h+8,0x12000020u,0)==_gz_hl_allocations+((start+3)&mask));
 check(custom_find(p,h+8,0x12000040u,0)==0);
 /* An earlier DEAD row does not beat a later exact match. */
 _gz_hl_allocations[start].state=HL_DEAD;
 check(custom_find(p,h+8,0x12000020u,1)==_gz_hl_allocations+((start+3)&mask));
 check(allocation_find(p,h+12,0)==0);
 check(allocation_find(p,h+12,1)==_gz_hl_allocations+start);
 check(custom_find(p,h+8,0x12000040u,1)==_gz_hl_allocations+start);
 /* First DEAD wins, not the last DEAD or the following never-used slot. */
 _gz_hl_allocations[(start+1)&mask].state=HL_DEAD;
 check(allocation_find(p,h+12,1)==_gz_hl_allocations+start);
 /* Full fixed capacity still terminates and refuses without a tombstone. */
 for(i=0;i<HL_ALLOCS;i++)put(i,p,h,HL_EXP,0,HL_LIVE);
 check(allocation_find(p,h+4,0)==0);
 check(allocation_find(p,h+4,1)==0);
 check(custom_find(p,h,0x12000000u,0)==0);
 check(custom_find(p,h,0x12000000u,1)==0);
 _gz_hl_allocations[(start+HL_ALLOCS-1)&mask].state=HL_DEAD;
 check(allocation_find(p,h+4,1)==_gz_hl_allocations+((start+HL_ALLOCS-1)&mask));
 check(custom_find(p,h,0x12000000u,1)==_gz_hl_allocations+((start+HL_ALLOCS-1)&mask));
 /* Exercise the shared public layout adapters without asserting host wire ABI. */
 memset(&a,0,sizeof(a));a.pointer=p;a.kind=HL_EXP;a.state=HL_LIVE;a.missing=31;
 check(hl_import(&stored,&a));hl_export(&b,&stored);check(b.pointer==p&&b.missing==31&&b.kind==HL_EXP);
 printf("HL selector cases: %u passed\n",checks);return 0;
}
