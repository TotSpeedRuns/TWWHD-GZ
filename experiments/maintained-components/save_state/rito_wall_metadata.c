/* Pointer words are snapshot audit values only. This pure format predicate
 * does not read a native address, resolve a lifetime or grant replay. */
typedef unsigned int RwmU;
typedef struct {RwmU generation,owner_generation,epoch,vtable,resource,signature,triangles;} RwmMeta;
typedef char RwmSize[sizeof(RwmMeta)==28?1:-1];
RwmU _gz_rwm_check(RwmU index,RwmU object,RwmU pid,const RwmMeta*m){RwmU i;const RwmU*p=(const RwmU*)m;
 if(!m)return 0;
 if(!index&&!object&&!pid){for(i=0;i<7;i++)if(p[i])return 0;return 1;}
 return index<256&&object>=0x10000000u&&object<=0xbfffff58u&&!(object&3u)&&pid<0xfffffffeu&&
  m->generation&&m->owner_generation&&m->epoch&&m->vtable>=0x10000000u&&m->vtable<0xc0000000u&&!(m->vtable&3u)&&
  m->resource>=0x10000000u&&m->resource<=0xbfffffd0u&&!(m->resource&3u)&&m->triangles&&m->triangles<0x80000000u;
}
