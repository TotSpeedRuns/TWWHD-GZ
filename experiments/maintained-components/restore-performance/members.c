#include "members.h"
extern volatile u32 _gz_co_frame;
CmControl _gz_cm_control={CM_MAGIC,64,1};
CmMember _gz_cm_member_observation;
u32 _gz_cm_query_busy;
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static void cm_flag(u32 f){__sync_fetch_and_or(&_gz_cm_control.flags,f);}
static void co_flag(u32 f){__sync_fetch_and_or(&_gz_co_header.flags,f);}
static u32 cm_header(void){const CmControl*c=&_gz_cm_control;return c->magic==CM_MAGIC&&c->bytes==64&&c->version==1&&!c->session&&!c->reserved;}
static u32 co_header(void){const CoHeader*h=&_gz_co_header;return h->magic==CO_MAGIC&&h->bytes==64&&h->version==1&&h->capacity==CO_CAP&&h->rows==(u32)_gz_co_rows&&h->active<=CO_CAP;}
static void bump(u32*p){if(*p==0xffffffffu)cm_flag(CM_COUNTER_EXHAUSTED);else ++*p;}
static u32 is_active(u32 s){return s==CO_LIVE||s==CO_UNOBSERVED;}
static void copy(u32*d,const u32*s,u32 words){u32 i;for(i=0;i<words;i++)d[i]=s[i];}
static u32 same(const u32*a,const u32*b,u32 words){u32 i;for(i=0;i<words;i++)if(a[i]!=b[i])return 0;return 1;}
u32 _gz_cm_healthy(void){return cm_header()&&!_gz_cm_control.flags&&!_gz_cm_control.busy&&
 co_header()&&!_gz_co_header.flags&&!_gz_co_header.busy&&!_gz_co_header.session;}
u32 _gz_cm_invalidate(u32 p,u32 n,u32 reason){CmControl*c=&_gz_cm_control;CoHeader*h=&_gz_co_header;
 u32 i,active=0,count=0,frame,flag,first=CO_CAP,last=0;CoEntry*e;
 if(!cm_header()){cm_flag(CM_BAD_CO_HEADER);return CM_UNAVAILABLE;}
 if(!__sync_bool_compare_and_swap(&c->busy,0,1)){cm_flag(CM_REENTRY);return CM_UNAVAILABLE;}
 bump(&c->attempts);
 if(!span(p,n)||(reason!=CM_CREATED&&reason!=CM_ENDING)){cm_flag(CM_BAD_SPAN);goto done;}
 if(!co_header()){cm_flag(CM_BAD_CO_HEADER);co_flag(CO_BAD_HEADER);goto done;}
 if(!__sync_bool_compare_and_swap(&h->busy,0,1)){cm_flag(CM_CO_BUSY);co_flag(CO_REENTRY);goto done;}
 frame=_gz_co_frame;
 /* Validate the whole fixed private table before changing any row. Native
  * pointers are scalar addresses only and are never dereferenced here. */
 for(i=0;i<CO_CAP;i++){
  e=&_gz_co_rows[i];
  if(e->state==CO_EMPTY){if(e->object|e->generation)goto corrupt;continue;}
  if(e->state!=CO_LIVE&&e->state!=CO_DEAD&&e->state!=CO_UNOBSERVED)goto corrupt;
  if(!span(e->object,0x50)||e->generation>h->serial||
   (e->state==CO_LIVE&&!e->generation)||(e->state==CO_UNOBSERVED&&e->generation))goto corrupt;
  if(is_active(e->state)){
   ++active;
   /* Remember only scalar row bounds while validating. Every row and the
    * complete active count must still pass before any retirement write. */
   if(p<e->object+0x50&&e->object<p+n){if(first==CO_CAP)first=i;last=i;}
  }
 }
 if(active!=h->active)goto corrupt;
 flag=reason==CM_CREATED?CM_ROW_CREATED:CM_ROW_ENDING;
 /* No candidates leaves first==CO_CAP, so this pass is empty. Keep the
  * original predicate inside the bounds; nonmatching rows between them
  * remain unchanged under the same continuously held metadata locks. */
 for(i=first;i<=last;i++){
  e=&_gz_co_rows[i];
  if(is_active(e->state)&&p<e->object+0x50&&e->object<p+n){
   e->dead_frame=frame;e->dtor_site=0;e->flags|=flag;
   __sync_synchronize();e->state=CO_DEAD;++count;
  }
 }
 h->active-=count;c->process=p;c->allocation_bytes=n;c->reason=reason;c->frame=frame;
 c->count=count;c->co_serial=h->serial;bump(&c->commits);
 if(c->retired>0xffffffffu-count){c->retired=0xffffffffu;cm_flag(CM_COUNTER_EXHAUSTED);}else c->retired+=count;
 if(h->flags)cm_flag(CM_SOURCE_GAP);
 if(_gz_co_frame!=frame)cm_flag(CM_FRAME_CHANGED);
 __sync_synchronize();*(volatile u32*)&h->busy=0;
 __sync_synchronize();*(volatile u32*)&c->busy=0;
 return c->flags?CM_UNAVAILABLE:CM_OK;
corrupt:cm_flag(CM_BAD_ROW);co_flag(CO_BAD_HEADER);
 __sync_synchronize();*(volatile u32*)&h->busy=0;
done:__sync_synchronize();*(volatile u32*)&c->busy=0;return CM_UNAVAILABLE;
}
u32 _gz_cm_member(u32 p,u32 n){CmMember*out=&_gz_cm_member_observation;
 PoEntry process,again_process;CoEntry collider,again_collider;u32 ph[16],ch[16],i,frame,result=CM_UNAVAILABLE;
 out->valid=0;
 if(!__sync_bool_compare_and_swap(&_gz_cm_query_busy,0,1))return CM_UNAVAILABLE;
 for(i=1;i<sizeof(*out)/4;i++)((u32*)out)[i]=0;
 out->missing=CM_MISSING_ALL;
 if(!span(p,n)||n<0x50){result=CM_BAD_QUERY;goto done;}
 if(!_gz_cm_healthy())goto done;
 frame=_gz_co_frame;
 copy(ph,(const u32*)&_gz_po_header,16);copy(ch,(const u32*)&_gz_co_header,16);
 if(ph[5]||ph[6]||ph[15]||ch[5]||ch[6]||ch[15])goto done;
 if(_gz_po_owner_of(p,n)||!_gz_po_observation.valid||_gz_po_observation.flags||_gz_po_observation.session)goto done;
 copy((u32*)&process,(const u32*)&_gz_po_observation.entry,16);
 if(_gz_co_lookup(p)!=1||!_gz_co_observation.valid||_gz_co_observation.flags||_gz_co_observation.session)goto done;
 copy((u32*)&collider,(const u32*)&_gz_co_observation.entry,16);
 if(collider.state!=CO_LIVE||!collider.generation||(collider.flags&(CM_ROW_CREATED|CM_ROW_ENDING))){result=CM_NOT_LIVE;goto done;}
 if(collider.object!=p||process.state!=PO_LIVE||!process.generation||!span(process.object,process.allocation_bytes)||
  p<process.object||p-process.object>process.allocation_bytes||n>process.allocation_bytes-(p-process.object))goto changed;
 if(_gz_po_lookup(process.object)||!_gz_po_observation.valid)goto changed;
 copy((u32*)&again_process,(const u32*)&_gz_po_observation.entry,16);
 if(_gz_co_lookup(p)!=1||!_gz_co_observation.valid)goto changed;
 copy((u32*)&again_collider,(const u32*)&_gz_co_observation.entry,16);
 if(!same((const u32*)&process,(const u32*)&again_process,16)||!same((const u32*)&collider,(const u32*)&again_collider,16)||
  !same(ph,(const u32*)&_gz_po_header,16)||!same(ch,(const u32*)&_gz_co_header,16)||
  !_gz_cm_healthy()||frame!=_gz_co_frame)goto changed;
 out->process=process.object;out->process_generation=process.generation;out->profile=process.profile;out->pid=process.pid;
 out->allocation_bytes=process.allocation_bytes;out->collider=p;out->collider_generation=collider.generation;
 out->member_offset=p-process.object;out->status_audit=collider.status;out->object_vtable_audit=collider.object_vtable;
 out->co_flags=collider.flags;out->frame=frame;out->po_serial=ph[7];out->co_serial=ch[7];out->requested_bytes=n;
 out->result=CM_OK;out->cm_flags=_gz_cm_control.flags;__sync_synchronize();out->valid=1;
 __sync_synchronize();*(volatile u32*)&_gz_cm_query_busy=0;return CM_OK;
changed:result=CM_CHANGED;
done:out->result=result;out->cm_flags=_gz_cm_control.flags;
 __sync_synchronize();*(volatile u32*)&_gz_cm_query_busy=0;return result;
}
