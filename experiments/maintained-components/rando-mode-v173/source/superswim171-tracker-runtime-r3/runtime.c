#include "../superswim171-tracker-index-r2/hash_index.h"
typedef struct {HxU magic,bytes,version,busy,flags,serial,release_serial,active_heaps,active_allocations;
 HxU births,allocations,release_begin,release_end,heap_end,dropped,session;} HxHlControl;
extern HxHlControl _gz_hl_control;
extern HxRow _gz_hl_allocations[HX_ROWS];
extern HxU _gz_hx_legacy_hi_store(HxRow*,const HxRow*);
extern HxU _gz_hx_legacy_hl_import(HxRow*,const void*);
HxIndex _gz_hx_index __attribute__((section(".bss.gz_hx_index")));
_Static_assert(sizeof(HxHlControl)==64,"existing HL control ABI");
static HxU bad(void){_gz_hx_index.ready=0;__sync_synchronize();__sync_fetch_and_or(&_gz_hl_control.flags,
 _gz_hx_index.generation==0xffffffffu?1024u:1u);return 0;}
static HxU locked(void){return _gz_hl_control.magic==0x484c4231u&&_gz_hl_control.bytes==64&&
 _gz_hl_control.version==1&&_gz_hl_control.busy==1&&_gz_hl_control.active_heaps<=4096&&
 _gz_hl_control.active_allocations<=HX_ROWS;}
static HxU ready(void){
 if(!locked())return bad();
 if(!_gz_hx_index.magic&&!_gz_hx_index.version&&!_gz_hx_index.ready&&!_gz_hx_index.generation){
  if(!_gz_hx_rebuild(&_gz_hx_index,_gz_hl_allocations))return bad();
 }
 return _gz_hx_index.magic==HX_MAGIC&&_gz_hx_index.bytes==sizeof(_gz_hx_index)&&
  _gz_hx_index.version==HX_VERSION&&_gz_hx_index.ready==1&&_gz_hx_index.generation&&
  _gz_hx_index.occupied<=HX_ROWS&&!_gz_hx_index.reserved[0]&&!_gz_hx_index.reserved[1]?1:bad();
}
static HxU row_id(const HxRow*a){HxU p=(HxU)a,base=(HxU)_gz_hl_allocations;
 return p>=base&&p-base<sizeof(_gz_hl_allocations)&&(p-base)%sizeof(HxRow)==0?
  (p-base)/sizeof(HxRow):HX_BAD_ROW;
}
static HxU primary_apart(HxU p,HxU n){HxU base=(HxU)_gz_hl_allocations;
 return n&&n<=0xffffffffu-p&&(p+n<=base||base+sizeof(_gz_hl_allocations)<=p);
}
HxU _gz_hx_private_apart(HxU p,HxU n){HxU base=(HxU)&_gz_hx_index;
 return n&&n<=0xffffffffu-p&&sizeof(_gz_hx_index)<=0xffffffffu-base&&
  (p+n<=base||base+sizeof(_gz_hx_index)<=p);
}
HxRow*_gz_hx_select_allocation(HxU p,HxU heap,HxU create){HxU id;
 if(!ready())return 0;id=_gz_hx_find(&_gz_hx_index,_gz_hl_allocations,p,heap,0,0,create);
 if(id==HX_BAD_ROW){bad();return 0;}return id?_gz_hl_allocations+id-1:0;
}
HxRow*_gz_hx_select_custom(HxU p,HxU heap,HxU desc,HxU create){HxU id;
 if(!ready())return 0;id=_gz_hx_find(&_gz_hx_index,_gz_hl_allocations,p,heap,desc,1,create);
 if(id==HX_BAD_ROW){bad();return 0;}return id?_gz_hl_allocations+id-1:0;
}
HxU _gz_hx_store(HxRow*a,const HxRow*r){HxU id=row_id(a),old,result;
 if(!_gz_hx_private_apart((HxU)a,sizeof(*a)))return bad();
 if(id==HX_BAD_ROW)return primary_apart((HxU)a,sizeof(*a))?_gz_hx_legacy_hi_store(a,r):bad();
 if(!ready())return 0;old=a->pointer;result=_gz_hx_legacy_hi_store(a,r);
 /* The genuine store can fail in attach after the primary copy. Synchronize
  * the resulting metadata even on that refusal; never fake a successful store. */
 if(!_gz_hx_row_changed(&_gz_hx_index,_gz_hl_allocations,id,old))return bad();
 return result;
}
HxU _gz_hx_import(HxRow*a,const void*r){HxU id=row_id(a),old,result;
 if(!_gz_hx_private_apart((HxU)a,sizeof(*a)))return 0;
 if(id==HX_BAD_ROW)return primary_apart((HxU)a,sizeof(*a))?_gz_hx_legacy_hl_import(a,r):0;
 if(!ready())return 0;old=a->pointer;result=_gz_hx_legacy_hl_import(a,r);
 if(!_gz_hx_row_changed(&_gz_hx_index,_gz_hl_allocations,id,old))return bad();
 return result;
}
/* Called after a genuine primary policy store, while its caller still holds
 * the original metadata lock. No heap/actor pointer is read. */
void _gz_hx_note_row(HxRow*a){HxU id=row_id(a);
 if(id==HX_BAD_ROW||!ready()){bad();return;}
 if(!_gz_hx_row_changed(&_gz_hx_index,_gz_hl_allocations,id,a->pointer))bad();
}
/* Post-primary-write epilogues of hmx_apply / hmx_inverse, before their
 * genuine caller performs verification or releases PO/HL metadata locks. */
void _gz_hx_after_bulk(void){
 if(!locked()||!_gz_hx_rebuild(&_gz_hx_index,_gz_hl_allocations))bad();
}
/* Genuine allocated() compiler-inlined primary store. old_pointer is retained
 * in its proven private Native frame local+44, before any key overwrite. */
void _gz_hx_note_replaced(HxRow*a,HxU old_pointer){HxU id=row_id(a);
 if(id==HX_BAD_ROW||!ready()){bad();return;}
 if(!_gz_hx_row_changed(&_gz_hx_index,_gz_hl_allocations,id,old_pointer))bad();
}
