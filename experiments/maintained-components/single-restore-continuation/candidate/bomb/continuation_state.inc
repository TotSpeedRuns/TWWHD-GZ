/* Fixed private continuation metadata. No raw SOURCE/native pointer or token. */
enum {WCC_MAGIC=0x57434332u,WCC_BOUND=1,WCC_RETRY=2,WCC_RETAINED=3,
 WCC_DEADLINE=650,WCC_CANCEL_SELECTION=1,WCC_CANCEL_SCENE=2,
 WCC_CANCEL_DEADLINE=3,WCC_CANCEL_SOURCE=4};
typedef struct {
 u32 magic,bytes,phase,request,retry_request,token,frame,slot,identity,generation;
 u32 used,hash,cookie,nonce,epoch,stage0,stage1,room,layer,cancel,error,seal;
} WcmContinuation;
static WcmContinuation wcc;
static u32 wcc_entered;
typedef struct {u32 request,token,frame;} WcmCommitBlocker;
static WcmCommitBlocker wcc_blocker;
static void wcc_seal(void){wcc.seal=btr_hash(&wcc,sizeof(wcc)-4);}
static u32 wcc_valid(void){return wcc.magic==WCC_MAGIC&&wcc.bytes==sizeof(wcc)&&
 wcc.phase>=WCC_BOUND&&wcc.phase<=WCC_RETAINED&&wcc.request&&wcc.token&&
 wcc.slot<10&&wcc.identity&&wcc.generation&&wcc.used>=216&&wcc.cookie&&
 wcc.nonce&&wcc.cancel<=WCC_CANCEL_SOURCE&&wcc.seal==btr_hash(&wcc,sizeof(wcc)-4);}
static u32 wcc_empty(void){u32 i;const u32*p=(const u32*)&wcc;
 for(i=0;i<sizeof(wcc)/4;i++)if(p[i])return 0;return 1;}
static u32 wcc_private_apart(u32 p,u32 n){return epj_apart(p,n,(u32)&wcc,sizeof(wcc))&&
 epj_apart(p,n,(u32)&wcc_entered,sizeof(wcc_entered))&&
 epj_apart(p,n,(u32)&wcc_blocker,sizeof(wcc_blocker));}
/* Fixed metadata only: safe from LRW's requested predicate, including while
 * its actual Gate is held. It never looks up NCB, SOURCE or a native actor. */
static u32 wcc_cancelled(u32 request,u32 nonce){u32 reason=0;
 if(!wcc_valid()||wcc.request!=request||wcc.nonce!=nonce)return 0;
 if(wcc.cancel)return wcc.cancel;
 if(_gz_pbac_selected!=wcc.slot)reason=WCC_CANCEL_SELECTION;
 else if(_gz_pbac_epoch!=wcc.epoch||_gz_pbac_stage[0]!=wcc.stage0||
  _gz_pbac_stage[1]!=wcc.stage1||_gz_pbac_room!=wcc.room||_gz_pbac_layer!=wcc.layer||
  *(volatile u8*)&_gz_pbac_loading||_gz_pbac_event||_gz_pbac_world_resume_pending)reason=WCC_CANCEL_SCENE;
 else if((u32)(_gz_pbac_frame-wcc.frame)>WCC_DEADLINE)reason=WCC_CANCEL_DEADLINE;
 if(reason){wcc.cancel=reason;wcc_seal();}return reason;
}
static u32 wcm_lrr_allowed(u32,u32,u32);
static u32 wcm_lrr_bind(u32,u32,u32,u32,u32,u32,u32,u32,u32);
