#define RBC_ACTION_TABLE_HASH 0x5ceb466du
/* Typed Rito behavior/pose source and exact current-attempt reversal.
 * Named numeric fields only: no dispatch, scheduler, path, event pointer,
 * message/partner identity, resource, collision object or emitter is copied.
 * Action is an enum from the exact owned immutable GHS action table. Action
 * transitions and external participants remain the outer transaction's duty.
 * This module has no runtime writer caller and grants no restore admission.
 */
enum {RBC_BYTES=1024,RBC_HEADER=128,RBC_BAD=1390,RBC_CHANGED=1391,RBC_ALIAS=1392,RBC_NUMERIC=1393,RBC_MISSING=63};
#define RBC_W(p,o,v) (*(volatile u32*)((p)+(o))=(v))
typedef struct {u16 offset,bytes;u32 floats;} RbcField;
static const RbcField rbc_fields[] __attribute__((section(".text.rbc_fields"),aligned(4)))={
 {0x2e0,4,0},{0x300,12,1},{0x30c,6,0},{0x312,1,0},{0x314,12,1},{0x320,6,0},{0x326,1,0},
 {0x328,6,0},{0x330,12,1},{0x33c,12,1},{0x370,24,1},{0x388,8,0},{0x390,12,1},{0x39c,4,0},{0x3a0,2,0},
 {0x3ac,38,0},{0x3d8,8,0},{0x7c0,8,0},{0x7cc,1,0},{0x7d4,4,0},{0x7d8,1,0},
 {0x87c,1,0},{0x87e,2,0},{0x8a4,96,1},
 {0x97c,12,1},{0x988,6,0},{0x990,12,1},{0x99c,12,1},{0x9a8,12,1},{0x9b4,24,1},
 {0x9cc,6,0},{0x9dc,14,0},{0x9ea,22,0},{0xa00,4,0},{0xa04,5,0},{0xa0c,48,1},
 {0xa3c,1,0},{0xa40,1,0},{0xa68,7,0},{0xa71,4,0},{0xa78,2,0}
};
static const u16 rbc_ref_offsets[] __attribute__((section(".text.rbc_refs"),aligned(4)))={
 0xf4,0x2e8,0x348,0x368,0x36c,0x3e0,0x3e8,0x3ec,0x3fc,0x420,0x448,0x44c,
 0x7c8,0x7d0,0x7e4,0x7f0,0x7f4,0x7f8,0x7fc,0x800,0x804,0x884,0x890,0x89c,0x8a0,0x904,
 0x90c,0x914,0x918,0x928,0x94c,0x974,0x978,0xa48,0xa4c,0xa50,0xa54,0xa58,0xa5c,0xa60,0xa64
};
typedef struct {u32 magic,bytes,valid,frame;PoEntry process;u32 refs,before_hash,target_hash;
 u8 before[RBC_BYTES],target[RBC_BYTES];u32 seal;} RbcPlan;
typedef char RbcPlanSize[sizeof(RbcPlan)==2144?1:-1];
u32 _gz_rbc_field_count(void){return sizeof(rbc_fields)/sizeof(rbc_fields[0]);}
u32 _gz_rbc_field_word(u32 i){return i<_gz_rbc_field_count()?((u32)rbc_fields[i].offset<<16)|rbc_fields[i].bytes:0;}
u32 _gz_rbc_packed_bytes(void){u32 i,n=0;for(i=0;i<_gz_rbc_field_count();i++)n+=rbc_fields[i].bytes;return n;}
static u32 rbc_layout(void){u32 i,h=2166136261u;for(i=0;i<_gz_rbc_field_count();i++){
 h=(h^_gz_rbc_field_word(i))*16777619u;h=(h^rbc_fields[i].floats)*16777619u;}return h;}
static u32 rbc_source_hash(u32 p){u32 i,h=2166136261u;for(i=0;i<RBC_BYTES;i++)h=(h^((i>=12&&i<16)||(i>=64&&i<68)?0:R8(p,i)))*16777619u;return h;}
static u32 rbc_plan_hash(const RbcPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
static u32 rbc_refs(u32 a){u32 i,h=2166136261u;for(i=0;i<sizeof(rbc_ref_offsets)/sizeof(rbc_ref_offsets[0]);i++)
 h=(h^R32(a,rbc_ref_offsets[i]))*16777619u;return h;}
/* Exact immutable data bytes were verified against the owned EUR v0 RPX.
 * A changed action table refuses rather than adopting a different pointer.
 */
/* All 88 original bytes, with only the eleven genuine retail ADDR32
 * fields resolved by the native loader. No pointer-shape guessing. */
static const u32 r313_rito_action_words[22]={0x0000ffffu,GZ_R313_NATIVE_02203910,0x0000ffffu,GZ_R313_NATIVE_022039E4,0x0000ffffu,GZ_R313_NATIVE_02203BB0,0x0000ffffu,GZ_R313_NATIVE_02203FB0,0x0000ffffu,GZ_R313_NATIVE_02203D4C,0x0000ffffu,GZ_R313_NATIVE_02203EDC,0x0000ffffu,GZ_R313_NATIVE_02203AA0,0x0000ffffu,GZ_R313_NATIVE_02203C48,0x0000ffffu,GZ_R313_NATIVE_02203C9C,0x0000ffffu,GZ_R313_NATIVE_02204048,0x0000ffffu,GZ_R313_NATIVE_02203DFC};
u32 _gz_r313_native_rito_table_valid(void){u32 i;
 for(i=0;i<22;i++)if(R32(GZ_R313_NATIVE_10016F18,i*4)!=r313_rito_action_words[i])return 0;
 return 1;
}

static u32 rbc_action(u32 a){u32 i,table=GZ_R313_NATIVE_10016F18;
 if(!_gz_r313_native_rito_table_valid())return 0;
 for(i=0;i<11;i++)if(R32(a,0x89c)==R32(table,i*8)&&R32(a,0x8a0)==R32(table,i*8+4))return i+1;
 return 0;}
static u32 rbc_value(u32 p,u32 offset,u32 bytes){u32 i,j,k=RBC_HEADER,v;
 for(i=0;i<_gz_rbc_field_count();i++){
  if(offset>=rbc_fields[i].offset&&offset-rbc_fields[i].offset+bytes<=rbc_fields[i].bytes){
   v=0;for(j=0;j<bytes;j++)v=(v<<8)|R8(p,k+offset-rbc_fields[i].offset+j);return v;}
  k+=rbc_fields[i].bytes;
 }return 0;
}
u32 _gz_rbc_check(u32 p){u32 i,j,k=RBC_HEADER,n=_gz_rbc_packed_bytes();
 if(!epj_descriptor(p,RBC_BYTES)||R32(p,0)!=0x52424331u||R32(p,4)!=RBC_BYTES||R32(p,8)!=1||R32(p,12)!=1||
  !R32(p,16)||R32(p,16)>=0xfffffffeu||!R32(p,20)||!R32(p,24)||!rtf_profile(R32(p,28))||R32(p,36)>65535||
  !R32(p,44)||R32(p,44)>11||R32(p,48)!=_gz_rbc_field_count()||R32(p,52)!=n||n>RBC_BYTES-RBC_HEADER||
  R32(p,56)!=rbc_layout()||R32(p,60)!=RBC_MISSING||R32(p,64)!=rbc_source_hash(p)||
  R32(p,68)>255||R32(p,72)>255||R32(p,76)>255||R32(p,80)>255||!rbs_zero(p,96,RBC_HEADER)||
  !rbs_zero(p,RBC_HEADER+n,RBC_BYTES)||R32(p,80)!=rbc_value(p,0x326,1))return RBC_BAD;
 for(i=0;i<_gz_rbc_field_count();i++){
  if(rbc_fields[i].floats)for(j=0;j<rbc_fields[i].bytes;j+=4){u32 v=(R8(p,k+j)<<24)|(R8(p,k+j+1)<<16)|(R8(p,k+j+2)<<8)|R8(p,k+j+3);if(!epj_finite(v))return RBC_NUMERIC;}
  k+=rbc_fields[i].bytes;
 }return 0;
}
u32 _gz_rbc_capture(const KbjPlan*j,u32 frame,u32 out){u8 scratch[RBC_BYTES] __attribute__((aligned(4)));
 PoEntry first,last;u32 p=(u32)scratch,z,a,i,k=RBC_HEADER,action,refs;
 if(frame!=_gz_pbac_frame)return RBC_BAD;z=rbs_current(j,1,frame,&first);if(z)return z;a=j->layout.common.bounds.actor;
 z=_gz_kbj_before_matches(j);if(z)return z;
 if(!rbs_output(j,out,RBC_BYTES)||!epj_apart(out,RBC_BYTES,p,RBC_BYTES)||!epj_apart(out,RBC_BYTES,(u32)&first,sizeof(first))||
  !epj_apart(out,RBC_BYTES,(u32)&last,sizeof(last)))return RBC_ALIAS;
 action=rbc_action(a);if(!action)return RBC_CHANGED;refs=rbc_refs(a);eks_zero(scratch,sizeof(scratch));
 RBC_W(p,0,0x52424331u);RBC_W(p,4,RBC_BYTES);RBC_W(p,8,1);RBC_W(p,12,1);RBC_W(p,16,first.pid);RBC_W(p,20,first.generation);RBC_W(p,24,frame);RBC_W(p,28,first.profile);
 RBC_W(p,32,R32(a,0xb0));RBC_W(p,36,R16(a,0x2d8));RBC_W(p,40,R32(a,0x2e8));RBC_W(p,44,action);
 RBC_W(p,48,_gz_rbc_field_count());RBC_W(p,52,_gz_rbc_packed_bytes());RBC_W(p,56,rbc_layout());RBC_W(p,60,RBC_MISSING);
 RBC_W(p,68,R8(a,0xa75));RBC_W(p,72,R8(a,0xa76));RBC_W(p,76,R8(a,0xa77));RBC_W(p,80,R8(a,0x326));
 RBC_W(p,84,R32(a,0x7c8));RBC_W(p,88,R32(a,0x7d0));RBC_W(p,92,R32(a,0x978));
 for(i=0;i<_gz_rbc_field_count();i++){btr_copy(scratch+k,(const void*)(a+rbc_fields[i].offset),rbc_fields[i].bytes);k+=rbc_fields[i].bytes;}
 RBC_W(p,64,rbc_source_hash(p));z=_gz_rbc_check(p);if(z)return z;
 k=RBC_HEADER;for(i=0;i<_gz_rbc_field_count();i++){if(!btr_equal(scratch+k,(const void*)(a+rbc_fields[i].offset),rbc_fields[i].bytes))return RBC_CHANGED;k+=rbc_fields[i].bytes;}
 z=rbs_current(j,1,frame,&last);if(z)return z;
 if(!btr_equal(&first,&last,sizeof(first))||rbc_refs(a)!=refs||rbc_action(a)!=action||R32(a,0xb0)!=R32(p,32)||R16(a,0x2d8)!=R32(p,36)||
  R32(a,0x2e8)!=R32(p,40)||R32(a,0x7c8)!=R32(p,84)||R32(a,0x7d0)!=R32(p,88)||R32(a,0x978)!=R32(p,92)||
  R8(a,0xa75)!=R32(p,68)||R8(a,0xa76)!=R32(p,72)||R8(a,0xa77)!=R32(p,76))return RBC_CHANGED;
 RBC_W(out,12,0);btr_copy((void*)out,scratch,12);btr_copy((void*)(out+16),scratch+16,RBC_BYTES-16);__asm__ volatile("sync":::"memory");RBC_W(out,12,1);return 0;
}
static u32 rbc_packet(const KbjPlan*j,const PoEntry*e,u32 p){u32 z=_gz_rbc_check(p),a=j->layout.common.bounds.actor;
 if(z)return z;
 return R32(p,28)==e->profile&&rri_identity(e,p)&&R32(p,32)==R32(a,0xb0)&&R32(p,36)==R16(a,0x2d8)&&R32(p,40)==R32(a,0x2e8)&&
  R32(p,44)==rbc_action(a)&&R32(p,68)==R8(a,0xa75)&&R32(p,72)==R8(a,0xa76)&&R32(p,76)==R8(a,0xa77)&&
  R32(p,84)==R32(a,0x7c8)&&R32(p,88)==R32(a,0x7d0)&&rri_child_match(e,R32(p,92),R32(a,0x978))?0:RBC_CHANGED;
}
u32 _gz_rbc_prepare(const KbjPlan*j,u32 source,u32 frame,RbcPlan*out){PoEntry current,again;const EpjBounds*b;u32 z,refs;
 if(frame!=_gz_pbac_frame)return RBC_BAD;z=rbs_current(j,1,frame,&current);if(z)return z;b=&j->layout.common.bounds;refs=rbc_refs(b->actor);
 if(!epj_ram(source,RBC_BYTES)||source<b->source||source-b->source>b->source_bytes||RBC_BYTES>b->source_bytes-(source-b->source))return RBC_ALIAS;
 z=rbc_packet(j,&current,source);if(z)return z;
 if(!out||!rbs_output(j,(u32)out,sizeof(*out)))return RBC_ALIAS;
 eks_zero(out,sizeof(*out));z=_gz_rbc_capture(j,frame,(u32)out->before);if(z)return z;
 btr_copy(out->target,(const void*)source,RBC_BYTES);z=rbc_packet(j,&current,(u32)out->target);if(z)return z;
 z=rbs_current(j,1,frame,&again);if(z)return z;if(!btr_equal(&current,&again,sizeof(current))||rbc_refs(b->actor)!=refs)return RBC_CHANGED;
 out->magic=0x52424a31u;out->bytes=sizeof(*out);out->frame=frame;btr_copy(&out->process,&current,sizeof(current));out->refs=refs;
 out->before_hash=btr_hash(out->before,RBC_BYTES);out->target_hash=btr_hash(out->target,RBC_BYTES);out->seal=rbc_plan_hash(out);
 __asm__ volatile("sync":::"memory");out->valid=1;return 0;
}
u32 _gz_rbc_validate(const KbjPlan*j,const RbcPlan*p){PoEntry current;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x52424a31u||p->bytes!=sizeof(*p)||p->valid!=1||p->seal!=rbc_plan_hash(p)||
  p->before_hash!=btr_hash(p->before,RBC_BYTES)||p->target_hash!=btr_hash(p->target,RBC_BYTES))return RBC_BAD;
 z=rbs_current(j,1,p->frame,&current);if(z)return z;
 if(!btr_equal(&p->process,&current,sizeof(current))||!rbs_output(j,(u32)p,sizeof(*p))||rbc_refs(current.object)!=p->refs)return RBC_CHANGED;
 z=rbc_packet(j,&current,(u32)p->before);return z?z:rbc_packet(j,&current,(u32)p->target);
}
static void rbc_write(u32 a,u32 p){u32 i,k=RBC_HEADER;for(i=0;i<_gz_rbc_field_count();i++){
 btr_copy((void*)(a+rbc_fields[i].offset),(const void*)(p+k),rbc_fields[i].bytes);k+=rbc_fields[i].bytes;}}
static u32 rbc_match(u32 a,u32 p){u32 i,k=RBC_HEADER;for(i=0;i<_gz_rbc_field_count();i++){
 if(!btr_equal((const void*)(a+rbc_fields[i].offset),(const void*)(p+k),rbc_fields[i].bytes))return 0;k+=rbc_fields[i].bytes;}return 1;}
u32 _gz_rbc_apply(const KbjPlan*j,const RbcPlan*p){u32 z=_gz_rbc_validate(j,p);if(z)return z;rbc_write(p->process.object,(u32)p->target);return 0;}
u32 _gz_rbc_matches(const KbjPlan*j,const RbcPlan*p){u32 z=_gz_rbc_validate(j,p);return z?z:rbc_match(p->process.object,(u32)p->target)?0:RBC_CHANGED;}
u32 _gz_rbc_undo(const KbjPlan*j,const RbcPlan*p){u32 z=_gz_rbc_validate(j,p);if(z)return z;rbc_write(p->process.object,(u32)p->before);return 0;}
u32 _gz_rbc_before_matches(const KbjPlan*j,const RbcPlan*p){u32 z=_gz_rbc_validate(j,p);return z?z:rbc_match(p->process.object,(u32)p->before)?0:RBC_CHANGED;}
#undef RBC_W

#undef RBC_ACTION_TABLE_HASH
