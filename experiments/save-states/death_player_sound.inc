/* Sound-animation references omitted by LPC's numeric map. Reconstruct from
 * the CURRENT resident LkAnm BFRES and the captured track selectors. Native
 * reset23E07C4 supplies the canonical empty string; no saved pointer is used.
 * This changes neither sound-manager handles nor music/audio ownership. */
extern const u8 _gz_dls_native_empty[];
static u32 fpc_archive(u32,LpcArc*,u32*,u32*,u32*);
typedef struct {
 u32 magic,bytes,valid,token,frame,actor,pid,body_seal;
 u32 container,header,extent,before_index,before_track,saved_index,saved_track;
 u32 before_data,before_frame,saved_data,saved_frame,reserved;
} DlsPlan;
typedef char DlsPlanSize[sizeof(DlsPlan)==80?1:-1];
static DlsPlan _gz_dls_plan;
static u32 _gz_dls_seal,_gz_dls_active;
u32 _gz_dls_report[24]={0x444c5331u,96};
/* Address arithmetic is bounded to the currently certified BFRES file before
 * any data load. The native relative sound offset may point in either direction. */
static u32 dls_data(u32 id,u32*out){const DlsPlan*d=&_gz_dls_plan;u32 p,q,rel,data,z;
 z=lpc_resource(&_gz_lpc_plan.archive,id,&p);if(z||!p)return LPC_RESOURCE;
 q=R32(p,8);if(q<d->header||q-d->header>d->extent-8)return LPC_RESOURCE;
 rel=R32(q,4);if(!rel)return LPC_RESOURCE;data=q+4+rel;
 if(data<d->header||data-d->header>=d->extent)return LPC_RESOURCE;
 *out=data;return 0;
}
static u32 dls_root(u32 capture){DlsPlan*d=&_gz_dls_plan;LpcArc a;u32 c,h,n,z;
 z=fpc_archive(237211244u,&a,&c,&h,&n);if(z)return z;
 if(!btr_equal(&a,&_gz_lpc_plan.archive,sizeof(a)))return LPC_RESOURCE;
 if(capture){d->container=c;d->header=h;d->extent=n;}
 else if(c!=d->container||h!=d->header||n!=d->extent)return LPC_RESOURCE;
 return 0;
}
static u32 dls_targets(u32 saved,u32*data,u32*frame){const DlsPlan*d=&_gz_dls_plan;const LpcPlan*p=&_gz_lpc_plan;
 const u8*s=saved?p->saved:p->before;const u32*ids=saved?p->saved_bind:p->before_bind;
 u32 i=saved?d->saved_index:d->before_index,t=saved?d->saved_track:d->before_track,v;
 if(!lpc_value(s,0x6940,&v))return LPC_SOURCE;
 if(i==BAD){
  if(t!=BAD||(v>>16)!=65535)return LPC_SOURCE;
  *data=(u32)_gz_dls_native_empty;*frame=0;return 0;
 }
 if(i>=5||t>=5||(v>>16)!=R16(s,134+t*200))return LPC_SOURCE;
 if(dls_data(ids[t],data))return LPC_RESOURCE;
 *frame=p->actor+0x5898+i*16;return 0;
}
static u32 dls_prepare(void){DlsPlan*d=&_gz_dls_plan;LpcPlan*p=&_gz_lpc_plan;u32 i,z,data,frame,a=p->actor;
 dlr_zero(d,sizeof(*d));_gz_dls_seal=_gz_dls_active=0;
 d->magic=0x444c5032u;d->bytes=sizeof(*d);d->token=p->token;d->frame=p->frame;d->actor=a;d->pid=p->pid;d->body_seal=_gz_lpc_seal;
 z=dls_root(1);if(z)return z;
 d->before_data=R32(a,0x6938);d->before_frame=R32(a,0x6a94);d->before_index=d->before_track=BAD;
 if(d->before_frame){
  if(d->before_frame<a+0x5898||d->before_frame>=a+0x58e8||((d->before_frame-a-0x5898)&15))return LPC_RESOURCE;
  d->before_index=(d->before_frame-a-0x5898)>>4;
  for(i=0;i<5;i++)if(!dls_data(p->before_bind[i],&data)&&data==d->before_data&&R16(a,0x584e + i*16)==R16(a,0x6940)){
   d->before_track=i;break;
  }
  if(d->before_track==BAD)return LPC_RESOURCE;
 }
 z=dls_targets(0,&data,&frame);if(z||data!=d->before_data||frame!=d->before_frame)return LPC_RESOURCE;
 d->saved_index=R32(p->saved,112);d->saved_track=R32(p->saved,116);
 z=dls_targets(1,&d->saved_data,&d->saved_frame);if(z)return z;
 if(dls_root(0)||R32(a,0x693c)!=(u32)_gz_lpc_native_sound_string_vtable)return LPC_RESOURCE;
 d->valid=1;_gz_dls_seal=btr_hash(d,sizeof(*d));_gz_dls_active=p->token;
 _gz_dls_report[2]=p->token;_gz_dls_report[3]=p->frame;_gz_dls_report[4]=_gz_dls_seal;
 _gz_dls_report[5]=d->before_index;_gz_dls_report[6]=d->before_track;_gz_dls_report[7]=d->saved_index;_gz_dls_report[8]=d->saved_track;
 _gz_dls_report[9]=d->before_data;_gz_dls_report[10]=d->before_frame;_gz_dls_report[11]=d->saved_data;_gz_dls_report[12]=d->saved_frame;
 return 0;
}
static u32 dls_bound(void){const DlsPlan*d=&_gz_dls_plan;const LpcPlan*p=&_gz_lpc_plan;u32 data,frame;
 if(!_gz_dls_active)return (_gz_dck_mode<5||_gz_dck_mode>7);
 if((_gz_dck_mode<5||_gz_dck_mode>7)||d->magic!=0x444c5032u||d->bytes!=sizeof(*d)||d->valid!=1||d->reserved||
  d->token!=_gz_dls_active||d->token!=p->token||d->frame!=p->frame||d->actor!=p->actor||d->pid!=p->pid||d->body_seal!=_gz_lpc_seal||
  _gz_dls_seal!=btr_hash(d,sizeof(*d))||dls_root(0)||
  R32(p->actor,0x693c)!=(u32)_gz_lpc_native_sound_string_vtable)return 0;
 if(dls_targets(0,&data,&frame)||data!=d->before_data||frame!=d->before_frame)return 0;
 if(dls_targets(1,&data,&frame)||data!=d->saved_data||frame!=d->saved_frame)return 0;
 return 1;
}
static u32 dls_match(u32 role){const DlsPlan*d=&_gz_dls_plan;u32 data,frame;
 if(!_gz_dls_active)return (_gz_dck_mode<5||_gz_dck_mode>7);
 if(!dls_bound())return 0;data=R32(d->actor,0x6938);frame=R32(d->actor,0x6a94);
 if(role==0)return data==d->before_data&&frame==d->before_frame;
 if(role==1)return data==d->saved_data&&frame==d->saved_frame;
 return (data==d->before_data||data==d->saved_data)&&(frame==d->before_frame||frame==d->saved_frame);
}
static u32 dls_values(u32 saved){const DlsPlan*d=&_gz_dls_plan;
 if(!_gz_dls_active)return (_gz_dck_mode>=5&&_gz_dck_mode<=7)?LPC_RESOURCE:0;
 if(!dls_bound()||!dls_match(2))return LPC_RESOURCE;
 *(volatile u32*)(d->actor+0x6938)=saved?d->saved_data:d->before_data;
 *(volatile u32*)(d->actor+0x6a94)=saved?d->saved_frame:d->before_frame;
 if(!dls_match(saved))return LPC_CHANGED;
 _gz_dls_report[saved?13:14]++;return 0;
}
