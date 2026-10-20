/* A typed source recipe prepared within real CLEAN/OBSERVE ownership. It does
 * not issue a constructor, advance a native queue, or enable lifetime scope.
 * The future mutating participant must separately certify live actor deltas,
 * layer/resources, native request ownership and complete transaction undo. */
extern u32 _gz_som_bomb_source(u32,u32,u32);
typedef struct {
 u32 magic,bytes,version,valid,token,slot,source,used;
 u32 pin,identity,generation,revision,frame,serial,source_row,row_hash;
 PbaKey recipe;
 PbRow saved;
} HbpRecipe;
static HbpRecipe _gz_hbp_recipe;
static u32 _gz_hbp_seal;
static u32 hbp_sealed(void){const HbpRecipe*s=&_gz_hbp_recipe;
 return s->magic==0x48425031u&&s->bytes==sizeof(*s)&&s->version==1&&s->valid==1&&
  _gz_hbp_seal==btr_hash(s,sizeof(*s));
}
static u32 hbp_binding(const HbpRecipe*s,const volatile u32*v){return
 v[0]==0x53425631u&&v[1]==64&&v[2]==1&&(v[3]==2||v[3]==3)&&v[4]==s->token&&
 v[7]==s->pin&&v[8]==s->slot&&v[9]==s->source&&v[10]==s->used&&v[11]==s->identity&&
 v[12]==s->generation&&v[13]==s->revision&&!v[14]&&!v[15];}
u32 _gz_hbp_prepare(u32 token,u32 frame){HbpRecipe*s=&_gz_hbp_recipe;const volatile u32*v;const PbRow*row;u32 a,i,p,serial;
 if(s->valid||_gz_hbp_seal||_gz_pbac_active||_gz_pbac_fatal||_gz_pbac_issued.valid)return 5;
 a=_gz_som_bomb_source(token,frame,1);if(!a)return 42;v=(const volatile u32*)a;
 if(v[3]!=2||v[8]>9||v[5]!=frame)return 42;
 p=v[9]+R32(v[9],208);if(R32(p,72)!=1)return 28;
 row=(const PbRow*)(p+96);
 if(_gz_ibi_record_safe(row)||btr_saved(row,0xa70,1)!=1||btr_saved(row,0xa78,4)!=3)return 28;
 serial=v[6];s->valid=0;s->magic=0x48425031u;s->bytes=sizeof(*s);s->version=1;s->token=token;
 s->slot=v[8];s->source=v[9];s->used=v[10];s->pin=v[7];s->identity=v[11];s->generation=v[12];s->revision=v[13];
 s->frame=frame;s->serial=serial;s->source_row=(u32)row;s->row_hash=btr_hash(row,sizeof(*row));
 btr_copy(&s->saved,row,sizeof(*row));
 for(i=0;i<sizeof(s->recipe);i++)((u8*)&s->recipe)[i]=0;
 s->recipe.token=token;s->recipe.slot=s->slot;s->recipe.archive=s->source;s->recipe.stage0=row->context.stage0;
 s->recipe.stage1=row->context.stage1;s->recipe.room=row->context.room;s->recipe.layer=row->context.layer;
 s->recipe.generation=token;s->recipe.pid=BAD;s->recipe.callback=(u32)_gz_pbac_ready_callback;
 for(i=0;i<20;i++)s->recipe.home[i]=row->home[i];
 for(i=0;i<5;i++)s->recipe.family[i]=row->resources[i+1];
 if(!recipe_safe(&s->recipe))return 28;
 a=_gz_som_bomb_source(token,frame,0);if(!a||!hbp_binding(s,(const volatile u32*)a)||
  R32(a,12)!=2||R32(a,24)!=serial||!btr_equal(row,&s->saved,sizeof(*row)))return 42;
 s->valid=1;_gz_hbp_seal=btr_hash(s,sizeof(*s));return 0;
}
u32 _gz_hbp_read(u32 token,u32 frame){const HbpRecipe*s=&_gz_hbp_recipe;u32 a;
 if(!hbp_sealed()||!token||s->token!=token)return 0;
 a=_gz_som_bomb_source(token,frame,0);if(!a||!hbp_binding(s,(const volatile u32*)a))return 0;
 /* Source pointers are touched only AFTER a fresh real owner/pin check. The
  * expired captured actor and model addresses are never followed. */
 if(btr_hash((const void*)s->source_row,sizeof(PbRow))!=s->row_hash||
  !btr_equal((const void*)s->source_row,&s->saved,sizeof(PbRow)))return 0;
 return (u32)s;
}
/* The original pin/common sampler already authenticates the real installed
 * allocation. This separate service reader retains the saved-row comparison;
 * it does not broaden the constructor's original CLEAN/OBSERVE/service9 API. */
extern u32 _gz_roh_replay_common(u32,u32,u32);
u32 _gz_hbp_service_read(u32 token,u32 frame){const HbpRecipe*s=&_gz_hbp_recipe;
 if(!hbp_sealed()||!token||s->token!=token||_gz_roh_replay_common(token,frame,1))return 0;
 if(_gz_osa_pin_id()!=s->pin||_gz_osa_validate(s->pin)||_gz_osa_source(s->pin)!=s->source||
  _gz_osa_bytes(s->pin)!=s->used||_gz_roh_owner[16]!=s->slot||_gz_roh_owner[17]!=s->source||
  _gz_roh_owner[18]!=s->used||s->source_row!=s->source+R32(s->source,208)+96||
  btr_hash((const void*)s->source_row,sizeof(PbRow))!=s->row_hash||
  !btr_equal((const void*)s->source_row,&s->saved,sizeof(PbRow)))return 0;
 return _gz_roh_replay_common(token,frame,0)?0:(u32)s;
}
u32 _gz_hbp_forget(u32 token){const volatile u32*o=_gz_roh_owner;
 if(!token||!hbp_sealed()||_gz_hbp_recipe.token!=token)return 28;
 if(o[0]!=0x524f4831u||o[1]!=240||o[2]!=2||o[3]||o[4]||o[5]||o[6]||o[12]||_gz_roh_user_guard())return 5;
 /* No source read and no source-pin release. Original owner already released
  * its own pin through the existing clean abort / future final transaction. */
 _gz_hbp_recipe.valid=0;_gz_hbp_seal=0;return 0;
}
