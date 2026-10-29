/* Portable old/new codec comparison. Buffers belong to the test caller. */
#include <stdint.h>
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

uint32_t old_sc_hash(const void*,uint32_t);
uint32_t new_sc_hash(const void*,uint32_t);
uint32_t old_sc_measure(const void*,uint32_t,uint32_t*);
uint32_t new_sc_measure(const void*,uint32_t,uint32_t*);
uint32_t old_sc_encode(const void*,uint32_t,void*,uint32_t,uint32_t*);
uint32_t new_sc_encode(const void*,uint32_t,void*,uint32_t,uint32_t*);
uint32_t old_sc_decode(const void*,uint32_t,void*,uint32_t,uint32_t,uint32_t,uint32_t*);
uint32_t new_sc_decode(const void*,uint32_t,void*,uint32_t,uint32_t,uint32_t,uint32_t*);

typedef struct {
    uint32_t error,old_hash,new_hash,old_measure,new_measure;
    uint32_t old_size,new_size,old_encode,new_encode;
    uint32_t old_written,new_written,old_decode,new_decode,old_valid,new_valid;
} CompatReport;

static uint32_t same(const void*a,const void*b,uint32_t n){
    const unsigned char*x=a,*y=b;
    uint32_t i;
    for(i=0;i<n;i++)if(x[i]!=y[i])return 0;
    return 1;
}

EXPORT uint32_t codec_compare(const void*source,uint32_t n,
    void*old_encoded,void*new_encoded,void*old_decoded,void*new_decoded,
    uint32_t capacity,CompatReport*r){
    uint32_t i;
    for(i=0;i<sizeof(*r)/sizeof(uint32_t);i++)((uint32_t*)r)[i]=0;
    r->old_hash=old_sc_hash(source,n);
    r->new_hash=new_sc_hash(source,n);
    if(r->old_hash!=r->new_hash)return r->error=1;
    r->old_measure=old_sc_measure(source,n,&r->old_size);
    r->new_measure=new_sc_measure(source,n,&r->new_size);
    if(r->old_measure!=r->new_measure||r->old_size!=r->new_size)return r->error=2;
    r->old_encode=old_sc_encode(source,n,old_encoded,capacity,&r->old_written);
    r->new_encode=new_sc_encode(source,n,new_encoded,capacity,&r->new_written);
    if(r->old_encode!=r->new_encode||r->old_written!=r->new_written)return r->error=3;
    if(!same(old_encoded,new_encoded,capacity))return r->error=4;
    if(r->old_encode!=1)return 0;
    r->old_decode=old_sc_decode(old_encoded,r->old_written,old_decoded,
        capacity,n,r->old_hash,&r->old_valid);
    r->new_decode=new_sc_decode(new_encoded,r->new_written,new_decoded,
        capacity,n,r->new_hash,&r->new_valid);
    if(r->old_decode!=r->new_decode||r->old_valid!=r->new_valid)return r->error=5;
    if(r->old_decode!=1||r->old_valid!=1)return r->error=6;
    if(!same(old_decoded,new_decoded,capacity)||!same(old_decoded,source,n))return r->error=7;
    return 0;
}
