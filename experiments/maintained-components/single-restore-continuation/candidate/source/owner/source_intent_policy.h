#ifndef GZ_SOURCE_INTENT_POLICY_H
#define GZ_SOURCE_INTENT_POLICY_H
#include <stdint.h>

/* Logical identity only. No source address, physical lease or native token. */
typedef struct {
 uint32_t magic,bytes,version,serial,inverse;
 uint32_t cookie,request,slot,identity,generation,used,hash,flags,seal;
} SoIntentState;
typedef char SoIntentStateSize[sizeof(SoIntentState)==56?1:-1];
enum {SOI_MAGIC=0x4f534931u,SOI_VERSION=1,SOI_BYTES=56};

/* Word-wise private-metadata seal is independent of target byte order. */
static uint32_t soi_seal_value(const SoIntentState*s){
 const uint32_t*p=(const uint32_t*)s;uint32_t i,h=2166136261u;
 for(i=0;i<13;i++)h=(h^p[i])*16777619u;return h;
}
static uint32_t soi_shape(const SoIntentState*s){
 if(s->magic!=SOI_MAGIC||s->bytes!=SOI_BYTES||s->version!=SOI_VERSION||
  s->serial==0xffffffffu||s->inverse!=~s->serial||s->flags||
  s->seal!=soi_seal_value(s))return 0;
 if(!s->cookie)return !(s->request|s->slot|s->identity|s->generation|s->used|s->hash);
 return s->cookie==s->serial&&s->request&&s->slot<10&&s->identity&&
  s->generation&&s->used>=216&&s->used<=0x200000u&&!(s->used&3);
}
static uint32_t soi_matches(const SoIntentState*s,uint32_t cookie,uint32_t request,
 uint32_t slot,uint32_t identity,uint32_t generation,uint32_t used,uint32_t hash){
 return soi_shape(s)&&cookie&&s->cookie==cookie&&s->request==request&&
  s->slot==slot&&s->identity==identity&&s->generation==generation&&
  s->used==used&&s->hash==hash;
}
static uint32_t soi_write_allowed(const SoIntentState*s,uint32_t slot){
 return soi_shape(s)&&(!s->cookie||s->slot!=slot);
}
/* Entry authority and the actual row/lease are checked by the enclosing TU.
 * This helper changes only fixed private metadata after those checks. */
static uint32_t soi_arm(SoIntentState*s,uint32_t request,uint32_t slot,
 uint32_t identity,uint32_t generation,uint32_t used,uint32_t hash){
 if(!soi_shape(s)||s->cookie||s->serial>=0xfffffffeu||!request||slot>9||
  !identity||!generation||used<216||used>0x200000u||(used&3))return 0;
 s->serial++;s->inverse=~s->serial;s->cookie=s->serial;s->request=request;
 s->slot=slot;s->identity=identity;s->generation=generation;s->used=used;
 s->hash=hash;s->flags=0;s->seal=soi_seal_value(s);return s->cookie;
}
/* Caller has already authenticated the exact receipt. Preserve the cookie
 * generator; an old request/cookie cannot identify a later reservation. */
static void soi_clear_leaf(SoIntentState*s){
 s->cookie=0;s->request=0;s->slot=0;s->identity=0;s->generation=0;
 s->used=0;s->hash=0;s->flags=0;s->seal=soi_seal_value(s);
}
#endif
