#include "native_shared_data.h"
/* Private offline candidate. Actual original-RPX sead Heap registry, not a
 * GameCube JKRHeap/JSUTree layout. No game writes, allocator, virtual calls,
 * locks or process access. See native-evidence.txt for pinned provenance. */
#include "heap_owner.h"
#define R32(p,o) (*(volatile const u32 *)((p)+(o)))
#define ROOT0 ((u32)_gz_native_heap_root0)
#define ROOT1 ((u32)_gz_native_heap_root1)
#define MANAGER ((u32)_gz_native_heap_manager)
#define MANAGER_SLOT ((u32)_gz_native_heap_manager_slot)
#define SOLID_TABLE ((u32)_gz_native_heap_solid_vtable)
volatile GhIdentity _gz_gh_identity;
typedef struct {u32 target,absence,nodes,found,model,model_bytes;} Scan;
static u32 span(u32 p,u32 n) {
 return !(p&3u)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;
}
static u32 heap_table(u32 t) {
 /* Five base-Heap layouts observed in this exact native image. Unknown
  * subclasses/metadata regions deny the proof; they never get skipped. */
 return t==(u32)_gz_native_heap_exp_vtable||t==SOLID_TABLE||t==(u32)_gz_native_heap_vtable_10145ccc||
        t==(u32)_gz_native_heap_vtable_10145eb8||t==(u32)_gz_native_heap_vtable_1015f84c;
}
static u32 walk(Scan *s,u32 heap,u32 parent,u32 depth,u32 under) {
 u32 count,sentinel,first,last,node,prev,i,next,child;
 /* This comparison MUST precede every read at the target address. */
 if(heap==s->target) {
  if(s->absence)return 0;
  if(++s->found!=1)return 0;
  under=1;
 }
 if(depth>=GH_MAX_DEPTH||++s->nodes>GH_MAX_NODES||!span(heap,0x94))return 0;
 if(!heap_table(R32(heap,0xc))||R32(heap,0x24)!=parent||R32(heap,0x34)!=0x38)return 0;
 /* A subheap may occupy an allocation carved out of the target solid heap.
  * The model must not belong to such a registered descendant. This check
  * uses its validated current registry membership, never the saved heap. */
 if(under&&heap!=s->target&&s->model) {
  u32 start=R32(heap,0x1c),bytes=R32(heap,0x20);
  if(!span(start,bytes)||
     (s->model<start+bytes&&start<s->model+s->model_bytes))return 0;
 }
 count=R32(heap,0x30);sentinel=heap+0x28;
 first=R32(heap,0x2c);last=R32(heap,0x28);
 if(count>GH_MAX_NODES-s->nodes)return 0;
 node=first;prev=sentinel;
 for(i=0;i<count;i++) {
  if(node==sentinel||node<0x38)return 0;
  child=node-0x38;
  /* A saved/retired target's node is inside its old header. Recognizing
   * the reference must not first read prev/next or parent from that node. */
  if(s->absence&&child==s->target)return 0;
  if(!span(node,8)||R32(node,0)!=prev)return 0;
  next=R32(node,4);
  if(!walk(s,child,heap,depth+1,under))return 0;
  /* Recheck local topology so observed changes deny, without claiming an
   * atomic/concurrent snapshot. The held engine boundary remains required. */
  if(R32(node,0)!=prev||R32(node,4)!=next)return 0;
  prev=node;node=next;
 }
 return node==sentinel&&last==prev&&R32(heap,0x30)==count&&
        R32(heap,0x2c)==first&&R32(heap,0x28)==last&&
        R32(heap,0x34)==0x38&&R32(heap,0x24)==parent;
}
static u32 scan(u32 target,u32 absence,u32 model,u32 model_bytes) {
 Scan s;u32 roots[8],counts[2],i,j,n=0,a,count;
 s.target=target;s.absence=absence;s.nodes=0;s.found=0;
 s.model=model;s.model_bytes=model_bytes;
 if(!span(target,0x9c)||R32(MANAGER_SLOT,0)!=MANAGER)return 0;
 for(i=0;i<2;i++) {
  a=i?ROOT1:ROOT0;count=R32(a,0);counts[i]=count;
  /* Exact static-capacity arrays established by the native initializer. */
  if(count>4||R32(a,4)!=4||R32(a,8)!=a+12)return 0;
  if(!i&&!count)return 0;
  for(j=0;j<count;j++) {
   u32 k,root=R32(a+12,j*4);
   /* First record every root reference; absence needs no target reads. */
   if(absence&&root==target)return 0;
   for(k=0;k<n;k++)if(roots[k]==root)return 0;
   roots[n++]=root;
  }
 }
 for(i=0;i<n;i++)if(!walk(&s,roots[i],0,0,0))return 0;
 if(R32(MANAGER_SLOT,0)!=MANAGER)return 0;
 n=0;
 for(i=0;i<2;i++) {
  a=i?ROOT1:ROOT0;
  if(R32(a,0)!=counts[i]||R32(a,4)!=4||R32(a,8)!=a+12)return 0;
  for(j=0;j<counts[i];j++)if(R32(a+12,j*4)!=roots[n++])return 0;
 }
 return absence||s.found==1;
}
u32 _gz_gh_absent(u32 heap) {
 _gz_gh_identity.valid=0;
 return scan(heap,1,0,0)?GH_OK:GH_CLOSURE;
}
u32 _gz_gh_solid(u32 heap,u32 model,u32 model_bytes) {
 u32 parent,start,size,end,head,tail,name,parent_start,parent_size;
 _gz_gh_identity.valid=0;
 if(!span(model,model_bytes)||!scan(heap,0,model,model_bytes))return GH_CLOSURE;
 /* Only now is the candidate known reachable from the current game registry. */
 if(R32(heap,0xc)!=SOLID_TABLE||R32(heap,0x50)!=1)return GH_CLOSURE;
 parent=R32(heap,0x24);start=R32(heap,0x1c);size=R32(heap,0x20);
 if(!parent||R32(heap,0)!=parent||start!=heap||size<0x9c||!span(start,size))return GH_CLOSURE;
 end=start+size;head=R32(heap,0x94);tail=R32(heap,0x98);name=R32(heap,0x10);
 if(!name||head<start+0x9c||head>tail||tail>end)return GH_CLOSURE;
 /* The complete model must be allocated, not merely inside the heap's span. */
 if(!((model>=start+0x9c&&model<=head&&model_bytes<=head-model)||
      (model>=tail&&model<end&&model_bytes<=end-model)))return GH_CLOSURE;
 parent_start=R32(parent,0x1c);parent_size=R32(parent,0x20);
 if(!span(parent_start,parent_size)||start<parent_start||end-parent_start>parent_size)return GH_CLOSURE;
 if(R32(heap,0xc)!=SOLID_TABLE||R32(heap,0x24)!=parent||R32(heap,0x1c)!=start||
    R32(heap,0x20)!=size||R32(heap,0x50)!=1||R32(heap,0x94)!=head||
    R32(heap,0x98)!=tail||R32(heap,0x10)!=name)return GH_CLOSURE;
 _gz_gh_identity.heap=heap;_gz_gh_identity.parent=parent;_gz_gh_identity.vtable=SOLID_TABLE;
 _gz_gh_identity.name=name;_gz_gh_identity.start=start;_gz_gh_identity.bytes=size;
 _gz_gh_identity.direction=1;_gz_gh_identity.head=head;_gz_gh_identity.tail=tail;
 _gz_gh_identity.model=model;_gz_gh_identity.model_bytes=model_bytes;
 __asm__ volatile("":::"memory");_gz_gh_identity.valid=1;
 return GH_OK;
}
