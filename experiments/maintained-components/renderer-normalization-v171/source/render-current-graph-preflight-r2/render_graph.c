/* Private fixed-provider CPU preflight. No callback/function-pointer authority,
 * SDK allocation, GPU call, game write, source publication or reset call.
 * Native bindings are deliberately unresolved until independently qualified.
 * Scope/staging checks run with interrupts enabled, outside the HL lock.
 */
#include "render_graph.h"
#include "../render-normalized-slots-r1/render_trace.h"
#include <stddef.h>
typedef uint32_t U;
extern U _gz_ghs_scope(U,U,U),_gz_ghs_hg(U,U);
extern U _gz_ois_heap_stage_match(U,U,U,U),_gz_ois_heap_apart(U,U,U,U);
extern U _gz_hl_meta_try_lock(void);extern void _gz_hl_meta_unlock(void);
#ifdef RCG_HOST_TEST
extern U _gz_rcg_test_read(U),_gz_rcg_test_symbol(U);
extern U *_gz_rcg_test_scratch(U,U);
#define SYM(n) _gz_rcg_test_symbol(n)
static U load(U p){return _gz_rcg_test_read(p);}
static U*scratch_pointer(U p,U n){return _gz_rcg_test_scratch(p,n);}
#else
extern const unsigned char _gz_rcg_game_owner[],_gz_rcg_game_initialized[],_gz_rcg_j3d[];
extern const unsigned char _gz_hl_control[],_gz_hl_allocations[],_gz_hl_heaps[],_gz_hm_scopes[];
extern const unsigned char _gz_rcg_native_ghc_proposal[]; /* exact GhcProposal148 */
extern const unsigned char _gz_hl_exp_table[],_gz_hl_solid_table[],_gz_hl_unit_table[];
static U symbol(U n){switch(n){
 case 0:return(U)(uintptr_t)_gz_rcg_game_owner;case 1:return(U)(uintptr_t)_gz_rcg_game_initialized;
 case 2:return(U)(uintptr_t)_gz_rcg_j3d;case 3:return(U)(uintptr_t)_gz_hl_control;
 case 4:return(U)(uintptr_t)_gz_hl_allocations;case 5:return(U)(uintptr_t)_gz_hl_heaps;
 case 6:return(U)(uintptr_t)_gz_hm_scopes;case 7:return(U)(uintptr_t)_gz_rcg_native_ghc_proposal;
 case 8:return(U)(uintptr_t)_gz_hl_exp_table;case 9:return(U)(uintptr_t)_gz_hl_solid_table;
 default:return(U)(uintptr_t)_gz_hl_unit_table;}}
#define SYM(n) symbol(n)
static U load(U p){return *(volatile const U*)(uintptr_t)p;}
static U*scratch_pointer(U p,U n){(void)n;return(U*)(uintptr_t)p;}
#endif
typedef struct {U *s,*nodes,*fields,*log;U pass,at,first,reads,count,error,stage,bad,cell;
 U scratch,owner,raw,bytes,initial_raw,initial_bytes;U proposal[37],control[16];} Work;
static const U offsets[24]={0x1c,0x20,0x24,0x28,0x2c,0x34,0x38,0x3c,0x30,0x40,0x44,0x48,0x4c,0x50,0x54,0x58,0x5c,0x60,0x64,0x6c,0x68,0x70,0x74,0x78};
static const U counts[24]={32,32,1,32,32,1,32,32,32,128,128,256,256,32,32,256,32,32,32,1,1,1,1,32};
static U span(U p,U n){return !(p&3)&&n&&p>=0x10000000u&&p<=0xbffe0000u-n&&n<=0xaffe0000u;}
static U inside(U p,U n,U q,U m){return n&&p>=q&&p-q<m&&n<=m-(p-q);}
static U apart(U p,U n,U q,U m){return p>=q?p-q>=m:q-p>=n;}
static U fail(Work*w,U z,U p){if(!w->error){w->error=z;w->bad=p;}return 0;}
static U meta(Work*w,U p){if(w->error)return 0;if(w->reads++==RCG_READ_BUDGET){fail(w,RCG_LIMIT,p);return 0;}return load(p);}
static U graph(Work*w,U p){U v=meta(w,p),at=w->at;if(w->error)return 0;
 if(at==RCG_LOG){fail(w,RCG_LIMIT,p);return 0;}
 if(!w->pass){w->log[at*2]=p;w->log[at*2+1]=v;}
 else if(at>=w->first||w->log[at*2]!=p||w->log[at*2+1]!=v){fail(w,RCG_CHANGED,p);return 0;}
 w->at=at+1;return v;}
static U insert(Work*w,U*p,U mask,U value,U kind){U i,start=((value>>2)*0x9e3779b1u)&mask;
 for(i=0;i<RCG_HASH_PROBES;i++){U at=(start+i)&mask;if(!p[at]){p[at]=value;return 1;}if(p[at]==value)return fail(w,kind,value);}
 return fail(w,RCG_LIMIT,value);}
static U present(const U*p,U mask,U value){U i,start=((value>>2)*0x9e3779b1u)&mask;
 for(i=0;i<RCG_HASH_PROBES;i++){U v=p[(start+i)&mask];if(!v)return 0;if(v==value)return 1;}return 0;}
static U field(Work*w,U p){return insert(w,w->fields,RCG_FIELDS-1,p,RCG_ALIAS);}
static U node(Work*w,U p){if(w->count==RCG_MAX_NODES)return fail(w,RCG_LIMIT,p);
 if(!insert(w,w->nodes,RCG_HASH-1,p,RCG_GRAPH))return 0;
 ++w->count;return 1;}
static U heap(Work*w,U p,U out[16],U*slot){U i,j,start=(((p>>2)*0x9e3779b1u)>>20);
 for(i=0;i<RCG_HASH_PROBES&&!w->error;i++){U at=(start+i)&4095,base=SYM(5)+at*64,key=meta(w,base);
  if(!key)return fail(w,RCG_UNSUPPORTED,p);
  if(key!=p)continue;
  out[0]=key;for(j=1;j<16;j++)out[j]=meta(w,base+j*4);
  if(w->error)return 0;
  if(!out[1]||out[2]!=1||out[3]<1||out[3]>3||!span(out[4],out[5])||out[11]||out[13]||(out[14]&~8u)||
   out[8]!=SYM(7+out[3])||(out[9]!=1&&out[9]!=0xffffffffu))return fail(w,RCG_UNSUPPORTED,p);
  *slot=at;return 1;
 }return fail(w,RCG_LIMIT,p);}
/* Exact-pointer lookup only. Interior allocation ownership is deliberately not
 * guessed; outside the genuine current raw span it returns UNSUPPORTED.
 * Pair/parent-child aliases must all be considered before the first empty row.
 */
static U allocation(Work*w,U p,U n,U exact){U start=(((p>>2)*0x9e3779b1u)>>14),i,j,found=0,ended=0;
 if(!span(p,n)||!apart(p,n,w->scratch,RCG_SCRATCH_BYTES))return fail(w,RCG_ALIAS,p);
 for(i=0;i<RCG_ALLOC_PROBES&&!w->error;i++){
  U base=SYM(4)+((start+i)&262143)*60,a[15],h[16],hs,policy,flags,kind;
  a[0]=meta(w,base);if(!a[0]){ended=1;break;}if(a[0]!=p)continue;
  for(j=1;j<15;j++)a[j]=meta(w,base+j*4);
  policy=a[14];if((policy>>29)!=1)continue;
  flags=(policy>>17)&511;kind=(policy>>26)&7;
  if(found++||!a[1]||!a[3]||a[4]<n||(exact&&a[4]!=exact)||a[7]<a[4]||!span(p,a[7])||
   kind<1||kind>3||a[9]||a[11]||a[12]||flags&~8u||((policy>>12)&31)!=31||(policy&4095))return fail(w,RCG_UNSUPPORTED,p);
  if(!heap(w,a[2],h,&hs)||h[1]!=a[3]||h[3]!=kind||h[15]!=a[13]||!inside(p,a[7],h[4],h[5]))return fail(w,RCG_UNSUPPORTED,p);
  if(kind==1?(!span(a[6],16)||a[6]>p||p-a[6]<16||!inside(a[6],16,h[4],h[5])):a[6]!=p)return fail(w,RCG_UNSUPPORTED,p);
 }
 if(w->error)return 0;
 if(!ended)return fail(w,RCG_LIMIT,p);
 return found?1:fail(w,RCG_UNSUPPORTED,p);
}
static U raw_or_allocation(Work*w,U p,U n){
 if(!span(p,n)||!apart(p,n,w->scratch,RCG_SCRATCH_BYTES))return fail(w,RCG_ALIAS,p);
 return inside(p,n,w->raw,w->bytes)?1:allocation(w,p,n,0);
}
static U proposal(Work*w,U req,U frame){U i,h[16],hs,begin,end,head,p=SYM(7);
 for(i=0;i<37;i++){U v=meta(w,p+i*4);if(w->pass&&v!=w->proposal[i])return fail(w,RCG_CHANGED,p+i*4);if(!w->pass)w->proposal[i]=v;}
 if(w->error)return 0;
 if(w->proposal[0]!=1||w->proposal[1]!=req||w->proposal[2]!=frame||w->proposal[34]!=5||
  !span(w->proposal[3],w->proposal[4])||(w->proposal[4]&3)||w->proposal[5]<w->proposal[4]||
  w->proposal[3]!=w->initial_raw||w->proposal[4]!=w->initial_bytes||
  !apart(w->proposal[3],w->proposal[4],w->scratch,RCG_SCRATCH_BYTES)||
  !apart(w->proposal[3],w->proposal[4],SYM(3),64)||
  !apart(w->proposal[3],w->proposal[4],SYM(4),262144u*60u)||
  !apart(w->proposal[3],w->proposal[4],SYM(5),4096u*64u)||
  !apart(w->proposal[3],w->proposal[4],SYM(6),4096u*32u)||
  !apart(w->proposal[3],w->proposal[4],SYM(7),148))return fail(w,RCG_CHANGED,p);
 if(!heap(w,w->proposal[10],h,&hs))return 0;
 for(i=0;i<16;i++)if(h[i]!=w->proposal[10+i])return fail(w,RCG_CHANGED,p+40+i*4);
 for(i=0;i<8;i++)if(meta(w,SYM(6)+hs*32+i*4)!=w->proposal[26+i])return fail(w,RCG_CHANGED,p+104+i*4);
 if(w->proposal[31]||w->proposal[32]||w->proposal[33]||h[8]!=SYM(7+h[3])||!span(h[0],h[3]==1?0xb8:h[3]==2?0x9c:0xac))return fail(w,RCG_UNSUPPORTED,p);
 head=h[3]==1?0xb8:h[3]==2?0x9c:0xac;begin=h[0]<h[4]?h[0]:h[4];end=h[4]+h[5];if(h[0]+head>end)end=h[0]+head;
 if(end<=begin||!span(begin,end-begin)||begin!=w->proposal[3]||end-begin!=w->proposal[4])return fail(w,RCG_CHANGED,p);
 w->raw=begin;w->bytes=end-begin;return 1;
}
static U controls(Work*w){U i;
 for(i=0;i<16;i++){U v=meta(w,SYM(3)+i*4);if(w->pass&&v!=w->control[i])return fail(w,RCG_CHANGED,SYM(3)+i*4);if(!w->pass)w->control[i]=v;}
 return !w->error&&w->control[0]==0x484c4231u&&w->control[1]==64&&w->control[2]==1&&w->control[3]==1&&
  !w->control[4]&&w->control[7]<=4096&&w->control[8]<=262144?1:fail(w,RCG_UNSUPPORTED,SYM(3));}
static U graph_pass(Work*w){U q[24],arrays[24],i,j,p,count,array,head,cur,next,child,entry,e[2],cursors[4]={0x8c,0xd4,0x1dc,0x264};
 for(i=0;i<RCG_HASH;i++)w->nodes[i]=0;
 for(i=0;i<RCG_FIELDS;i++)w->fields[i]=0;
 w->count=0;w->at=0;
 if(graph(w,SYM(1))!=1)return fail(w,RCG_UNSUPPORTED,SYM(1));
 for(i=0;i<24;i++){
  p=graph(w,w->owner+offsets[i]);if(w->error||!allocation(w,p,64,64))return 0;q[i]=p;
  for(j=0;j<i;j++)if(!apart(p,64,q[j],64))return fail(w,RCG_ALIAS,p);
  for(j=0;j<16;j++){U v=graph(w,p+j*4);if(!j)count=v;else if(j==1)array=v;}
  if(w->error||count!=counts[i])return fail(w,RCG_GRAPH,p);
  if(!allocation(w,array,count*4,count*4))return 0;
  arrays[i]=array;
  for(j=0;j<i;j++)if(!apart(array,count*4,arrays[j],counts[j]*4))return fail(w,RCG_ALIAS,array);
  if(!field(w,p)||!field(w,p+4)||!field(w,p+0x20))return 0;
 }
 for(i=0;i<24;i++)for(j=0;j<24;j++)if(!apart(arrays[i],counts[i]*4,q[j],64))return fail(w,RCG_ALIAS,arrays[i]);
 if(graph(w,SYM(2)+0x74)!=q[11]||graph(w,SYM(2)+0x78)!=q[12])return fail(w,RCG_GRAPH,SYM(2)+0x74);
 if(!field(w,SYM(2)+0x74)||!field(w,SYM(2)+0x78))return 0;
 for(i=0;i<4;i++){(void)graph(w,w->owner+cursors[i]);if(!field(w,w->owner+cursors[i]))return 0;}
 /* Add all bucket cells before any packet fields, so cross-object write aliases
  * are rejected even when they involve a bucket processed later. */
 for(i=0;i<24;i++)for(j=0;j<counts[i];j++)if(!field(w,arrays[i]+j*4))return 0;
 for(i=0;i<24;i++)for(j=0;j<counts[i];j++){
  w->cell=arrays[i]+j*4;head=graph(w,w->cell);cur=head;
  while(cur&&!w->error){
   U embedded=cur==w->owner+0x284||cur==w->owner+0x320;
   if(!node(w,cur))return 0;
   if(!embedded&&!raw_or_allocation(w,cur,0x98))return 0;
   if(!field(w,cur+0x10)||!field(w,cur+0x14)||!field(w,cur+0x94))return 0;
   next=graph(w,cur+0x10);child=graph(w,cur+0x14);entry=graph(w,cur+0x94);
   if(w->error||entry!=w->cell)return fail(w,RCG_GRAPH,cur+0x94);
   if((next&&!span(next,0x98))||(child&&!span(child,4)))return fail(w,RCG_GRAPH,cur);
   cur=next;
  }
 }
 /* Authentic reset overwrites four scalar cursors without following them.
  * Special receivers are exact fixed Game DATA objects. After all24 queues
  * drain, their entryPtr must be zero before native remove(buffer48,e,0).
  * A nonzero initial entryPtr is permitted only for an actually queued e.
  */
 e[0]=w->owner+0x284;e[1]=w->owner+0x320;
 for(i=0;i<2;i++){
  entry=graph(w,e[i]+0x94);
  if(entry&&!present(w->nodes,RCG_HASH-1,e[i]))return fail(w,RCG_GRAPH,e[i]+0x94);
  if(!present(w->nodes,RCG_HASH-1,e[i])&&!field(w,e[i]+0x94))return 0;
  if(!field(w,e[i]+0x98))return 0;
 }
 for(i=0;i<2;i++){
  cur=graph(w,e[i]+0x98);
  while(cur&&!w->error){
   if(!raw_or_allocation(w,cur,4)||!node(w,cur)||!field(w,cur))return 0;
   /* Reject nodes inside fixed owner storage: their clear could alter a later
    * receiver/list field even if that field is not currently dereferenced. */
   if(!apart(cur,4,w->owner,0x3bc))return fail(w,RCG_ALIAS,cur);
   next=graph(w,cur);if(next&&!span(next,4))return fail(w,RCG_GRAPH,cur);cur=next;
  }
 }
 if(!w->pass)w->first=w->at;else if(w->at!=w->first)return fail(w,RCG_CHANGED,w->owner);
 return !w->error;
}
U _gz_rcg_preflight(U req,U frame,U scratch,U bytes){Work w;U i,hg,z=RCG_SCOPE,locked=0,writable=0,*s;
 if(!req||!frame||bytes<RCG_SCRATCH_BYTES||!span(scratch,RCG_SCRATCH_BYTES)){
  _gz_rns_trace(req,frame,RT_RCG_ARGUMENT,RCG_ARGUMENT,scratch);return RCG_ARGUMENT;}
 if(!_gz_ghs_scope(req,frame,2)){
  _gz_rns_trace(req,frame,RT_RCG_SCOPE,RCG_SCOPE,1);return RCG_SCOPE;}
 if(!(hg=_gz_ghs_hg(req,frame))){
  _gz_rns_trace(req,frame,RT_RCG_SCOPE,RCG_SCOPE,2);return RCG_SCOPE;}
 if(!_gz_ois_heap_stage_match(req,frame,scratch,RCG_SCRATCH_BYTES)){
  _gz_rns_trace(req,frame,RT_RCG_SCOPE,RCG_SCOPE,3);return RCG_SCOPE;}
 /* These genuine staging/world exclusions execute with IRQs enabled. */
 if(!apart(scratch,RCG_SCRATCH_BYTES,SYM(0),0x3bc)||!apart(scratch,RCG_SCRATCH_BYTES,SYM(1),4)||!apart(scratch,RCG_SCRATCH_BYTES,SYM(2),0x100)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(3),64)||!apart(scratch,RCG_SCRATCH_BYTES,SYM(4),262144u*60u)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(5),4096u*64u)||!apart(scratch,RCG_SCRATCH_BYTES,SYM(6),4096u*32u)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(7),148)){
  _gz_rns_trace(req,frame,RT_RCG_STAGE_APART,RCG_SCOPE,scratch);return RCG_SCOPE;}
 /* Read only the fixed native proposal to check staging disjointness before
  * scratch writes. Its raw authority is not used until root/scope revalidation
  * under the genuine HL lock below. */
 for(i=0;i<(U)sizeof(w);i++)((unsigned char*)&w)[i]=0;
 w.initial_raw=load(SYM(7)+12);w.initial_bytes=load(SYM(7)+16);
 if(!span(w.initial_raw,w.initial_bytes)){
  _gz_rns_trace(req,frame,RT_RCG_RAW_SPAN,RCG_SCOPE,w.initial_raw);return RCG_SCOPE;}
 if(!_gz_ois_heap_apart(req,frame,w.initial_raw,w.initial_bytes)){
  _gz_rns_trace(req,frame,RT_RCG_RAW_APART,RCG_SCOPE,w.initial_raw);return RCG_SCOPE;}
 s=scratch_pointer(scratch,RCG_SCRATCH_BYTES);
 w.s=s;w.nodes=s+RCG_HEADER_WORDS;w.fields=w.nodes+RCG_HASH;w.log=w.fields+RCG_FIELDS;
 w.scratch=scratch;w.owner=SYM(0);
 if(!_gz_hl_meta_try_lock()){z=RCG_BUSY;
  _gz_rns_trace(req,frame,RT_RCG_BUSY,z,SYM(3));goto unlocked;}locked=1;
 for(w.pass=0;w.pass<2;w.pass++){
  w.reads=0;w.stage=1;if(!controls(&w))break;
  w.stage=2;if(!proposal(&w,req,frame))break;
  if(!w.pass){for(i=0;i<RCG_HEADER_WORDS;i++)s[i]=0;
   s[0]=0x52434731;s[1]=RCG_CHANGED;s[8]=req;s[9]=frame;s[10]=hg;writable=1;}
  w.stage=3;if(!graph_pass(&w))break;
 }
 z=w.error?w.error:(w.pass==2?RCG_OK:RCG_CHANGED);
 if(z)_gz_rns_trace(req,frame,RT_RCG_WALK|(w.pass<<4)|w.stage,z,w.bad);
 _gz_hl_meta_unlock();locked=0;
unlocked:
 /* No scope/staging/JWA call above can execute while interrupts are masked. */
 if(!_gz_ghs_scope(req,frame,2)){
  _gz_rns_trace(req,frame,RT_RCG_POST_SCOPE,RCG_SCOPE,1);return RCG_SCOPE;}
 if(_gz_ghs_hg(req,frame)!=hg){
  _gz_rns_trace(req,frame,RT_RCG_POST_SCOPE,RCG_SCOPE,2);return RCG_SCOPE;}
 if(!_gz_ois_heap_stage_match(req,frame,scratch,RCG_SCRATCH_BYTES)){
  _gz_rns_trace(req,frame,RT_RCG_POST_SCOPE,RCG_SCOPE,3);return RCG_SCOPE;}
 if(!writable)return z;
 if(locked)_gz_hl_meta_unlock();
 s[1]=z;s[2]=w.stage;s[3]=w.bad;s[4]=w.cell;s[5]=w.count;s[6]=w.first;s[7]=w.pass;s[11]=w.raw;s[12]=w.bytes;
 if(!z)_gz_rns_trace(req,frame,RT_RCG_OK,0,w.count);
 return z;
}
