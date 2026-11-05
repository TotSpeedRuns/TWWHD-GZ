/* Private fixed-provider CPU preflight. No callback/function-pointer authority,
 * SDK allocation, GPU call, game write, source publication or reset call.
 * Native bindings are deliberately unresolved until independently qualified.
 * Scope/staging checks run with interrupts enabled, outside the HL lock.
 */
#include "render_graph.h"
#include "../render-normalized-slots-r1/render_trace.h"
#include <stddef.h>
#include "render_refusal_trace.h"
#include "hx_index_layout.h"
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
extern const unsigned char _gz_hx_index[];
extern const unsigned char _gz_rcg_fixed_packets[],_gz_rcg_fixed_packet_initialized[],_gz_rcg_fixed_packet_vtable[];
extern const unsigned char _gz_rcg_static_receiver[],_gz_rcg_static_receiver_published[],_gz_rcg_static_receiver_initialized[],_gz_rcg_static_receiver_vtable[];
static U symbol(U n){switch(n){
 case 0:return(U)(uintptr_t)_gz_rcg_game_owner;case 1:return(U)(uintptr_t)_gz_rcg_game_initialized;
 case 2:return(U)(uintptr_t)_gz_rcg_j3d;case 3:return(U)(uintptr_t)_gz_hl_control;
 case 4:return(U)(uintptr_t)_gz_hl_allocations;case 5:return(U)(uintptr_t)_gz_hl_heaps;
 case 6:return(U)(uintptr_t)_gz_hm_scopes;case 7:return(U)(uintptr_t)_gz_rcg_native_ghc_proposal;
 case 8:return(U)(uintptr_t)_gz_hl_exp_table;case 9:return(U)(uintptr_t)_gz_hl_solid_table;
 case 10:return(U)(uintptr_t)_gz_hl_unit_table;
 case 11:return(U)(uintptr_t)_gz_rcg_fixed_packets;
 case 12:return(U)(uintptr_t)_gz_rcg_fixed_packet_initialized;
 case 13:return(U)(uintptr_t)_gz_rcg_fixed_packet_vtable;
 case 14:return(U)(uintptr_t)_gz_hx_index;
 case 15:return(U)(uintptr_t)_gz_rcg_static_receiver;
 case 16:return(U)(uintptr_t)_gz_rcg_static_receiver_published;
 case 17:return(U)(uintptr_t)_gz_rcg_static_receiver_initialized;
 default:return(U)(uintptr_t)_gz_rcg_static_receiver_vtable;}}
#define SYM(n) symbol(n)
static U load(U p){return *(volatile const U*)(uintptr_t)p;}
static U*scratch_pointer(U p,U n){(void)n;return(U*)(uintptr_t)p;}
#endif
typedef struct {U *s,*nodes,*fields,*log;U pass,at,first,reads,count,error,stage,bad,cell;
 U scratch,owner,raw,bytes,initial_raw,initial_bytes;U proposal[37],control[16],index_control[8];U reason;} Work;
static const U offsets[24]={0x1c,0x20,0x24,0x28,0x2c,0x34,0x38,0x3c,0x30,0x40,0x44,0x48,0x4c,0x50,0x54,0x58,0x5c,0x60,0x64,0x6c,0x68,0x70,0x74,0x78};
static const U counts[24]={32,32,1,32,32,1,32,32,32,128,128,256,256,32,32,256,32,32,32,1,1,1,1,32};
static U span(U p,U n){return !(p&3)&&n&&p>=0x10000000u&&p<=0xbffe0000u-n&&n<=0xaffe0000u;}
static U inside(U p,U n,U q,U m){return n&&p>=q&&p-q<m&&n<=m-(p-q);}
static U apart(U p,U n,U q,U m){return p>=q?p-q>=m:q-p>=n;}
static U fail(Work*w,U z,U p){if(!w->error){w->error=z;w->bad=p;}return 0;}
/* Private stack reason: a later wrapper never replaces the first refusal. */
__attribute__((noinline)) static U refuse(Work*w,U z,U p,U reason){
 if(!w->error){w->error=z;w->bad=p;w->reason=reason;}return 0;}
static U refusal_step(const Work*w){return w->reason&&w->pass<2&&w->stage>=1&&w->stage<=3?
 RT_RCG_REFUSAL_V1|(w->pass<<11)|(w->stage<<8)|w->reason:
 RT_RCG_WALK|(w->pass<<4)|w->stage;}
static U meta(Work*w,U p){if(w->error)return 0;if(w->reads++==RCG_READ_BUDGET){refuse(w,RCG_LIMIT,p,RRF_READ_BUDGET);return 0;}return load(p);}
static U graph(Work*w,U p){U v=meta(w,p),at=w->at;if(w->error)return 0;
 if(at==RCG_LOG){refuse(w,RCG_LIMIT,p,RRF_GRAPH_LOG_LIMIT);return 0;}
 if(!w->pass){w->log[at*2]=p;w->log[at*2+1]=v;}
 else if(at>=w->first||w->log[at*2]!=p||w->log[at*2+1]!=v){refuse(w,RCG_CHANGED,p,RRF_GRAPH_REPLAY_DRIFT);return 0;}
 w->at=at+1;return v;}
static U insert(Work*w,U*p,U mask,U value,U kind){U i,start=((value>>2)*0x9e3779b1u)&mask;
 for(i=0;i<RCG_HASH_PROBES;i++){U at=(start+i)&mask;if(!p[at]){p[at]=value;return 1;}if(p[at]==value)return refuse(w,kind,value,kind==RCG_GRAPH?RRF_NODE_DUPLICATE:RRF_FIELD_DUPLICATE);}
 return refuse(w,RCG_LIMIT,value,RRF_HASH_PROBE_LIMIT);}
static U present(const U*p,U mask,U value){U i,start=((value>>2)*0x9e3779b1u)&mask;
 for(i=0;i<RCG_HASH_PROBES;i++){U v=p[(start+i)&mask];if(!v)return 0;if(v==value)return 1;}return 0;}
static U field(Work*w,U p){return insert(w,w->fields,RCG_FIELDS-1,p,RCG_ALIAS);}
static U node(Work*w,U p){if(w->count==RCG_MAX_NODES)return refuse(w,RCG_LIMIT,p,RRF_NODE_LIMIT);
 if(!insert(w,w->nodes,RCG_HASH-1,p,RCG_GRAPH))return 0;
 ++w->count;return 1;}
static U heap(Work*w,U p,U out[16],U*slot){U i,j,start=(((p>>2)*0x9e3779b1u)>>20);
 for(i=0;i<RCG_HASH_PROBES&&!w->error;i++){U at=(start+i)&4095,base=SYM(5)+at*64,key=meta(w,base);
  if(!key)return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_MISSING);
  if(key!=p)continue;
  out[0]=key;for(j=1;j<16;j++)out[j]=meta(w,base+j*4);
  if(w->error)return 0;
  if(!out[1])return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_GENERATION);
 if(out[2]!=1)return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_STATE);
 if(out[3]<1)return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_KIND_LOW);
 if(out[3]>3)return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_KIND_HIGH);
 if(!span(out[4],out[5]))return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_SPAN);
 if(out[11])return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_WORD11);
 if(out[13])return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_WORD13);
 if((out[14]&~8u))return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_FLAGS);
 if(out[8]!=SYM(7+out[3]))return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_TABLE);
 if((out[9]!=1&&out[9]!=0xffffffffu))return refuse(w,RCG_UNSUPPORTED,p,RRF_HEAP_MODE);
  *slot=at;return 1;
 }return refuse(w,RCG_LIMIT,p,RRF_HEAP_PROBE_LIMIT);}

/* All index reads are fixed owner metadata and use meta(), including bitmap
 * traversal and every bucket link. No hidden unbounded cursor call can bypass
 * the unchanged READ_BUDGET. The genuine common HL lock excludes every writer;
 * control words and index generation are checked again before acceptance. */
typedef struct {U pointer,start,edge,node,visited,epoch;} IndexWalk;
static U index_read(Work*w,U offset){return meta(w,SYM(14)+offset);}
static U index_from(U bit){return bit<32?~0u<<bit:0;}
static U index_first(U bits){return 31u-(U)__builtin_clz(bits&(0u-bits));}
static U index_empty_linear(Work*w,U start){
 U at=(U)offsetof(HxIndex,empty),word=start>>5,m=word>>5,u=m>>5,v;
 v=index_read(w,at+word*4)&index_from(start&31);
 if(v)return(word<<5)+index_first(v);
 v=index_read(w,at+(U)offsetof(HxBits,middle)+m*4)&index_from((word&31)+1);
 if(v){word=(m<<5)+index_first(v);v=index_read(w,at+word*4);return v?(word<<5)+index_first(v):HX_BAD_ROW;}
 v=index_read(w,at+(U)offsetof(HxBits,upper)+u*4)&index_from((m&31)+1);
 if(v){m=(u<<5)+index_first(v);v=index_read(w,at+(U)offsetof(HxBits,middle)+m*4);if(!v)return HX_BAD_ROW;
  word=(m<<5)+index_first(v);v=index_read(w,at+word*4);return v?(word<<5)+index_first(v):HX_BAD_ROW;}
 v=index_read(w,at+(U)offsetof(HxBits,root))&index_from(u+1);if(!v)return HX_NONE;
 u=index_first(v);v=index_read(w,at+(U)offsetof(HxBits,upper)+u*4);if(!v)return HX_BAD_ROW;
 m=(u<<5)+index_first(v);v=index_read(w,at+(U)offsetof(HxBits,middle)+m*4);if(!v)return HX_BAD_ROW;
 word=(m<<5)+index_first(v);v=index_read(w,at+word*4);return v?(word<<5)+index_first(v):HX_BAD_ROW;
}
static U index_begin(Work*w,U pointer,IndexWalk*c){U empty,root;
 c->pointer=pointer;c->start=((pointer>>2)*0x9e3779b1u)>>14;c->visited=0;c->epoch=w->index_control[5];
 root=index_read(w,(U)offsetof(HxIndex,empty)+(U)offsetof(HxBits,root));
 if(w->error)return 0;if(root&~255u)return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_EMPTY_ROOT);
 empty=root?index_empty_linear(w,c->start):HX_NONE;
 if(empty==HX_NONE&&root)empty=index_empty_linear(w,0);
 if(w->error)return 0;if(empty==HX_BAD_ROW)return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_EMPTY_TREE);
 c->edge=empty==HX_NONE?HX_ROWS:(empty-c->start)&HX_MASK;
 c->node=index_read(w,(U)offsetof(HxIndex,buckets)+c->start*4);
 return !w->error&&(c->node<=HX_ROWS?1:refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_HEAD_RANGE));
}
static U index_next(Work*w,IndexWalk*c){U id,base,key,next;
 if(index_read(w,(U)offsetof(HxIndex,generation))!=c->epoch)return refuse(w,RCG_CHANGED,SYM(14),RRF_INDEX_EPOCH_DRIFT);
 while(c->node&&!w->error){
  if(c->node>HX_ROWS)return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_NODE_RANGE);
  if(c->visited++==HX_ROWS)return refuse(w,RCG_LIMIT,c->pointer,RRF_INDEX_VISIT_LIMIT);
  id=c->node-1;base=SYM(4)+id*60;key=meta(w,base);
  next=index_read(w,(U)offsetof(HxIndex,next)+id*4);
  if(w->error)return 0;
  if(!key)return refuse(w,RCG_UNSUPPORTED,c->pointer,RRF_INDEX_KEY_ZERO);
 if((((key>>2)*0x9e3779b1u)>>14)!=c->start)return refuse(w,RCG_UNSUPPORTED,c->pointer,RRF_INDEX_KEY_BUCKET);
 if(next>HX_ROWS)return refuse(w,RCG_UNSUPPORTED,c->pointer,RRF_INDEX_NEXT_RANGE);
  c->node=next;
  if(key==c->pointer&&((id-c->start)&HX_MASK)<c->edge)return base;
 }
 return 0;
}

/* Exact-pointer lookup only. Interior allocation ownership is deliberately not
 * guessed; outside the genuine current raw span it returns UNSUPPORTED.
 * Pair/parent-child aliases must all be considered before the first empty row.
 */
static U allocation(Work*w,U p,U n,U exact){U i,j,found=0,ended=0;IndexWalk cursor;
 if(!span(p,n))return refuse(w,RCG_ALIAS,p,RRF_ALLOC_SPAN);
 if(!apart(p,n,w->scratch,RCG_SCRATCH_BYTES))return refuse(w,RCG_ALIAS,p,RRF_ALLOC_SCRATCH_ALIAS);
 if(!apart(p,n,SYM(14),sizeof(HxIndex)))return refuse(w,RCG_ALIAS,p,RRF_ALLOC_INDEX_ALIAS);
 if(!index_begin(w,p,&cursor))return 0;
 for(i=0;i<RCG_ALLOC_PROBES&&!w->error;i++){
  U base=index_next(w,&cursor),a[15],h[16],hs,policy,flags,kind;
  if(!base){ended=!w->error;break;}
  a[0]=meta(w,base);if(a[0]!=p)return refuse(w,RCG_CHANGED,p,RRF_ALLOC_KEY_DRIFT);
  for(j=1;j<15;j++)a[j]=meta(w,base+j*4);
  policy=a[14];if((policy>>29)!=1)continue;
  flags=(policy>>17)&511;kind=(policy>>26)&7;
  if(found++)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_DUPLICATE);
 if(!a[1])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_GENERATION);
 if(!a[3])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_HEAP_GENERATION);
 if(a[4]<n)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_REQUESTED_SMALL);
 if((exact&&a[4]!=exact))return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_EXACT_SIZE);
 if(a[7]<a[4])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_CAPACITY_SMALL);
 if(!span(p,a[7]))return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_CAPACITY_SPAN);
 if(kind<1)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_KIND_LOW);
 if(kind>3)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_KIND_HIGH);
 if(a[9])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_END_FRAME);
 if(a[11])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_END_SITE);
 if(a[12])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_RELEASE_TOKEN);
 if(flags&~8u)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_FLAGS);
 if(((policy>>12)&31)!=31)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_VALIDATION_MASK);
 if((policy&4095))return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_POLICY_LOW);
  if(!heap(w,a[2],h,&hs))return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_HEAP_LOOKUP);
 if(h[1]!=a[3])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_HEAP_GENERATION_MISMATCH);
 if(h[3]!=kind)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_HEAP_KIND_MISMATCH);
 if(h[15]!=a[13])return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_HEAP_EPOCH_MISMATCH);
 if(!inside(p,a[7],h[4],h[5]))return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_MANAGED_SPAN);
  if(kind==1){if(!span(a[6],16))return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_EXP_HEADER_SPAN);
   if(a[6]>p)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_EXP_HEADER_AFTER);
   if(p-a[6]<16)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_EXP_HEADER_GAP);
   if(!inside(a[6],16,h[4],h[5]))return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_EXP_HEADER_MANAGED);
  }else if(a[6]!=p)return refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_BLOCK_POINTER);
 }
 if(w->error)return 0;
 if(!ended)return refuse(w,RCG_LIMIT,p,RRF_ALLOC_PROBE_LIMIT);
 return found?1:refuse(w,RCG_UNSUPPORTED,p,RRF_ALLOC_MISSING);
}
static U raw_or_allocation(Work*w,U p,U n){
 if(!span(p,n))return refuse(w,RCG_ALIAS,p,RRF_RAW_SPAN);
 if(!apart(p,n,w->scratch,RCG_SCRATCH_BYTES))return refuse(w,RCG_ALIAS,p,RRF_RAW_SCRATCH_ALIAS);
 return inside(p,n,w->raw,w->bytes)?1:allocation(w,p,n,0);
}
static U proposal(Work*w,U req,U frame){U i,h[16],hs,begin,end,head,p=SYM(7);
 for(i=0;i<37;i++){U v=meta(w,p+i*4);if(w->pass&&v!=w->proposal[i])return refuse(w,RCG_CHANGED,p+i*4,RRF_PROPOSAL_DRIFT);if(!w->pass)w->proposal[i]=v;}
 if(w->error)return 0;
 if(w->proposal[0]!=1)return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_VALID);
 if(w->proposal[1]!=req)return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_REQUEST);
 if(w->proposal[2]!=frame)return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_FRAME);
 if(w->proposal[34]!=5)return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_VERSION);
 if(!span(w->proposal[3],w->proposal[4]))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_RAW_SPAN);
 if((w->proposal[4]&3))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_RAW_ALIGNMENT);
 if(w->proposal[5]<w->proposal[4])return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_CAPACITY);
 if(w->proposal[3]!=w->initial_raw)return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_RAW_IDENTITY);
 if(w->proposal[4]!=w->initial_bytes)return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_BYTES_IDENTITY);
 if(!apart(w->proposal[3],w->proposal[4],w->scratch,RCG_SCRATCH_BYTES))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_SCRATCH_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(3),64))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_CONTROL_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(4),262144u*60u))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_ALLOCATIONS_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(5),4096u*64u))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_HEAPS_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(6),4096u*32u))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_SCOPES_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(7),148))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_SELF_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(11),4*0x98))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_PACKETS_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(12),16))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_GUARDS_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(15),936))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_STATIC_RECEIVER_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(16),4))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_STATIC_PUBLISHED_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(17),4))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_STATIC_INITIALIZED_ALIAS);
 if(!apart(w->proposal[3],w->proposal[4],SYM(14),sizeof(HxIndex)))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_INDEX_ALIAS);
 if(!heap(w,w->proposal[10],h,&hs))return 0;
 for(i=0;i<16;i++)if(h[i]!=w->proposal[10+i])return refuse(w,RCG_CHANGED,p+40+i*4,RRF_PROPOSAL_HEAP_COPY);
 for(i=0;i<8;i++)if(meta(w,SYM(6)+hs*32+i*4)!=w->proposal[26+i])return refuse(w,RCG_CHANGED,p+104+i*4,RRF_PROPOSAL_SCOPE_COPY);
 if(w->proposal[31])return refuse(w,RCG_UNSUPPORTED,p,RRF_PROPOSAL_SCOPE_WORD5);
 if(w->proposal[32])return refuse(w,RCG_UNSUPPORTED,p,RRF_PROPOSAL_SCOPE_WORD6);
 if(w->proposal[33])return refuse(w,RCG_UNSUPPORTED,p,RRF_PROPOSAL_SCOPE_WORD7);
 if(h[8]!=SYM(7+h[3]))return refuse(w,RCG_UNSUPPORTED,p,RRF_PROPOSAL_HEAP_TABLE);
 if(!span(h[0],h[3]==1?0xb8:h[3]==2?0x9c:0xac))return refuse(w,RCG_UNSUPPORTED,p,RRF_PROPOSAL_HEAP_HEADER_SPAN);
 head=h[3]==1?0xb8:h[3]==2?0x9c:0xac;begin=h[0]<h[4]?h[0]:h[4];end=h[4]+h[5];if(h[0]+head>end)end=h[0]+head;
 if(end<=begin)return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_RAW_END);
 if(!span(begin,end-begin))return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_COMBINED_SPAN);
 if(begin!=w->proposal[3])return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_COMBINED_START);
 if(end-begin!=w->proposal[4])return refuse(w,RCG_CHANGED,p,RRF_PROPOSAL_COMBINED_BYTES);
 w->raw=begin;w->bytes=end-begin;return 1;
}
static U controls(Work*w){U i;
 for(i=0;i<16;i++){U v=meta(w,SYM(3)+i*4);if(w->pass&&v!=w->control[i])return refuse(w,RCG_CHANGED,SYM(3)+i*4,RRF_CONTROL_DRIFT);if(!w->pass)w->control[i]=v;}
 for(i=0;i<8;i++){U v=index_read(w,i*4);if(w->pass&&v!=w->index_control[i])return refuse(w,RCG_CHANGED,SYM(14)+i*4,RRF_INDEX_CONTROL_DRIFT);if(!w->pass)w->index_control[i]=v;}
 if(w->error)return 0;
 if(w->index_control[0]!=HX_MAGIC)return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_MAGIC);
 if(w->index_control[1]!=sizeof(HxIndex))return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_BYTES);
 if(w->index_control[2]!=HX_VERSION)return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_VERSION);
 if(w->index_control[3]!=1)return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_READY);
 if(w->index_control[4]>HX_ROWS)return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_OCCUPIED);
 if(!w->index_control[5])return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_GENERATION);
 if(w->index_control[6])return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_RESERVED6);
 if(w->index_control[7])return refuse(w,RCG_UNSUPPORTED,SYM(14),RRF_INDEX_RESERVED7);
 if(w->error)return 0;
 if(w->control[0]!=0x484c4231u)return refuse(w,RCG_UNSUPPORTED,SYM(3),RRF_CONTROL_MAGIC);
 if(w->control[1]!=64)return refuse(w,RCG_UNSUPPORTED,SYM(3),RRF_CONTROL_BYTES);
 if(w->control[2]!=1)return refuse(w,RCG_UNSUPPORTED,SYM(3),RRF_CONTROL_VERSION);
 if(w->control[3]!=1)return refuse(w,RCG_UNSUPPORTED,SYM(3),RRF_CONTROL_READY);
 if(w->control[4])return refuse(w,RCG_UNSUPPORTED,SYM(3),RRF_CONTROL_FLAGS);
 if(w->control[7]>4096)return refuse(w,RCG_UNSUPPORTED,SYM(3),RRF_CONTROL_HEAP_COUNT);
 if(w->control[8]>262144)return refuse(w,RCG_UNSUPPORTED,SYM(3),RRF_CONTROL_ALLOCATION_COUNT);
 return 1;}
/* Four exact native static packet receivers, each constructed as0x98 bytes
 * and entered via genuine J3DDrawBuffer::entry at bucket0. They are module BSS,
 * not heap allocations. No other static address/interior pointer is accepted.
 * Native lazy guards and installed derived vtables must both match; logged
 * reads participate in the existing stable two-pass current-graph check.
 */
static U fixed_packet(Work*w,U p){U d,i;
 if(p<SYM(11))return 0;d=p-SYM(11);
 if(d>3u*0x98u||d%0x98u)return 0;i=d/0x98u;
 if(graph(w,SYM(12)+i*4)!=1)return refuse(w,RCG_UNSUPPORTED,p,RRF_STATIC_PACKET_GUARD);
 if(graph(w,p+0xc)!=SYM(13)+(i&1?0:0x30))return refuse(w,RCG_UNSUPPORTED,p,RRF_STATIC_PACKET_VTABLE);
 return !w->error;
}
/* One additional exact original936B static receiver. Native setup passes the
 * nonnull fixed object to the152B packet constructor, then publishes it only
 * after constructor completion and static-destructor registration. A genuine
 * original call queues that published pointer through J3DDrawBuffer::entry.
 * Never admit its interiors, neighbours, queue arrays or receiver child lists.
 * The guard is a32-bit word and is set BEFORE construction; it alone is not
 * sufficient. Publication and installed derived vtable must also match.
 * All three observations join the existing ordered stable two-pass log.
 */
static U static_receiver(Work*w,U p){
 if(p!=SYM(15))return 0;
 if(graph(w,SYM(16))!=p)return refuse(w,RCG_UNSUPPORTED,p,RRF_STATIC_RECEIVER_PUBLISHED);
 if(graph(w,SYM(17))!=1)return refuse(w,RCG_UNSUPPORTED,p,RRF_STATIC_RECEIVER_INITIALIZED);
 if(graph(w,p+12)!=SYM(18))return refuse(w,RCG_UNSUPPORTED,p,RRF_STATIC_RECEIVER_VTABLE);
 return !w->error;
}
static U graph_pass(Work*w){U q[24],arrays[24],i,j,p,count,array,head,cur,next,child,entry,e[2],cursors[4]={0x8c,0xd4,0x1dc,0x264};
 for(i=0;i<RCG_HASH;i++)w->nodes[i]=0;
 for(i=0;i<RCG_FIELDS;i++)w->fields[i]=0;
 w->count=0;w->at=0;
 if(graph(w,SYM(1))!=1)return refuse(w,RCG_UNSUPPORTED,SYM(1),RRF_GAME_INITIALIZED);
 for(i=0;i<24;i++){
  p=graph(w,w->owner+offsets[i]);if(w->error||!allocation(w,p,64,64))return 0;q[i]=p;
  for(j=0;j<i;j++)if(!apart(p,64,q[j],64))return refuse(w,RCG_ALIAS,p,RRF_QUEUE_OBJECT_ALIAS);
  for(j=0;j<16;j++){U v=graph(w,p+j*4);if(!j)count=v;else if(j==1)array=v;}
  if(w->error||count!=counts[i])return refuse(w,RCG_GRAPH,p,RRF_QUEUE_COUNT);
  if(!allocation(w,array,count*4,count*4))return 0;
  arrays[i]=array;
  for(j=0;j<i;j++)if(!apart(array,count*4,arrays[j],counts[j]*4))return refuse(w,RCG_ALIAS,array,RRF_QUEUE_ARRAY_ALIAS);
  if(!field(w,p)||!field(w,p+4)||!field(w,p+0x20))return 0;
 }
 for(i=0;i<24;i++)for(j=0;j<24;j++)if(!apart(arrays[i],counts[i]*4,q[j],64))return refuse(w,RCG_ALIAS,arrays[i],RRF_QUEUE_ARRAY_CROSS_ALIAS);
 if(graph(w,SYM(2)+0x74)!=q[11])return refuse(w,RCG_GRAPH,SYM(2)+0x74,RRF_J3D_QUEUE11);
 if(graph(w,SYM(2)+0x78)!=q[12])return refuse(w,RCG_GRAPH,SYM(2)+0x74,RRF_J3D_QUEUE12);
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
   if(!embedded){U fixed=fixed_packet(w,cur);if(w->error)return 0;if(!fixed)fixed=static_receiver(w,cur);if(w->error||(!fixed&&!raw_or_allocation(w,cur,0x98)))return 0;}
   if(!field(w,cur+0x10)||!field(w,cur+0x14)||!field(w,cur+0x94))return 0;
   next=graph(w,cur+0x10);child=graph(w,cur+0x14);entry=graph(w,cur+0x94);
   if(w->error||entry!=w->cell)return refuse(w,RCG_GRAPH,cur+0x94,RRF_PACKET_ENTRY);
   if((next&&!span(next,0x98)))return refuse(w,RCG_GRAPH,cur,RRF_PACKET_NEXT_SPAN);
 if((child&&!span(child,4)))return refuse(w,RCG_GRAPH,cur,RRF_PACKET_CHILD_SPAN);
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
  if(entry&&!present(w->nodes,RCG_HASH-1,e[i]))return refuse(w,RCG_GRAPH,e[i]+0x94,RRF_EMBEDDED_ENTRY);
  if(!present(w->nodes,RCG_HASH-1,e[i])&&!field(w,e[i]+0x94))return 0;
  if(!field(w,e[i]+0x98))return 0;
 }
 for(i=0;i<2;i++){
  cur=graph(w,e[i]+0x98);
  while(cur&&!w->error){
   if(!raw_or_allocation(w,cur,4)||!node(w,cur)||!field(w,cur))return 0;
   /* Reject nodes inside fixed owner storage: their clear could alter a later
    * receiver/list field even if that field is not currently dereferenced. */
   if(!apart(cur,4,w->owner,0x3bc))return refuse(w,RCG_ALIAS,cur,RRF_LIST_OWNER_ALIAS);
   next=graph(w,cur);if(next&&!span(next,4))return refuse(w,RCG_GRAPH,cur,RRF_LIST_NEXT_SPAN);cur=next;
  }
 }
 if(!w->pass)w->first=w->at;else if(w->at!=w->first)return refuse(w,RCG_CHANGED,w->owner,RRF_GRAPH_WORD_COUNT);
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
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(7),148)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(11),4*0x98)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(12),16)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(15),936)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(16),4)||
  !apart(scratch,RCG_SCRATCH_BYTES,SYM(17),4)||
  !span(SYM(14),sizeof(HxIndex))||!apart(scratch,RCG_SCRATCH_BYTES,SYM(14),sizeof(HxIndex))){
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
 if(z)_gz_rns_trace(req,frame,refusal_step(&w),z,w.bad);
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
