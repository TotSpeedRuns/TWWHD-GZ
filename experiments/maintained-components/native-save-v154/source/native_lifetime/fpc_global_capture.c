#include "fpc_global_cut.h"
#include "fpc_global_templates.inc"
#include "fpc_global_native_addresses.inc"
extern FgcU _gz_ghs_scope(FgcU,FgcU,FgcU),_gz_ghs_hg(FgcU,FgcU);
extern FgcU _gz_jwa_ghs_check(FgcU),_gz_jwa_ghs_generation(FgcU),_gz_jwa_ghs_workers_zero(FgcU);
extern FgcU _gz_ois_heap_stage_match(FgcU,FgcU,FgcU,FgcU),_gz_ois_heap_apart(FgcU,FgcU,FgcU,FgcU);
extern FgcU _gz_rsa_private_apart(FgcU,FgcU),_gz_ghc_all_private_apart(FgcU,FgcU);
static FgcU fgc_buffer(FgcU p,FgcU n){return !(p&3u)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static FgcU fgc_scope(FgcU req,FgcU frame,FgcU hg,FgcU job){return
 _gz_ghs_scope(req,frame,2)==1&&_gz_ghs_hg(req,frame)==hg&&hg&&job&&
 _gz_jwa_ghs_check(req)==0&&_gz_jwa_ghs_generation(req)==job&&_gz_jwa_ghs_workers_zero(req)!=2u;}
/* Whole staging/published/undo/work allocation exclusion is checked by the
 * real OIS owner, not inferred from output having been allocated earlier. */
static FgcU fgc_source_apart(FgcU req,FgcU frame){FgcU i,p,n;
 for(i=0;i<FGC_COUNT;i++){p=fgc_native_root(i);n=_gz_fgc_layout[i].bytes;
  if(!fgc_buffer(p,n)||!_gz_rsa_private_apart(p,n)||
   !_gz_ghc_all_private_apart(p,n)||!_gz_ois_heap_apart(req,frame,p,n))return 0;}
 for(i=0;i<3;i++){p=fgc_native_template(i);n=fgc_static[i].bytes;
  if(!fgc_buffer(p,n)||!_gz_rsa_private_apart(p,n)||
   !_gz_ghc_all_private_apart(p,n)||!_gz_ois_heap_apart(req,frame,p,n))return 0;}
 return 1;}
static FgcU fgc_static_match(void){FgcU i,k,p;
 for(i=0;i<3;i++){p=fgc_native_template(i);
  if(!fgc_buffer(p,fgc_static[i].bytes))return 0;
  for(k=0;k<fgc_static[i].bytes/4;k++)
   if(((volatile const FgcU*)p)[k]!=fgc_static[i].words[k])return 0;}
 return 1;}
FgcU _gz_fgc_capture(FgcU req,FgcU frame,void*output,FgcU capacity){FgcHeader*h=output;FgcRecord*r;
 FgcU i,k,hg,job,z;FgcU p=(FgcU)output;
 if(!output||!fgc_buffer(p,FGC_BYTES))return FGC_ARGUMENT;
 if(capacity<FGC_BYTES)return FGC_SPACE;
 if(!_gz_ghs_scope(req,frame,2)||!_gz_ois_heap_stage_match(req,frame,p,FGC_BYTES))return FGC_SCOPE;
 hg=_gz_ghs_hg(req,frame);job=_gz_jwa_ghs_generation(req);
 if(!fgc_scope(req,frame,hg,job))return FGC_SCOPE;
 if(!fgc_source_apart(req,frame))return FGC_PROTECTED;
 h->valid=0; /* Only after genuine ownership and whole-source apart proof. */
 if(!fgc_static_match())return FGC_CHANGED;
 /* No private output is installed or reused as native state. Valid last. */
 for(i=0;i<FGC_BYTES/4;i++)((FgcU*)output)[i]=0;
 h->magic=FGC_MAGIC;h->bytes=FGC_BYTES;h->version=FGC_VERSION;
 h->missing=FGC_MISSING_ALL;h->count=FGC_COUNT;h->table=FGC_TABLE;h->data=FGC_DATA;
 h->data_bytes=FGC_PAYLOAD_BYTES;h->model=FGC_MODEL;h->flags=FGC_ROOTS_ONLY;h->static_hash=FGC_STATIC_HASH;
 z=_gz_ghs_source(req,frame,&h->producer);if(z)return FGC_SCOPE;
 if(h->producer.hg!=hg||h->producer.job_generation!=job)return FGC_SCOPE;
 r=(FgcRecord*)(p+FGC_TABLE);
 for(i=0;i<FGC_COUNT;i++){
  const FgcLayout*d=_gz_fgc_layout+i;FgcU native=fgc_native_root(i);FgcU*dst=(FgcU*)(p+FGC_DATA+d->offset);
  if(!fgc_scope(req,frame,hg,job)||!fgc_buffer(native,d->bytes))return FGC_SCOPE;
  r[i].id=d->id;r[i].address=d->address;r[i].bytes=d->bytes;r[i].kind=d->kind;
  r[i].data=FGC_DATA+d->offset;r[i].items=d->items;r[i].item_bytes=d->item_bytes;r[i].flags=d->flags;
  for(k=0;k<d->bytes/4;k++)dst[k]=((volatile const FgcU*)native)[k];
 }
 /* Equal repeat is an observation, not escaped-writer/device coherence. */
 for(i=0;i<FGC_COUNT;i++){
  const FgcLayout*d=_gz_fgc_layout+i;FgcU native=fgc_native_root(i);const FgcU*saved=(const FgcU*)(p+FGC_DATA+d->offset);
  if(!fgc_scope(req,frame,hg,job)||!fgc_buffer(native,d->bytes))return FGC_SCOPE;
  for(k=0;k<d->bytes/4;k++)if(saved[k]!=((volatile const FgcU*)native)[k])return FGC_CHANGED;
 }
 if(!fgc_scope(req,frame,hg,job)||!_gz_ois_heap_stage_match(req,frame,p,FGC_BYTES)||
  !fgc_source_apart(req,frame))return FGC_SCOPE;
 if(!fgc_static_match())return FGC_CHANGED;
 h->checksum=_gz_fgc_checksum(h,FGC_BYTES);__sync_synchronize();h->valid=1;
 z=_gz_fgc_validate(h,FGC_BYTES);if(z){h->valid=0;return z;}return FGC_OK;}
FgcU _gz_fgc_verify_held(FgcU req,FgcU frame,const void*image,FgcU bytes){
 const FgcHeader*h=image;FgcU p=(FgcU)image,hg,job,i,k;
 if(!image||bytes!=FGC_BYTES||!fgc_buffer(p,bytes)||!_gz_ois_heap_stage_match(req,frame,p,bytes))return FGC_SCOPE;
 if(_gz_fgc_validate(image,bytes))return FGC_FORMAT;
 hg=_gz_ghs_hg(req,frame);job=_gz_jwa_ghs_generation(req);
 if(!fgc_scope(req,frame,hg,job)||h->producer.request!=req||h->producer.frame!=frame||
  h->producer.hg!=hg||h->producer.job_generation!=job||
  h->producer.reserved!=_gz_ghs_role(req,frame)||h->producer.slot+1!=_gz_ghs_target(req,frame))return FGC_SCOPE;
 if(!fgc_source_apart(req,frame))return FGC_PROTECTED;
 if(!fgc_static_match())return FGC_CHANGED;
 for(i=0;i<FGC_COUNT;i++){const FgcLayout*d=_gz_fgc_layout+i;FgcU native=fgc_native_root(i);
  const FgcU*saved=(const FgcU*)(p+FGC_DATA+d->offset);
  if(!fgc_scope(req,frame,hg,job)||!fgc_buffer(native,d->bytes))return FGC_SCOPE;
  for(k=0;k<d->bytes/4;k++)if(saved[k]!=((volatile const FgcU*)native)[k])return FGC_CHANGED;
 }
 if(!fgc_scope(req,frame,hg,job)||!_gz_ois_heap_stage_match(req,frame,p,bytes)||
  !fgc_source_apart(req,frame))return FGC_SCOPE;
 return fgc_static_match()?FGC_OK:FGC_CHANGED;}

#include "world_global_cut.h"
#include "owned_heap_held_view.h"
#include "held_heap_roundtrip.h"

/* Bind targets through the original compiled native DATA alias table.
 * Neither canonical record keys nor captured pointer VALUES select targets.
 * This is CPU root replay under the real held writer, not device closure.
 */
FgcU _gz_fgc_held_step(FgcU request, FgcU frame, FgcU phase, FgcU write) {
    const WgcHeader *bank;
    const unsigned char *image;
    FgcU i, k, native, bytes;
    if (write > 1 || (write && phase != GHW_APPLY && phase != GHW_INVERSE) ||
        !_gz_ghw_scope(request, frame, phase)) return FGC_SCOPE;
    bank = _gz_wgc_held_source(request, frame, phase);
    if (!bank) return FGC_SCOPE;
    image = (const unsigned char *)bank + WGC_FPC_OFFSET;
    if (_gz_fgc_validate(image, FGC_BYTES) || !fgc_static_match()) return FGC_FORMAT;
    /* Complete target preflight before the first native global store. */
    for (i = 0; i < FGC_COUNT; ++i) {
        native = fgc_native_root(i); bytes = _gz_fgc_layout[i].bytes;
        if (!fgc_buffer(native, bytes) || (bytes & 3u) ||
            !_gz_rsa_private_apart(native, bytes) ||
            !_gz_ghc_all_private_apart(native, bytes) ||
            !_gz_ois_held_apart(request, frame, &_gz_ois_held_record, native, bytes)) return FGC_PROTECTED;
    }
    for (i = 0; i < FGC_COUNT; ++i) {
        const FgcLayout *layout = _gz_fgc_layout + i;
        const FgcU *source = (const FgcU *)(image + FGC_DATA + layout->offset);
        volatile FgcU *target = (volatile FgcU *)fgc_native_root(i);
        if (!_gz_ghw_scope(request, frame, phase)) return FGC_SCOPE;
        for (k = 0; k < layout->bytes / 4; ++k) {
            if (write) target[k] = source[k];
            else if (target[k] != source[k]) return FGC_CHANGED;
        }
        if (write) _gz_ghw_report[phase == GHW_APPLY ? 22 : 23] += layout->bytes;
    }
    __sync_synchronize();
    return _gz_ghw_scope(request, frame, phase) && fgc_static_match() ? FGC_OK : FGC_SCOPE;
}
