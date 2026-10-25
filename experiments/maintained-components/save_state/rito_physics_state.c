/* Common fopNpc Acch/query/cylinder numeric fields, with Rito identity.
 * Background owners/resources are proved through the current registry.
 * Saved reference words are equality keys; none is followed or restored.
 * Global contacts/broadphase and physical BG lifetime remain outer domains.
 */
enum {RPH_BYTES=832,RPH_HEADER=128,RPH_META=NPH_META+48,RPH_USED=NPH_USED+48,
 RPH_BAD=1480,RPH_CHANGED=1481,RPH_ALIAS=1482};
static u32 rph_data(void){return RPH_HEADER+4*nph_ref_count();}
static u32 rph_hash(u32 p){u32 i,h=2166136261u;for(i=0;i<RPH_BYTES;i++)h=(h^((i>=12&&i<16)||(i>=80&&i<84)?0:R8(p,i)))*16777619u;return h;}
static u32 rph_value(u32 p,u32 off){u32 i,j,k=rph_data(),v;for(i=0;i<nph_count();i++){
 if(nph_fields[i].offset==off){v=0;for(j=0;j<nph_fields[i].bytes;j++)v=(v<<8)|R8(p,k+j);return v;}k+=nph_fields[i].bytes;}return 0;}
static u32 rph_numbers(u32 p){u32 i,j,k=rph_data(),v;for(i=0;i<nph_count();i++){
 if(nph_fields[i].floats)for(j=0;j<nph_fields[i].bytes;j+=4){v=((u32)R8(p,k+j)<<24)|((u32)R8(p,k+j+1)<<16)|((u32)R8(p,k+j+2)<<8)|R8(p,k+j+3);
  if(!epj_finite(v)&&!(nph_fields[i].floats==2&&v==0xff800000u))return 0;}k+=nph_fields[i].bytes;}return 1;}
static u32 rph_packet(u32 p,u32 published){u32 i,w,q;
 if(!epj_descriptor(p,RPH_BYTES)||rph_data()+nph_packed()>RPH_META||RPH_USED>RPH_BYTES||
  R32(p,0)!=0x52504831u||R32(p,4)!=RPH_BYTES||R32(p,8)!=1||R32(p,12)!=published||!R32(p,16)||R32(p,16)>=0xfffffffeu||!R32(p,20)||!R32(p,24)||!rtf_profile(R32(p,28))||
  R32(p,36)>65535||R32(p,40)>255||R32(p,44)>255||R32(p,48)>255||R32(p,52)!=nph_count()||R32(p,56)!=nph_packed()||
  R32(p,60)!=nph_ref_count()||R32(p,64)!=nph_layout()||R32(p,68)!=btr_hash((const void*)(p+RPH_HEADER),4*nph_ref_count())||
  R32(p,72)!=63||R32(p,76)!=RPH_USED||R32(p,80)!=rph_hash(p)||!rbs_zero(p,84,RPH_HEADER)||!rph_numbers(p))return RPH_BAD;
 for(i=0;i<4;i++){q=p+RPH_META+i*16;w=R32(q,0);if((w>>16)!=rph_value(p,nph_poly[i]))return RPH_BAD;
  if(w==0xffff0100u){if(R32(q,4)||R32(q,8)||R32(q,12))return RPH_BAD;}
  else if((w&65535)>=256||(w>>16)==65535||!R32(q,4)||!R32(q,8))return RPH_BAD;}
 return rbs_zero(p,rph_data()+nph_packed(),RPH_META)&&rbs_zero(p,RPH_USED,RPH_BYTES)?0:RPH_BAD;
}
u32 _gz_rph_check(u32 p){return rph_packet(p,1);}
static u32 rph_bg_match(u32 a,u32 p){u32 i,w,live[4],index,bgd;
 for(i=0;i<4;i++){
  w=R32(p,RPH_META+i*16);index=w&65535;if(nph_bg_sample(a,nph_poly[i],live))return 0;
  if(w==0xffff0100u){if(live[0]!=w)return 0;continue;}
  if((live[0]&65535)!=index||live[1]!=R32(p,RPH_META+i*16+4)||live[2]!=R32(p,RPH_META+i*16+8)||live[3]!=R32(p,RPH_META+i*16+12))return 0;
  bgd=_gz_bg_observation.entry.resource;if((w>>16)>=R32(bgd,8))return 0;
 }return 1;
}
static u32 rph_refs_match(u32 a,u32 p){u32 i;for(i=0;i<nph_ref_count();i++)if(R32(a,nph_refs[i])!=R32(p,RPH_HEADER+4*i))return 0;return rph_bg_match(a,p);}
static u32 rph_join(const PoEntry*process,u32 p,u32 published){u32 a=process->object,z=rph_packet(p,published);if(z)return z;
 if(R32(p,28)!=process->profile||!rri_identity(process,p)||R32(p,32)!=R32(a,0xb0)||R32(p,36)!=R16(a,0x2d8)||
  R32(p,40)!=R8(a,0xa75)||R32(p,44)!=R8(a,0xa76)||R32(p,48)!=R8(a,0xa77)||!rph_refs_match(a,p))return RPH_CHANGED;return 0;}
static u32 rph_encode(const PoEntry*process,u32 frame,u32 p,u32 published){u32 a=process->object,i,k=rph_data(),tuple[4],z;
 if(k+nph_packed()>RPH_META)return RPH_BAD;eks_zero((void*)p,RPH_BYTES);
 *(u32*)(p+0)=0x52504831u;*(u32*)(p+4)=RPH_BYTES;*(u32*)(p+8)=1;*(u32*)(p+12)=published;
 *(u32*)(p+16)=process->pid;*(u32*)(p+20)=process->generation;*(u32*)(p+24)=frame;*(u32*)(p+28)=process->profile;
 *(u32*)(p+32)=R32(a,0xb0);*(u32*)(p+36)=R16(a,0x2d8);*(u32*)(p+40)=R8(a,0xa75);*(u32*)(p+44)=R8(a,0xa76);*(u32*)(p+48)=R8(a,0xa77);
 *(u32*)(p+52)=nph_count();*(u32*)(p+56)=nph_packed();*(u32*)(p+60)=nph_ref_count();*(u32*)(p+64)=nph_layout();*(u32*)(p+72)=63;*(u32*)(p+76)=RPH_USED;
 for(i=0;i<nph_ref_count();i++)*(u32*)(p+RPH_HEADER+i*4)=R32(a,nph_refs[i]);
 for(i=0;i<nph_count();i++){btr_copy((void*)(p+k),(const void*)(a+nph_fields[i].offset),nph_fields[i].bytes);k+=nph_fields[i].bytes;}
 for(i=0;i<4;i++){z=nph_bg_sample(a,nph_poly[i],tuple);if(z)return z;btr_copy((void*)(p+RPH_META+i*16),tuple,16);}
 *(u32*)(p+68)=btr_hash((const void*)(p+RPH_HEADER),4*nph_ref_count());*(u32*)(p+80)=rph_hash(p);return rph_join(process,p,published);
}
static u32 rph_values(u32 a,u32 p,u32 write){u32 i,j,n,o,k=rph_data(),v;for(i=0;i<nph_count();i++){
 n=nph_fields[i].bytes;o=nph_fields[i].offset;
 if(!write){if(!btr_equal((const void*)(a+o),(const void*)(p+k),n))return RPH_CHANGED;}
 else if(n==2&&!(o&1)){v=((u32)R8(p,k)<<8)|R8(p,k+1);*(volatile u16*)(a+o)=(u16)v;}
 else if(n==4&&!(o&3)){v=0;for(j=0;j<4;j++)v=(v<<8)|R8(p,k+j);*(volatile u32*)(a+o)=v;}
 else btr_copy((void*)(a+o),(const void*)(p+k),n);
 k+=n;
 }return 0;
}
u32 _gz_rph_capture(const KbjPlan*j,u32 frame,u32 out){PoEntry first,last;u32 z;
 if(frame!=_gz_pbac_frame)return RPH_BAD;z=rbs_current(j,1,frame,&first);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;
 if(!rbs_output(j,out,RPH_BYTES)||!_gz_bg_private(out,RPH_BYTES)||!epj_apart(out,RPH_BYTES,(u32)&first,sizeof(first))||!epj_apart(out,RPH_BYTES,(u32)&last,sizeof(last)))return RPH_ALIAS;
 z=rph_encode(&first,frame,out,0);if(z)return z;z=rbs_current(j,1,frame,&last);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;
 if(!btr_equal(&first,&last,sizeof(first))||rph_values(first.object,out,0)||!rph_refs_match(first.object,out))return RPH_CHANGED;
 __asm__ volatile("sync":::"memory");*(volatile u32*)(out+12)=1;return 0;
}
typedef struct {u32 magic,bytes,valid,frame;PoEntry process;u32 before_hash,target_hash;u8 before[RPH_BYTES],target[RPH_BYTES];u32 seal;} RpjPlan;
typedef char RpjPlanSize[sizeof(RpjPlan)==1756?1:-1];
static u32 rpj_hash(const RpjPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
u32 _gz_rpj_prepare(const KbjPlan*j,u32 source,u32 frame,RpjPlan*out){PoEntry first,last;const EpjBounds*b;u32 z,h;
 if(frame!=_gz_pbac_frame)return RPH_BAD;z=rbs_current(j,1,frame,&first);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;b=&j->layout.common.bounds;
 if(!epj_ram(source,RPH_BYTES)||source<b->source||source-b->source>b->source_bytes||RPH_BYTES>b->source_bytes-(source-b->source))return RPH_ALIAS;
 z=rph_join(&first,source,1);if(z)return z;
 if(!out||!rbs_output(j,(u32)out,sizeof(*out))||!_gz_bg_private((u32)out,sizeof(*out)))return RPH_ALIAS;
 h=btr_hash((const void*)source,RPH_BYTES);eks_zero(out,sizeof(*out));btr_copy(out->target,(const void*)source,RPH_BYTES);
 if(h!=btr_hash(out->target,RPH_BYTES)||h!=btr_hash((const void*)source,RPH_BYTES))return RPH_CHANGED;
 z=rph_encode(&first,frame,(u32)out->before,1);if(z)return z;z=rbs_current(j,1,frame,&last);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;
 if(!btr_equal(&first,&last,sizeof(first))||rph_values(first.object,(u32)out->before,0)||!rph_refs_match(first.object,(u32)out->before))return RPH_CHANGED;
 out->magic=0x52504a31u;out->bytes=sizeof(*out);out->frame=frame;btr_copy(&out->process,&first,sizeof(first));out->before_hash=btr_hash(out->before,RPH_BYTES);out->target_hash=h;
 out->seal=rpj_hash(out);__asm__ volatile("sync":::"memory");out->valid=1;return 0;
}
u32 _gz_rpj_validate(const KbjPlan*j,const RpjPlan*p){PoEntry process;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x52504a31u||p->bytes!=sizeof(*p)||p->valid!=1||p->seal!=rpj_hash(p)||
  p->before_hash!=btr_hash(p->before,RPH_BYTES)||p->target_hash!=btr_hash(p->target,RPH_BYTES))return RPH_BAD;
 z=rbs_current(j,1,p->frame,&process);if(z)return z;
 if(!rbs_output(j,(u32)p,sizeof(*p))||!_gz_bg_private((u32)p,sizeof(*p))||!btr_equal(&process,&p->process,sizeof(process))||R32((u32)p->before,24)!=p->frame)return RPH_CHANGED;
 z=rph_join(&process,(u32)p->before,1);return z?z:rph_join(&process,(u32)p->target,1);
}
u32 _gz_rpj_apply(const KbjPlan*j,const RpjPlan*p){u32 z=_gz_rpj_validate(j,p);return z?z:rph_values(p->process.object,(u32)p->target,1);}
u32 _gz_rpj_undo(const KbjPlan*j,const RpjPlan*p){u32 z=_gz_rpj_validate(j,p);return z?z:rph_values(p->process.object,(u32)p->before,1);}
u32 _gz_rpj_matches(const KbjPlan*j,const RpjPlan*p){u32 z=_gz_rpj_validate(j,p);return z?z:rph_values(p->process.object,(u32)p->target,0);}
u32 _gz_rpj_before_matches(const KbjPlan*j,const RpjPlan*p){u32 z=_gz_rpj_validate(j,p);return z?z:rph_values(p->process.object,(u32)p->before,0);}
