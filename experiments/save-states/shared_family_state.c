#include "native_shared_data.h"
/* Broader typed state, never a raw saved actor/engine-object replay. */
#include "family_state.h"
#include "heap_owner.h"
typedef unsigned char u8;typedef unsigned short u16;
#define R32(p,o) (*(volatile const u32*)((p)+(o)))
#define R16(p,o) (*(volatile const u16*)((p)+(o)))
#define R8(p,o) (*(volatile const u8*)((p)+(o)))
#define W32(p,o,v) (*(volatile u32*)((p)+(o))=(v))
typedef struct {u16 offset,bytes;u32 floats;} Field;
/* Actor primitives, independent collision flags/vectors, Acch numerical
 * state and child simulation values. Self/native/callback/resource pointers,
 * polygon/query handles, vtables, scheduler fields and padding are excluded. */
static const Field fields[]={
 {0xb0,4,0},{0x2e0,4,0},{0x300,12,1},{0x30c,6,0},{0x312,1,0},
 {0x314,12,1},{0x320,6,0},{0x326,1,0},{0x328,6,0},{0x330,12,1},
 {0x33c,12,1},{0x370,12,1},{0x37c,12,1},{0x388,8,0},{0x390,12,1},
 {0x39c,4,0},{0x3a0,2,0},{0x3e4,4,0},{0x448,16,1},{0x458,16,1},
 {0x46c,1,0},{0x470,24,1},{0x48c,4,1},{0x590,4,0},
 {0x614,12,1},{0x620,2,0},{0x624,8,0},{0x634,4,0},{0x638,1,0},
 {0x63c,8,0},{0x64c,4,0},{0x650,8,0},{0x664,4,0},{0x674,8,0},
 {0x688,1,0},{0x690,4,0},{0x694,36,1},{0x6b8,8,0},{0x6cc,1,0},
 {0x6d4,4,0},{0x6d8,36,1},{0x700,8,0},{0x714,1,0},
 {0x73c,20,1},{0x754,12,1},{0x768,3,0},{0x774,3,0},{0x778,2,0},{0x78c,4,0},
 {0x798,2,0},{0x79a,9,0},{0x7a4,12,0},{0x7b0,16,1},{0x7cc,12,1},
 {0x7d8,48,1},{0x808,32,1},{0x8b4,1,0},
 {0x8c0,1,0},{0x8c2,4,0},{0x8c8,12,1},{0x8d4,1,0},
 {0x8dc,1,0},{0x8de,4,0},{0x8e4,12,1},{0x8f0,1,0},
 {0x8f8,1,0},{0x8fa,4,0},{0x900,12,1},{0x90c,1,0},
 {0x910,12,1},{0x91c,8,0},{0x924,8,1},{0x92c,2,0},{0x930,12,1}
};
static const u16 refs[PF_REF_COUNT]={0x77c,0x780,0x784,0x788,0x8bc,0x8d8,0x8f4,0x680,0x6c4,0x70c};
static const u16 audit[PF_AUDIT_COUNT]={0x62c,0x644,0x658,0x684,0x6c8,0x710,0x67c,0x6c0,0x708};
static const u16 hit_flags[3]={0x628,0x640,0x654};
enum {ACTOR_BYTES=2364};
#define FOLLOW_VT ((u32)_gz_native_follow_vtable)
static u32 count(void){return sizeof(fields)/sizeof(fields[0]);}
u32 _gz_pf_field_count(void){return count();}
u32 _gz_pf_field_word(u32 i){return i<count()?((u32)fields[i].offset<<16)|fields[i].bytes:0;}
u32 _gz_pf_packed_bytes(void){u32 i,n=0;for(i=0;i<count();i++)n+=fields[i].bytes;return n;}
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static u32 apart(u32 p,u32 n,u32 q,u32 m){return p+n>=p&&q+m>=q&&(p+n<=q||q+m<=p);}
static u32 packet(u32 p){u32 lo=(u32)_gz_pf_private_begin,hi=(u32)_gz_pf_private_end;
 return apart(p,PF_BYTES,(u32)&_gz_gh_identity,sizeof(_gz_gh_identity))&&
 (span(p,PF_BYTES)||(!(p&3)&&hi>lo&&p>=lo&&p<hi&&PF_BYTES<=hi-p));}
static void copy(u32 d,u32 s,u32 n){u32 i;for(i=0;i<n;i++)*(volatile u8*)(d+i)=R8(s,i);}
static u32 same(u32 a,u32 b,u32 n){u32 i;for(i=0;i<n;i++)if(R8(a,i)!=R8(b,i))return 0;return 1;}
static u32 hash(u32 p){u32 i,h=2166136261u;
 for(i=0;i<PF_BYTES;i++){h=(h^((i>=12&&i<16)||(i>=60&&i<64)?0:R8(p,i)))*16777619u;}
 return h;}
static u32 layout(void){u32 i,h=2166136261u;for(i=0;i<count();i++){
 h=(h^_gz_pf_field_word(i))*16777619u;h=(h^fields[i].floats)*16777619u;}return h;}
static u32 value(u32 p,u32 off,u32 bytes){u32 i,j,k=PF_HEADER,v=0;
 for(i=0;i<count();i++){
 if(off>=fields[i].offset&&off-fields[i].offset+bytes<=fields[i].bytes){
 for(j=0;j<bytes;j++){v=(v<<8)|R8(p,k+off-fields[i].offset+j);}
 return v;}
 k+=fields[i].bytes;}return 0;}
static u32 finite(u32 p){u32 i,j,k=PF_HEADER,v;
 for(i=0;i<count();i++){if(fields[i].floats)for(j=0;j<fields[i].bytes;j+=4){
 v=(R8(p,k+j)<<24)|(R8(p,k+j+1)<<16)|(R8(p,k+j+2)<<8)|R8(p,k+j+3);
 if((v&0x7f800000u)==0x7f800000u)return 0;}k+=fields[i].bytes;}return 1;}
static void pack(u32 p,u32 a){u32 i,k=PF_HEADER;for(i=0;i<count();i++){copy(p+k,a+fields[i].offset,fields[i].bytes);k+=fields[i].bytes;}}
static void unpack(u32 a,u32 p){u32 i,k=PF_HEADER;for(i=0;i<count();i++){copy(a+fields[i].offset,p+k,fields[i].bytes);k+=fields[i].bytes;}}
static u32 self_offset(u32 off,u32 angle){
 if(angle)return off==0x30c||off==0x320||off==0x328||off==0x7a4||off==0x7a6||off==0x7a8;
 return off==0x300||off==0x314||off==0x33c||off==0x370||off==0x37c||off==0x390||
 off==0x7cc||off==0x80c||off==0x81c||off==0x910||off==0x930;
}
static void encode_ref(u32 p,u32 address,u32 actor,u32 angle){
 if(!address){W32(p,0,0);W32(p,4,0);}
 else if(address>=actor&&self_offset(address-actor,angle)){W32(p,0,1);W32(p,4,address-actor);}
 else{W32(p,0,2);W32(p,4,address);}
}
/* Native GObj construction initializes its actor-ID cache to zero. Preserve
 * that exact numeric value only for an inactive hit tuple with no object or
 * actor pointer. Zero in any active tuple or other PID field remains a real
 * relationship requirement; it is never treated as a universal null PID. */
static u32 inert_ref(u32 p,u32 i){u32 n=i-7;
 return i>=7&&i<10&&!R32(p,PF_REFS+i*4)&&!(value(p,hit_flags[n],4)&1)&&
 !R32(p,PF_AUDIT+n*4)&&!R32(p,PF_AUDIT+(n+3)*4);}
static u32 dep(u32 p){u32 i,c,d=0;
 for(i=0;i<PF_REF_COUNT;i++)if(R32(p,PF_REFS+i*4)!=0xffffffffu&&!inert_ref(p,i))d|=PF_RELATED_PID;
 for(i=0;i<PF_AUDIT_COUNT;i++)if(R32(p,PF_AUDIT+i*4))d|=PF_COLLISION_GRAPH;
 for(i=0;i<PF_CB_COUNT;i++){
 c=p+PF_CB+i*28;
 if(R32(c,0)!=FOLLOW_VT)d|=PF_CALLBACK_CLASS;
 if(R32(c,4))d|=PF_EMITTER;
 if(R32(c,8)==2||R32(c,16)==2)d|=PF_EXTERNAL_CALLBACK;
 }
 if(R32(p,28)==7||R32(p,92))d|=PF_BRK;
 if(R32(p,32)==4||(value(p,0x2e0,4)&0x102000u))d|=PF_LINK_CARRY;
 if(R32(p,96)==2)d|=PF_EXTERNAL_DRAW_MODEL;
 if(R32(p,44)!=0xffffffffu)d|=PF_PARENT;
 return d;
}
static u32 basic(u32 a,u32 pid){
 if(!span(a,ACTOR_BYTES)||pid==0xffffffffu||R32(a,4)!=pid||R16(a,8)!=453||R8(a,12)!=2||R8(a,0x3aa)!=1)return PF_ACTOR;
 if(R32(a,0x790)>=16||R32(a,0x794)>=8||((R32(a,0xb0)>>24)&15)!=R32(a,0x790))return PF_ACTOR;
 if(R32(a,0x3e8)!=a+0x314||R32(a,0x3ec)!=a+0x300||R32(a,0x3f0)!=a+0x33c||
 R32(a,0x3f4)!=a+0x320||R32(a,0x3f8)!=a+0x328||R32(a,0x43c)!=a||R32(a,0x444)!=a+0x580||
 R32(a,0x668)!=a+0x754||R32(a,0x760)!=a||R32(a,0x764)!=pid)return PF_BINDING;
 return 0;
}
static u32 check(u32 p){u32 i,j,k,c,kind,v;
 if(_gz_pf_packed_bytes()>PF_SCALAR_CAP||R32(p,0)!=PF_MAGIC||R32(p,4)!=PF_BYTES||R32(p,8)!=2||R32(p,12)!=1||R32(p,16)!=453||
 R32(p,20)==0xffffffffu||R32(p,28)>=16||R32(p,32)>=8||R32(p,40)>65535||R32(p,48)>65535||
 R32(p,52)!=_gz_pf_packed_bytes()||R32(p,56)!=count()||R32(p,104)!=layout()||
 R32(p,108)!=PF_CB_COUNT||R32(p,112)!=PF_REF_COUNT||R32(p,116)!=PF_AUDIT_COUNT||R32(p,120)||R32(p,124)||
 !span(R32(p,64),ACTOR_BYTES)||R32(p,96)>2)return PF_FORMAT;
 if(hash(p)!=R32(p,60))return PF_CHECKSUM;
 if(!finite(p))return PF_FLOAT;
 for(i=72;i<84;i+=4)if((R32(p,i)&0x7f800000u)==0x7f800000u)return PF_FLOAT;
 if(value(p,0xb0,4)!=R32(p,36)||((R32(p,36)>>24)&15)!=R32(p,28)||value(p,0x798,2)!=R32(p,48))return PF_FORMAT;
 if(R32(p,96)==0&&R32(p,100))return PF_FORMAT;
 if(R32(p,96)!=0&&!R32(p,100))return PF_FORMAT;
 if(R32(p,96)==1&&!span(R32(p,100),0xf8))return PF_FORMAT;
 for(i=0;i<PF_CB_COUNT;i++)for(j=0;j<2;j++){
 c=p+PF_CB+i*28+8+j*8;kind=R32(c,0);v=R32(c,4);
 if(kind>2||(!kind&&v)||(kind==1&&!self_offset(v,j))||(kind==2&&!v))return PF_FORMAT;
 }
 if(dep(p)!=R32(p,68))return PF_FORMAT;
 /* Every byte outside named regions is canonical zero. */
 k=PF_HEADER+_gz_pf_packed_bytes();for(i=k;i<PF_REFS;i++)if(R8(p,i))return PF_FORMAT;
 for(i=PF_REFS+PF_REF_COUNT*4;i<PF_AUDIT;i++)if(R8(p,i))return PF_FORMAT;
 for(i=PF_AUDIT+PF_AUDIT_COUNT*4;i<PF_CB;i++)if(R8(p,i))return PF_FORMAT;
 for(i=PF_CB+PF_CB_COUNT*28;i<PF_BYTES;i++)if(R8(p,i))return PF_FORMAT;
 return 0;
}
u32 _gz_pf_packet_check(u32 p){return packet(p)?check(p):PF_RANGE;}
u32 _gz_pf_capture(u32 a,u32 out,u32 pid,u32 epoch){
 u8 scratch[PF_BYTES] __attribute__((aligned(4)));u32 p=(u32)scratch,i,c,v,model,e;
 if(!packet(out)||!apart(out,PF_BYTES,a,ACTOR_BYTES))return PF_RANGE;
 if(_gz_pf_packed_bytes()>PF_SCALAR_CAP)return PF_FORMAT;
 if(_gz_pt_epoch!=epoch)return PF_CONTEXT;
 e=basic(a,pid);if(e)return e;
 for(i=0;i<PF_BYTES;i++)scratch[i]=0;
 W32(p,0,PF_MAGIC);W32(p,4,PF_BYTES);W32(p,8,2);W32(p,12,1);W32(p,16,453);
 W32(p,20,pid);W32(p,24,epoch);W32(p,28,R32(a,0x790));W32(p,32,R32(a,0x794));
 W32(p,36,R32(a,0xb0));W32(p,40,R16(a,0x2d8));W32(p,44,R32(a,0x2e8));W32(p,48,R16(a,0x798));
 W32(p,52,_gz_pf_packed_bytes());W32(p,56,count());W32(p,64,a);copy(p+72,a+0x2ec,20);
 W32(p,92,R32(a,0x3b8));model=R32(a,0x3b4);v=R32(a,0x368);
 W32(p,96,!v?0:v==model?1:2);W32(p,100,v);W32(p,104,layout());
 W32(p,108,PF_CB_COUNT);W32(p,112,PF_REF_COUNT);W32(p,116,PF_AUDIT_COUNT);
 pack(p,a);
 for(i=0;i<PF_REF_COUNT;i++)W32(p,PF_REFS+4*i,R32(a,refs[i]));
 for(i=0;i<PF_AUDIT_COUNT;i++)W32(p,PF_AUDIT+4*i,R32(a,audit[i]));
 for(i=0;i<PF_CB_COUNT;i++){
 c=p+PF_CB+i*28;v=a+0x828+i*20;
 W32(c,0,R32(v,0));W32(c,4,R32(v,4));encode_ref(c+8,R32(v,8),a,0);encode_ref(c+16,R32(v,12),a,1);W32(c,24,R32(v,16));
 }
 W32(p,68,dep(p));W32(p,60,hash(p));e=check(p);if(e)return e;
 e=basic(a,pid);if(e)return e;
 if(_gz_pt_epoch!=epoch)return PF_CONTEXT;
 W32(out,12,0);copy(out,p,12);copy(out+16,p+16,PF_BYTES-16);
 if(_gz_pt_epoch!=epoch)return PF_CONTEXT;
 __asm__ volatile("sync":::"memory");
 W32(out,12,1);return 0;
}
static u32 current_dep(u32 a){u32 i,c,d=0,model=R32(a,0x3b4),v=R32(a,0x368);
 for(i=0;i<PF_REF_COUNT;i++)if(R32(a,refs[i])!=0xffffffffu){
 if(i<7||R32(a,refs[i])||(R32(a,hit_flags[i-7])&1)||R32(a,audit[i-7])||R32(a,audit[i-4]))d|=PF_RELATED_PID;
 }
 for(i=0;i<PF_AUDIT_COUNT;i++)if(R32(a,audit[i]))d|=PF_COLLISION_GRAPH;
 for(i=0;i<PF_CB_COUNT;i++){
 c=a+0x828+i*20;if(R32(c,0)!=FOLLOW_VT)d|=PF_CALLBACK_CLASS;if(R32(c,4))d|=PF_EMITTER;
 if(R32(c,8)&&!(R32(c,8)>=a&&self_offset(R32(c,8)-a,0)))d|=PF_EXTERNAL_CALLBACK;
 if(R32(c,12)&&!(R32(c,12)>=a&&self_offset(R32(c,12)-a,1)))d|=PF_EXTERNAL_CALLBACK;
 }
 if(R32(a,0x790)==7||R32(a,0x3b8))d|=PF_BRK;
 if(R32(a,0x794)==4||(R32(a,0x2e0)&0x102000u))d|=PF_LINK_CARRY;
 if(v&&v!=model)d|=PF_EXTERNAL_DRAW_MODEL;
 if(R32(a,0x2e8)!=0xffffffffu)d|=PF_PARENT;
 return d;
}
u32 _gz_pf_preflight(u32 a,u32 p,u32 pid,u32 epoch){u32 e;
 if(!packet(p)||!apart(p,PF_BYTES,a,ACTOR_BYTES))return PF_RANGE;
 if(_gz_pt_epoch!=epoch)return PF_CONTEXT;
 e=basic(a,pid);if(e)return e;e=check(p);if(e)return e;
 if(R32(a,0x790)!=R32(p,28)||R16(a,0x2d8)!=R32(p,40)||R32(a,0x2e8)!=R32(p,44)||
 R16(a,0x798)!=R32(p,48)||!same(p+72,a+0x2ec,19))return PF_RECIPE;
 if(R32(p,68)||current_dep(a))return PF_DEPENDENCY;
 e=_gz_pmr_model(a,pid,epoch);if(e)return PF_BINDING;
 if(_gz_pt_epoch!=epoch)return PF_CONTEXT;
 return 0;
}
u32 _gz_pf_apply_scalars(u32 a,u32 p,u32 pid,u32 epoch){
 u8 undo[PF_BYTES] __attribute__((aligned(4)));u32 u=(u32)undo,e,i,c,k,mode,draw,heap,model,checksum;
 e=_gz_pf_preflight(a,p,pid,epoch);if(e)return e;
 mode=R32(a,0x794);draw=R32(a,0x368);heap=R32(a,0xf4);model=R32(a,0x3b4);checksum=R32(p,60);
 pack(u,a);copy(u+PF_CB,a+0x828,140);
 for(i=0;i<PF_REF_COUNT;i++)W32(u,PF_REFS+i*4,R32(a,refs[i]));
 if(_gz_pt_epoch!=epoch||hash(p)!=checksum)return PF_CONTEXT;
 e=basic(a,pid);if(e)return e;
 if(R32(a,0xf4)!=heap||R32(a,0x3b4)!=model||current_dep(a))return PF_CONTEXT;
 unpack(a,p);
 for(i=0;i<PF_REF_COUNT;i++)W32(a,refs[i],R32(p,PF_REFS+i*4));
 for(i=0;i<PF_CB_COUNT;i++){
 c=p+PF_CB+i*28;k=a+0x828+i*20;
 W32(k,8,R32(c,8)?a+R32(c,12):0);W32(k,12,R32(c,16)?a+R32(c,20):0);W32(k,16,R32(c,24));
 }
 W32(a,0x368,R32(p,96)?model:0);
 __asm__ volatile("sync":::"memory");
 W32(a,0x794,R32(p,32));
 if(R32(a,4)!=pid||R16(a,8)!=453||R32(a,0xf4)!=heap||R32(a,0x3b4)!=model)return PF_DIRTY;
 if(_gz_pt_epoch!=epoch||hash(p)!=checksum){
 unpack(a,u);
 for(i=0;i<PF_REF_COUNT;i++)W32(a,refs[i],R32(u,PF_REFS+i*4));
 for(i=0;i<PF_CB_COUNT;i++)copy(a+0x828+i*20+8,u+PF_CB+i*20+8,12);
 W32(a,0x368,draw);W32(a,0x794,mode);
 return (R32(a,4)==pid&&R16(a,8)==453&&R32(a,0xf4)==heap&&R32(a,0x3b4)==model)?PF_ROLLED_BACK:PF_DIRTY;
 }
 return 0;
}
