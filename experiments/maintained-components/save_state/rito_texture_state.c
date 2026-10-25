/* RTP v2 preserves the eight unused bits-payload bytes of an invalid
 * (+infinity-marked) cache entry. Native profile329 has nonzero stale data
 * there, so v1 zero-normalization broke capture's own exact comparison.
 * Active entries keep all finite/range checks. This is bounded owned cache
 * data, never pointer/addressing input; v1's canonical zeros remain required.
 */
/* Pointer-free Rito texture control/result/cache state. Shares the owned
 * FTXP step-integer format contract, not Sue-Belle's actor/pot identity.
 * Current pattern selection must match; resource-changing replay remains
 * a separate native setter/ownership participant. No ordinary admission.
 */
enum {RTP_BYTES=256,RTP_BAD=1460,RTP_CHANGED=1461,RTP_ALIAS=1462};
typedef struct {EpjLayout control;EptResources resources;EptAnimation animation;u32 model_index,pattern_index;} RtpLayout;
typedef char RtpLayoutSize[sizeof(RtpLayout)==160?1:-1];
static u32 rtp_hash(u32 p){u32 i,h=2166136261u;for(i=0;i<RTP_BYTES;i++)h=(h^((i>=12&&i<16)||(i>=60&&i<64)?0:R8(p,i)))*16777619u;return h;}
static u32 rtp_packet(u32 p,u32 published){u32 i,c,v;
 if(!epj_descriptor(p,RTP_BYTES)||R32(p,0)!=0x52545031u||R32(p,4)!=RTP_BYTES||(R32(p,8)!=1&&R32(p,8)!=2)||R32(p,12)!=published||
  !R32(p,16)||R32(p,16)>=0xfffffffeu||!R32(p,20)||!R32(p,24)||!rtf_profile(R32(p,28))||R32(p,44)>65535||
  R32(p,48)>255||R32(p,52)>255||R32(p,56)>255||R32(p,60)!=rtp_hash(p)||R32(p,64)!=0x52545031u||R32(p,68)!=63||
  R32(p,72)!=4||R32(p,76)!=2||R32(p,80)!=1||R32(p,84)!=1||!R32(p,88)||R32(p,88)>32767||
  !R32(p,92)||R32(p,92)>4096||R32(p,96)>4||(R32(p,100)!=1&&R32(p,100)!=2)||!epj_finite(R32(p,104))||
  R32(p,108)>255||R32(p,112)>255||R32(p,116)>65535||R32(p,120)>1||!R32(p,124)||R32(p,124)>4096||
  R32(p,32)>=R32(p,124)||R32(p,36)>=R32(p,124))return RTP_BAD;
 if(!epj_finite(R32(p,128))||!epj_finite(R32(p,132))||R16(p,136)>R16(p,138)||R16(p,138)>R32(p,88)||
  R16(p,140)>R16(p,138)||R8(p,142)>4||R8(p,143)>3)return RTP_BAD;
 for(i=144;i<160;i+=4)if(!epj_finite(R32(p,i)))return RTP_BAD;
 if(R32(p,160)!=R32(p,96)||!nps_bound(R32(p,164))||R16(p,170)!=2||R16(p,172)>1||R16(p,174)!=2||
  R16(p,176)>=R32(p,92)||R16(p,178)||R8(p,188)>=2||R8(p,189)||R8(p,190)||R8(p,191))return RTP_BAD;
 for(i=180;i<188;i+=4){v=R32(p,i);if(v!=0xffffffffu&&v>=1)return RTP_BAD;}
 for(i=0;i<4;i++){
  c=p+192+i*12;if(i>=R32(p,96)){if(R32(c,0)||R32(c,4)||R32(c,8))return RTP_BAD;continue;}
  if(R32(c,0)==0x7f800000u){if(R32(p,8)==1&&(R32(c,4)||R32(c,8)))return RTP_BAD;}
  else if(!epj_finite(R32(c,0))||!epj_finite(R32(c,4))||(R32(c,0)&0x80000000u)||R32(c,4)<=R32(c,0)||R32(c,8)>=4096)return RTP_BAD;
 }return rbs_zero(p,240,RTP_BYTES)?0:RTP_BAD;
}
u32 _gz_rtp_check(u32 p){return rtp_packet(p,1);}
static u32 rtp_layout_mode(const KbjLayout*base,RtpLayout*out,const KbjPlan*const*known){EpjBounds b;EptResources r;RtpLayout q;RmsLayout models;u32 z,a=base->common.bounds.actor,o;
 z=rms_layout_mode(base,&models,known);if(z)return z;btr_copy(&b,&base->common.bounds,sizeof(b));btr_copy(&r,&base->resources,sizeof(r));
 b.morph=R32(a,0x44c);b.model=models.models[1].model;b.face=a+0x808;o=b.face;
 b.before_wrapper=R32(o,0x10);b.target_wrapper=b.before_wrapper;
 if(R32(a,0x804)!=b.before_wrapper)return RTP_CHANGED;r.model_wrapper=models.models[1].wrapper;r.model=R32(r.model_wrapper,0);
 eks_zero(&q,sizeof(q));z=npj_layout_mode(&b,&r,&q.control,b.source?0:1);if(z)return z;
 if(q.control.buffer_bytes!=64||q.control.cache_capacity!=4||q.control.materials!=2||q.control.animation_capacity!=1||q.control.result_capacity!=1||
  q.control.binding!=q.control.buffer+4||q.control.mapping!=q.control.buffer+12||q.control.cache!=q.control.buffer+16)return RTP_CHANGED;
 if(!rbs_index(&r,r.model_wrapper,&q.model_index)||!rbs_index(&r,b.before_wrapper,&q.pattern_index))return RTP_CHANGED;
 z=ept_animation(&r,&q.control,b.before_wrapper,&q.animation);if(z)return z;
 if(q.animation.cache>4||q.animation.materials!=1||q.animation.results!=1)return RTP_CHANGED;
 if(R32(r.model_wrapper,0x38)&&R32(r.model_wrapper,0x38)!=o+0x1c)return RTP_CHANGED;
 btr_copy(&q.resources,&r,sizeof(r));btr_copy(out,&q,sizeof(q));return 0;
}
static u32 rtp_layout(const KbjLayout*base,RtpLayout*out){return rtp_layout_mode(base,out,0);}
static u32 rtp_join(const RtpLayout*l,const PoEntry*process,u32 p,u32 published){u32 z=rtp_packet(p,published),a=l->control.bounds.actor,w=l->control.bounds.face+0x1c;
 if(z)return z;
 if(R32(p,28)!=process->profile||!rri_identity(process,p)||R32(p,32)!=l->model_index||R32(p,36)!=l->pattern_index||
  R32(p,40)!=R32(a,0xb0)||R32(p,44)!=R16(a,0x2d8)||R32(p,48)!=R8(a,0xa75)||R32(p,52)!=R8(a,0xa76)||R32(p,56)!=R8(a,0xa77)||
  R32(p,88)!=l->animation.max_frame||R32(p,92)!=l->animation.textures||R32(p,96)!=l->animation.cache||
  R32(p,100)!=(R32(w,0x14)==(u32)_gz_face_native_frame_clamp?1u:2u)||R32(p,124)!=l->resources.count)return RTP_CHANGED;
 return nps_caches(&l->resources,&l->animation,p);
}
static u32 rtp_encode(const RtpLayout*l,const PoEntry*process,u32 frame,u32 p,u32 published){u32 a=l->control.bounds.actor,o=l->control.bounds.face,w=o+0x1c,i,c;
 eks_zero((void*)p,RTP_BYTES);*(u32*)(p+0)=0x52545031u;*(u32*)(p+4)=RTP_BYTES;*(u32*)(p+8)=2;*(u32*)(p+12)=published;
 *(u32*)(p+16)=process->pid;*(u32*)(p+20)=process->generation;*(u32*)(p+24)=frame;*(u32*)(p+28)=process->profile;
 *(u32*)(p+32)=l->model_index;*(u32*)(p+36)=l->pattern_index;*(u32*)(p+40)=R32(a,0xb0);*(u32*)(p+44)=R16(a,0x2d8);
 *(u32*)(p+48)=R8(a,0xa75);*(u32*)(p+52)=R8(a,0xa76);*(u32*)(p+56)=R8(a,0xa77);*(u32*)(p+64)=0x52545031u;*(u32*)(p+68)=63;
 *(u32*)(p+72)=4;*(u32*)(p+76)=2;*(u32*)(p+80)=1;*(u32*)(p+84)=1;*(u32*)(p+88)=l->animation.max_frame;
 *(u32*)(p+92)=l->animation.textures;*(u32*)(p+96)=l->animation.cache;*(u32*)(p+100)=R32(w,0x14)==(u32)_gz_face_native_frame_clamp?1:2;
 *(u32*)(p+104)=R32(l->control.bounds.before_wrapper,0);*(u32*)(p+108)=R8(a,0x87c);*(u32*)(p+112)=R8(a,0xa6d);
 *(u32*)(p+116)=R16(a,0x87e);*(u32*)(p+120)=R32(l->resources.model_wrapper,0x38)?1:0;*(u32*)(p+124)=l->resources.count;
 btr_copy((void*)(p+128),(const void*)o,16);btr_copy((void*)(p+144),(const void*)(w+4),16);
 *(u32*)(p+160)=R32(w,0x24);*(u32*)(p+164)=R32(w,0x28);*(u32*)(p+168)=R32(w,0x3c);*(u32*)(p+172)=R32(w,0x40);
 *(u16*)(p+176)=R16(l->control.results,0);btr_copy((void*)(p+180),(const void*)l->control.binding,8);*(u8*)(p+188)=R8(l->control.mapping,0);
 for(i=0;i<l->animation.cache;i++){c=l->control.cache+i*12;*(u32*)(p+192+i*12)=R32(c,0);btr_copy((void*)(p+196+i*12),(const void*)(c+4),8);}
 *(u32*)(p+60)=rtp_hash(p);return rtp_join(l,process,p,published);
}
static u32 rtp_matches(const RtpLayout*l,u32 p){u32 o=l->control.bounds.face,w=o+0x1c,a=l->control.bounds.actor,i;
 if(!btr_equal((const void*)o,(const void*)(p+128),16)||!btr_equal((const void*)(w+4),(const void*)(p+144),16)||
  R32(w,0x24)!=R32(p,160)||R32(w,0x28)!=R32(p,164)||R32(w,0x3c)!=R32(p,168)||R32(w,0x40)!=R32(p,172)||
  R16(l->control.results,0)!=R16(p,176)||!btr_equal((const void*)l->control.binding,(const void*)(p+180),8)||R8(l->control.mapping,0)!=R8(p,188)||
  R32(l->control.bounds.before_wrapper,0)!=R32(p,104)||R8(a,0x87c)!=R32(p,108)||R8(a,0xa6d)!=R32(p,112)||R16(a,0x87e)!=R32(p,116)||
  R32(l->resources.model_wrapper,0x38)!=(R32(p,120)?o+0x1c:0))return RTP_CHANGED;
 for(i=0;i<l->animation.cache;i++)if(!btr_equal((const void*)(l->control.cache+i*12),(const void*)(p+192+i*12),12))return RTP_CHANGED;return 0;
}
u32 _gz_rtp_capture(const KbjPlan*j,u32 frame,u32 out){RtpLayout l,again;PoEntry first,last;u32 z;
 if(frame!=_gz_pbac_frame)return RTP_BAD;z=rbs_current(j,1,frame,&first);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;
 if(!rbs_output(j,out,RTP_BYTES)||!epj_apart(out,RTP_BYTES,(u32)&l,sizeof(l))||!epj_apart(out,RTP_BYTES,(u32)&again,sizeof(again))||
  !epj_apart(out,RTP_BYTES,(u32)&first,sizeof(first))||!epj_apart(out,RTP_BYTES,(u32)&last,sizeof(last)))return RTP_ALIAS;
 z=rtp_layout(&j->layout,&l);if(z)return z;z=rtp_encode(&l,&first,frame,out,0);if(z)return z;
 z=_gz_kbj_before_matches(j);if(z)return z;z=rbs_current(j,1,frame,&last);if(z)return z;z=rtp_layout(&j->layout,&again);if(z)return z;
 if(!btr_equal(&l,&again,sizeof(l))||!btr_equal(&first,&last,sizeof(first)))return RTP_CHANGED;z=rtp_matches(&l,out);if(z)return z;
 __asm__ volatile("sync":::"memory");*(volatile u32*)(out+12)=1;return 0;
}
typedef struct {u32 magic,bytes,valid,frame;PoEntry process;RtpLayout layout;u8 object_before[116],buffer_before[64];
 u32 model_attachment,wrapper_frame,blink_frame,blink_timer,btp_num;u8 target[RTP_BYTES];u32 seal;} RtjPlan;
typedef char RtjPlanSize[sizeof(RtjPlan)==700?1:-1];
static u32 rtj_hash(const RtjPlan*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
u32 _gz_rtj_prepare(const KbjPlan*j,u32 source,u32 frame,RtjPlan*out){PoEntry first,last;RtpLayout again;const EpjBounds*b;u32 z,h;
 if(frame!=_gz_pbac_frame)return RTP_BAD;z=rbs_current(j,1,frame,&first);if(z)return z;z=_gz_kbj_before_matches(j);if(z)return z;b=&j->layout.common.bounds;
 if(!epj_ram(source,RTP_BYTES)||source<b->source||source-b->source>b->source_bytes||RTP_BYTES>b->source_bytes-(source-b->source))return RTP_ALIAS;
 z=_gz_rtp_check(source);if(z)return z;if(!out||!rbs_output(j,(u32)out,sizeof(*out)))return RTP_ALIAS;
 h=btr_hash((const void*)source,RTP_BYTES);eks_zero(out,sizeof(*out));btr_copy(out->target,(const void*)source,RTP_BYTES);
 if(h!=btr_hash(out->target,RTP_BYTES)||h!=btr_hash((const void*)source,RTP_BYTES))return RTP_CHANGED;
 z=rtp_layout(&j->layout,&out->layout);if(z)return z;z=rtp_join(&out->layout,&first,(u32)out->target,1);if(z)return z;
 btr_copy(out->object_before,(const void*)out->layout.control.bounds.face,116);btr_copy(out->buffer_before,(const void*)out->layout.control.buffer,64);
 out->model_attachment=R32(out->layout.resources.model_wrapper,0x38);out->wrapper_frame=R32(out->layout.control.bounds.before_wrapper,0);
 out->blink_frame=R8(b->actor,0x87c);out->blink_timer=R16(b->actor,0x87e);out->btp_num=R8(b->actor,0xa6d);
 z=_gz_kbj_before_matches(j);if(z)return z;z=rbs_current(j,1,frame,&last);if(z)return z;z=rtp_layout(&j->layout,&again);if(z)return z;
 if(!btr_equal(&first,&last,sizeof(first))||!btr_equal(&again,&out->layout,sizeof(again))||
  !btr_equal(out->object_before,(const void*)again.control.bounds.face,116)||!btr_equal(out->buffer_before,(const void*)again.control.buffer,64)||
  out->model_attachment!=R32(again.resources.model_wrapper,0x38)||out->wrapper_frame!=R32(again.control.bounds.before_wrapper,0)||
  out->blink_frame!=R8(b->actor,0x87c)||out->blink_timer!=R16(b->actor,0x87e)||out->btp_num!=R8(b->actor,0xa6d))return RTP_CHANGED;
 out->magic=0x52544a31u;out->bytes=sizeof(*out);out->frame=frame;btr_copy(&out->process,&first,sizeof(first));out->seal=rtj_hash(out);
 __asm__ volatile("sync":::"memory");out->valid=1;return 0;
}
static u32 rtj_validate_mode(const KbjPlan*j,const RtjPlan*p,const KbjPlan*const*known){RtpLayout current;PoEntry process;u32 z;
 if(!p||!epj_ram((u32)p,sizeof(*p))||p->magic!=0x52544a31u||p->bytes!=sizeof(*p)||p->valid!=1||p->seal!=rtj_hash(p))return RTP_BAD;
 z=rbs_current(j,1,p->frame,&process);if(z)return z;if(!rbs_output(j,(u32)p,sizeof(*p))||!btr_equal(&process,&p->process,sizeof(process)))return RTP_CHANGED;
 z=rtp_layout_mode(&j->layout,&current,known);if(z)return z;if(!btr_equal(&current,&p->layout,sizeof(current)))return RTP_CHANGED;
 return rtp_join(&current,&process,(u32)p->target,1);
}
u32 _gz_rtj_validate(const KbjPlan*j,const RtjPlan*p){return rtj_validate_mode(j,p,0);}
u32 _gz_rtj_validate_group(const KbjPlan*j,const RtjPlan*p,const KbjPlan*const*known){return known?rtj_validate_mode(j,p,known):RTP_BAD;}
u32 _gz_rtj_apply(const KbjPlan*j,const RtjPlan*p){u32 z=_gz_rtj_validate(j,p),source=(u32)p->target,o,w,a,i;const RtpLayout*l;if(z)return z;l=&p->layout;o=l->control.bounds.face;w=o+0x1c;a=l->control.bounds.actor;
 btr_copy((void*)o,(const void*)(source+128),16);btr_copy((void*)(w+4),(const void*)(source+144),16);
 *(volatile u32*)(w+0x24)=R32(source,160);*(volatile u32*)(w+0x28)=R32(source,164);*(volatile u32*)(w+0x3c)=R32(source,168);*(volatile u32*)(w+0x40)=R32(source,172);
 *(volatile u16*)l->control.results=R16(source,176);btr_copy((void*)l->control.binding,(const void*)(source+180),8);*(volatile u8*)l->control.mapping=R8(source,188);
 for(i=0;i<l->animation.cache;i++)btr_copy((void*)(l->control.cache+i*12),(const void*)(source+192+i*12),12);
 *(volatile u32*)l->control.bounds.before_wrapper=R32(source,104);*(volatile u8*)(a+0x87c)=(u8)R32(source,108);*(volatile u8*)(a+0xa6d)=(u8)R32(source,112);
 *(volatile u16*)(a+0x87e)=(u16)R32(source,116);*(volatile u32*)(l->resources.model_wrapper+0x38)=R32(source,120)?o+0x1c:0;return 0;
}
static u32 rtj_undo_mode(const KbjPlan*j,const RtjPlan*p,const KbjPlan*const*known){const RtpLayout*l;u32 z=rtj_validate_mode(j,p,known),a;if(z)return z;l=&p->layout;a=l->control.bounds.actor;
 btr_copy((void*)l->control.bounds.face,p->object_before,116);btr_copy((void*)l->control.buffer,p->buffer_before,64);
 *(volatile u32*)(l->resources.model_wrapper+0x38)=p->model_attachment;*(volatile u32*)l->control.bounds.before_wrapper=p->wrapper_frame;
 *(volatile u8*)(a+0x87c)=(u8)p->blink_frame;*(volatile u16*)(a+0x87e)=(u16)p->blink_timer;*(volatile u8*)(a+0xa6d)=(u8)p->btp_num;return 0;
}
u32 _gz_rtj_undo(const KbjPlan*j,const RtjPlan*p){return rtj_undo_mode(j,p,0);}
u32 _gz_rtj_undo_group(const KbjPlan*j,const RtjPlan*p,const KbjPlan*const*known){return known?rtj_undo_mode(j,p,known):RTP_BAD;}
u32 _gz_rtj_matches(const KbjPlan*j,const RtjPlan*p){u32 z=_gz_rtj_validate(j,p);return z?z:rtp_matches(&p->layout,(u32)p->target);}
u32 _gz_rtj_before_matches(const KbjPlan*j,const RtjPlan*p){const RtpLayout*l;u32 z=_gz_rtj_validate(j,p),a;if(z)return z;l=&p->layout;a=l->control.bounds.actor;
 return btr_equal((const void*)l->control.bounds.face,p->object_before,116)&&btr_equal((const void*)l->control.buffer,p->buffer_before,64)&&
  R32(l->resources.model_wrapper,0x38)==p->model_attachment&&R32(l->control.bounds.before_wrapper,0)==p->wrapper_frame&&
  R8(a,0x87c)==p->blink_frame&&R16(a,0x87e)==p->blink_timer&&R8(a,0xa6d)==p->btp_num?0:RTP_CHANGED;
}
