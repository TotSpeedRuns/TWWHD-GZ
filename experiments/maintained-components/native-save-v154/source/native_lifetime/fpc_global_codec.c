#include "fpc_global_cut.h"
#include <stdint.h>
#include "fpc_global_layout.inc"
static FgcU fgc_apart(FgcU a,FgcU n,FgcU b,FgcU m){return n<=0xffffffffu-a&&m<=0xffffffffu-b&&(a+n<=b||b+m<=a);}
FgcU _gz_fgc_bytes(void){return FGC_BYTES;}
FgcU _gz_fgc_checksum(const void*image,FgcU bytes){const unsigned char*p=image;FgcU h=2166136261u,i;
 if(!image||bytes!=FGC_BYTES)return 0;
 for(i=0;i<bytes;i++)h=(h^((i>=12&&i<16)||(i>=36&&i<40)?0:p[i]))*16777619u;
 return h;}
FgcU _gz_fgc_validate(const void*image,FgcU bytes){const FgcHeader*h=image;const FgcRecord*r;FgcU i,k,end;
 if(!image||((uintptr_t)image&3u)||bytes!=FGC_BYTES||h->magic!=FGC_MAGIC||h->bytes!=FGC_BYTES||
  h->version!=FGC_VERSION||h->valid!=1||h->missing!=FGC_MISSING_ALL||h->count!=FGC_COUNT||h->table!=FGC_TABLE||
  h->data!=FGC_DATA||h->data_bytes!=FGC_PAYLOAD_BYTES||h->model!=FGC_MODEL||h->flags!=FGC_ROOTS_ONLY||
  h->static_hash!=FGC_STATIC_HASH||h->reserved[0]||h->reserved[1]||h->reserved[2]||h->producer.version!=2||
  !ghs_provenance_format(&h->producer,h->producer.request,h->producer.frame,h->producer.hg,h->producer.lease,
   h->producer.original_return,h->producer.thread))return FGC_FORMAT;
 r=(const FgcRecord*)((const unsigned char*)image+FGC_TABLE);end=FGC_DATA;
 for(i=0;i<FGC_COUNT;i++){
  const FgcLayout*d=_gz_fgc_layout+i;
  if(r[i].id!=d->id||r[i].address!=d->address||r[i].bytes!=d->bytes||r[i].kind!=d->kind||
   r[i].data!=FGC_DATA+d->offset||r[i].items!=d->items||r[i].item_bytes!=d->item_bytes||r[i].flags!=d->flags||
   r[i].data!=end||r[i].data>bytes||r[i].bytes>bytes-r[i].data)return FGC_FORMAT;
  end+=r[i].bytes;
  /* The fixed native intervals are distinct; no archive address is followed. */
  for(k=0;k<i;k++)if(!fgc_apart(r[i].address,r[i].bytes,r[k].address,r[k].bytes))return FGC_FORMAT;
 }
 if(end!=FGC_DATA+FGC_PAYLOAD_BYTES)return FGC_FORMAT;
 for(i=end;i<FGC_BYTES;i++)if(((const unsigned char*)image)[i])return FGC_FORMAT;
 return h->checksum==_gz_fgc_checksum(image,bytes)?FGC_OK:FGC_FORMAT;}
