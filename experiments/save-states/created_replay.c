/* Typed participant for one genuinely owned replacement inventory bomb.
 * The caller supplies native constructor ownership, never an archive pointer
 * or actor pointer from the host. This does not authorize whole-world replay. */
#include "bomb_state.h"
extern u32 _gz_btr_authority(u32 token,u32 actor,u32 count,u32 frame);
extern u32 _gz_btr_freeze_authority(u32 token,u32 actor,u32 link,u32 frame);
extern u32 _gz_boh_recovery_authority(u32 token,u32 actor,u32 link,u32 frame);
extern void _gz_btr_native_matrix(u32 actor);
extern volatile u32 _gz_btr_frame;
extern u8 _gz_btr_shared_matrix[48];
enum { BTR_PREPARED=1,BTR_APPLIED=2,BTR_ROLLED=3,BTR_COMMITTED=4,BTR_DIRTY=5 };
typedef struct {
    u32 phase,token,actor,link,frame,count,source,source_hash,model,counter;
    PbRow saved,before,mapped;
    u8 model_before[60],matrix_before[48];
} BtrJournal;
typedef char BtrJournalSize[(sizeof(BtrJournal)==6292)?1:-1];
BtrJournal _gz_btr_journal;
u32 _gz_btr_report[24];
static u32 _gz_btr_seal;
/* Independent bounded progress receipt. The existing journal layout/seal and
 * ordinary authority remain unchanged. No source address is needed for undo. */
static u32 _gz_btr_progress[5];
#define BR8(p,o) (*(volatile u8*)((u32)(p)+(o)))
#define BR32(p,o) (*(volatile u32*)((u32)(p)+(o)))
static u32 btr_span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n<=0xbffe0000u-p;}
static void btr_copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n;i++)((volatile u8*)d)[i]=((const volatile u8*)s)[i];}
static u32 btr_equal(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n;i++)if(((const volatile u8*)a)[i]!=((const volatile u8*)b)[i])return 0;return 1;}
static u32 btr_hash(const void*p,u32 n){u32 i,h=2166136261u;for(i=0;i<n;i++)h=(h^((const volatile u8*)p)[i])*16777619u;return h;}
static void btr_progress(u32 stage){const BtrJournal*j=&_gz_btr_journal;
 _gz_btr_progress[0]=j->token;_gz_btr_progress[1]=j->actor;
 _gz_btr_progress[2]=j->frame;_gz_btr_progress[3]=stage;
 _gz_btr_progress[4]=btr_hash(_gz_btr_progress,16);
}
static u32 btr_fields(u32 a,const PbRow*r){u32 i,p=0,n;
    for(i=0;i<_gz_ibi_field_count;i++){const PbField*f=&_gz_ibi_fields[i];n=f->count*f->width;
        if(p+n>r->scalar_bytes||!btr_equal((const void*)(a+f->offset),r->fields+p,n))return 0;
        p+=n;}
    return p==r->scalar_bytes;
}
static u32 btr_fuse(u32 a){return ((u32)BR8(a,0xa7c)<<8)|BR8(a,0xa7d);}
static u32 btr_saved(const PbRow*r,u32 offset,u32 width){u32 i,p=0,n,j,v=0;
    for(i=0;i<_gz_ibi_field_count;i++){const PbField*f=&_gz_ibi_fields[i];n=f->count*f->width;
        if(offset>=f->offset&&offset-f->offset+width<=n){for(j=0;j<width;j++)v=(v<<8)|r->fields[p+offset-f->offset+j];return v;}p+=n;}
    return 0xffffffffu;
}
static u32 btr_owner_live(u32 token,u32 count){BtrJournal*j=&_gz_btr_journal;
    /* Preparation/apply remain same-frame. Once applied, the replacement may
     * remain quarantined through the bounded outer operation. Preserve the
     * original collision frame/preimages; never refresh the journal seal to
     * pretend the constructor happened again in a later frame. */
    return _gz_btr_seal==btr_hash((const u8*)j+4,sizeof(*j)-4)&&j->token==token&&
        (j->phase==BTR_PREPARED?j->frame==_gz_btr_frame:_gz_btr_frame-j->frame<=120u)&&
        _gz_btr_authority(token,j->actor,count,_gz_btr_frame)&&
        BR32(j->actor,0x3bc)==j->model&&BR8(j->link,0x690c)==count;
}
static u32 btr_source(void){BtrJournal*j=&_gz_btr_journal;return btr_hash((const void*)j->source,sizeof(PbRow))==j->source_hash&&btr_equal((const void*)j->source,&j->saved,sizeof(PbRow));}
u32 _gz_btr_prepare(u32 token,u32 actor,u32 link,const void*source_raw,u32 count){
    BtrJournal*j=&_gz_btr_journal;const PbRow*source=(const PbRow*)source_raw;u32 i;
    if(j->phase!=0&&j->phase!=BTR_ROLLED&&j->phase!=BTR_COMMITTED)return 28;
    if(!token||count>=3||!btr_span(actor,0xb50)||!btr_span(link,0x690d)||!btr_span((u32)source,sizeof(*source))||
        !_gz_btr_authority(token,actor,count,_gz_btr_frame)||_gz_ibi_record_safe(source)||
        btr_saved(source,0xa70,1)!=1||btr_saved(source,0xa78,4)!=3)return 28;
    j->phase=0;j->token=token;j->actor=actor;j->link=link;j->frame=_gz_btr_frame;j->count=count;
    j->source=(u32)source;j->source_hash=btr_hash(source,sizeof(*source));btr_copy(&j->saved,source,sizeof(*source));
    if(_gz_ibi_capture((void*)actor,&j->before,&source->context)||BR8(actor,0xa70)!=0||BR32(actor,0xa78)!=1||
        btr_fuse(actor)!=150||BR8(link,0x690c)!=count)return 28;
    j->model=BR32(actor,0x3bc);if(!btr_span(j->model,0xf8))return 28;
    j->counter=BR32(actor,0xb48);btr_copy(j->model_before,(const void*)(j->model+0xbc),60);
    btr_copy(j->matrix_before,_gz_btr_shared_matrix,48);
    btr_copy(&j->mapped,source,sizeof(*source));j->mapped.pid=BR32(actor,4);j->mapped.actor=actor;j->mapped.resources[0]=j->model;
    for(i=0;i<19;i++)if(j->mapped.home[i]!=j->before.home[i])return 28;
    for(i=1;i<6;i++)if(j->mapped.resources[i]!=j->before.resources[i])return 28;
    /* This participant transfers one captured counted bomb. Other modes and
     * banks remain explicit gaps until their count/ownership closure is added. */
    if(_gz_ibi_preflight((void*)actor,&j->mapped,&source->context))return 28;
    _gz_btr_seal=btr_hash((const u8*)j+4,sizeof(*j)-4);j->phase=BTR_PREPARED;btr_progress(1);_gz_btr_report[0]=0x42545231u;_gz_btr_report[1]=sizeof(_gz_btr_report);
    _gz_btr_report[2]=token;_gz_btr_report[3]=j->before.scalar_bytes;_gz_btr_report[4]=btr_fuse(actor);
    _gz_btr_report[5]=count;_gz_btr_report[6]=BR8(actor,0xa70);_gz_btr_report[7]=j->frame;
    return btr_owner_live(token,count)&&btr_source()&&btr_fields(actor,&j->before)?0:28;
}
u32 _gz_btr_apply(u32 token){BtrJournal*j=&_gz_btr_journal;u32 z;
    if(j->phase!=BTR_PREPARED||!btr_owner_live(token,j->count)||!btr_source()||!btr_fields(j->actor,&j->before))return 28;
    j->phase=BTR_DIRTY;z=_gz_ibi_apply((void*)j->actor,&j->mapped,&j->mapped.context,j->frame);
    if(z)return z;
    btr_progress(2);
    if(BR8(j->actor,0xa70)!=1){return 28;}
    BR8(j->link,0x690c)=(u8)(j->count+1);
    btr_progress(3);
    j->phase=BTR_APPLIED;
    _gz_btr_native_matrix(j->actor);btr_copy(_gz_btr_shared_matrix,j->matrix_before,48);
    btr_progress(4);
    if(!btr_owner_live(token,j->count+1)||!btr_source()||!btr_fields(j->actor,&j->mapped)||BR32(j->actor,0xb48)!=j->frame-1)return 28;
    _gz_btr_report[8]=btr_fuse(j->actor);_gz_btr_report[9]=BR8(j->link,0x690c);_gz_btr_report[10]=BR8(j->actor,0xa70);
    _gz_btr_report[11]=BR32(j->actor,0xa78);j->phase=BTR_APPLIED;return 0;
}
u32 _gz_btr_rollback(u32 token){BtrJournal*j=&_gz_btr_journal;u32 z;
    /* Undo uses sealed private preimages and live constructor ownership. A
     * damaged immutable source must not block recovery after mutation. */
    if(j->phase!=BTR_APPLIED||!btr_owner_live(token,j->count+1)||!btr_fields(j->actor,&j->mapped))return 28;
    j->phase=BTR_DIRTY;z=_gz_ibi_apply((void*)j->actor,&j->before,&j->before.context,j->frame);if(z)return z;
    BR32(j->actor,0xb48)=j->counter;BR8(j->link,0x690c)=(u8)j->count;
    btr_copy((void*)(j->model+0xbc),j->model_before,60);btr_copy(_gz_btr_shared_matrix,j->matrix_before,48);
    btr_progress(5);
    if(!btr_owner_live(token,j->count)||!btr_fields(j->actor,&j->before)||BR32(j->actor,0xb48)!=j->counter||
        !btr_equal((const void*)(j->model+0xbc),j->model_before,60)||!btr_equal(_gz_btr_shared_matrix,j->matrix_before,48))return 28;
    _gz_btr_report[12]=btr_fuse(j->actor);_gz_btr_report[13]=BR8(j->link,0x690c);_gz_btr_report[14]=BR8(j->actor,0xa70);
    _gz_btr_report[15]=BR32(j->actor,0xa78);_gz_btr_report[16]=_gz_btr_frame;j->phase=BTR_ROLLED;return 0;
}
/* Recovery consumes only the sealed private preimages. The old source may be
 * corrupt or wholly unmapped and the ordinary operation deadline may have
 * expired. Fresh, strict recovery authority must separately own this exact
 * live constructor allocation; no ordinary authority or source read occurs. */
static u32 btr_recovery_private(u32 token,u32 frame){const BtrJournal*j=&_gz_btr_journal;u32 s=_gz_btr_progress[3];
 if(!token||j->token!=token||frame!=_gz_btr_frame||
  (j->phase!=BTR_PREPARED&&j->phase!=BTR_APPLIED&&j->phase!=BTR_DIRTY)||
  _gz_btr_seal!=btr_hash((const u8*)j+4,sizeof(*j)-4)||
  _gz_btr_progress[4]!=btr_hash(_gz_btr_progress,16)||
  _gz_btr_progress[0]!=token||_gz_btr_progress[1]!=j->actor||_gz_btr_progress[2]!=j->frame||
  s<1||s>5||(j->phase==BTR_PREPARED&&s!=1)||
  (j->phase==BTR_APPLIED&&s!=3&&s!=4)||(s==5&&j->phase!=BTR_DIRTY)||
  j->count>=3||!btr_span(j->actor,0xb50)||!btr_span(j->link,0x690d)||!btr_span(j->model,0xf8))return 0;
 return 1;
}
static u32 btr_recovery_live(u32 token,u32 frame){const BtrJournal*j=&_gz_btr_journal;u32 s,count,counter;const PbRow*r;
 if(!btr_recovery_private(token,frame)||
  _gz_boh_recovery_authority(token,j->actor,j->link,frame)!=1||
  !btr_recovery_private(token,frame))return 0;
 /* Both records refer to the genuine replacement allocation. Saved retired
  * actor/model addresses remain inert values in j->saved, never dereferenced. */
 if(j->before.actor!=j->actor||j->mapped.actor!=j->actor||
  j->before.resources[0]!=j->model||j->mapped.resources[0]!=j->model||
  _gz_ibi_preflight((void*)j->actor,&j->before,&j->before.context)||
  _gz_ibi_preflight((void*)j->actor,&j->mapped,&j->mapped.context))return 0;
 s=_gz_btr_progress[3];r=(s==1||s==5)?&j->before:&j->mapped;
 count=(s==1||s==2||s==5)?j->count:j->count+1;
 counter=(s==1||s==5)?j->counter:j->frame-1;
 if(BR32(j->actor,0x3bc)!=j->model||BR8(j->link,0x690c)!=count||
  BR32(j->actor,0xb48)!=counter||!btr_fields(j->actor,r)||
  !btr_equal(_gz_btr_shared_matrix,j->matrix_before,48))return 0;
 /* Before native SetMtx completes, it has not acquired permission to alter
  * model matrices. Stage4 owns precisely that native effect; stage5 already
  * restored it and waits only for a successful fresh final observation. */
 if(s!=4&&!btr_equal((const void*)(j->model+0xbc),j->model_before,60))return 0;
 return btr_recovery_private(token,frame);
}
u32 _gz_btr_recovery_rollback(u32 token){BtrJournal*j=&_gz_btr_journal;u32 frame=_gz_btr_frame,z,s;
 if(!btr_recovery_live(token,frame))return 28;
 s=_gz_btr_progress[3];
 if(s!=5){
  j->phase=BTR_DIRTY;
  z=_gz_ibi_apply((void*)j->actor,&j->before,&j->before.context,j->frame);if(z)return z;
  BR32(j->actor,0xb48)=j->counter;BR8(j->link,0x690c)=(u8)j->count;
  btr_copy((void*)(j->model+0xbc),j->model_before,60);
  btr_copy(_gz_btr_shared_matrix,j->matrix_before,48);btr_progress(5);
 }
 /* A refusal keeps the failed outer operation and this stage5 preimage
  * receipt dirty/held. Retrying verifies the complete restored preimages and
  * fresh authority; it never decrements the Link count for a second time. */
 if(!btr_recovery_live(token,frame))return 28;
 _gz_btr_report[12]=btr_fuse(j->actor);_gz_btr_report[13]=BR8(j->link,0x690c);
 _gz_btr_report[14]=BR8(j->actor,0xa70);_gz_btr_report[15]=BR32(j->actor,0xa78);
 _gz_btr_report[16]=frame;j->phase=BTR_ROLLED;return 0;
}
u32 _gz_btr_final_check(u32 token){BtrJournal*j=&_gz_btr_journal;u32 z=0;
 if(j->phase!=BTR_APPLIED)z=1;
 else if(_gz_btr_seal!=btr_hash((const u8*)j+4,sizeof(*j)-4))z=2;
 else if(j->token!=token)z=3;
 else if(_gz_btr_frame-j->frame>120u)z=4;
 else if(!_gz_btr_authority(token,j->actor,j->count+1,_gz_btr_frame))z=5;
 else if(BR32(j->actor,0x3bc)!=j->model)z=6;
 else if(BR8(j->link,0x690c)!=j->count+1)z=7;
 else if(!btr_source())z=8;
 else if(!btr_fields(j->actor,&j->mapped))z=9;
 if(z&&!_gz_btr_report[17]){_gz_btr_report[17]=z;_gz_btr_report[18]=_gz_btr_frame;_gz_btr_report[19]=token;_gz_btr_report[20]=j->phase;}
 return z?28:0;
}
void _gz_btr_commit_leaf(void){_gz_btr_journal.phase=BTR_COMMITTED;}

/* Pausing an already-applied owned actor is not a commit operation. Native
 * Management may be traversing a temporary creation queue at this call site.
 * No source read, replay, native call, release or publication is permitted.
 * Final commit still requires the original complete authority/source checks. */
u32 _gz_btr_suspended_check(u32 token){BtrJournal*j=&_gz_btr_journal;u32 z=0;
 if(j->phase!=BTR_APPLIED||_gz_btr_seal!=btr_hash((const u8*)j+4,sizeof(*j)-4))z=1;
 else if(j->token!=token||_gz_btr_frame-j->frame>120u)z=2;
 else if(!_gz_btr_freeze_authority(token,j->actor,j->link,_gz_btr_frame))z=3;
 else if(BR32(j->actor,0x3bc)!=j->model)z=4;
 else if(BR8(j->link,0x690c)!=j->count+1)z=5;
 else if(!btr_fields(j->actor,&j->mapped))z=6;
 else if(BR32(j->actor,0xb48)!=j->frame-1)z=7;
 if(z&&!_gz_btr_report[21]){_gz_btr_report[21]=z;_gz_btr_report[22]=_gz_btr_frame;_gz_btr_report[23]=token;}
 return z?28:0;
}

u32 _gz_btr_cancel_clean(u32 token){BtrJournal*j=&_gz_btr_journal;
 if(j->phase!=BTR_PREPARED||!btr_owner_live(token,j->count)||!btr_fields(j->actor,&j->before)||
  BR32(j->actor,0xb48)!=j->counter||!btr_equal((const void*)(j->model+0xbc),j->model_before,60)||
  !btr_equal(_gz_btr_shared_matrix,j->matrix_before,48))return 28;
 btr_progress(5);j->phase=BTR_ROLLED;return 0;
}
