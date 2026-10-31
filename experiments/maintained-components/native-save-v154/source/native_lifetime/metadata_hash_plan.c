#include "metadata_hash_plan.h"
#include <stdint.h>

static HhtU span(const void*p,HhtU n){uintptr_t a=(uintptr_t)p;return a&&n&&n<=UINTPTR_MAX-a;}
static HhtU apart(const void*a,HhtU n,const void*b,HhtU m){uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y>=m:y-x>=n;}
static HhtU key_equal(const HhtSpec*s,const HhtCell*a,const HhtCell*b){
    return a->primary==b->primary&&(s->mode==HHT_PRIMARY_KEY||
        (a->secondary==b->secondary&&(!(a->flags&HHT_EXTENDED)||
         ((b->flags&HHT_EXTENDED)&&a->descriptor==b->descriptor))));
}

HhtU _gz_hht_plan(const HhtSpec*s,const HhtInput*in,HhtU*shadow,HhtU*dest,HhtU*written){
    HhtU i,j,k,bits=0,bytes,outbytes,start,first,target,tag;HhtCell source,cell;
    if(!s||!in||((uintptr_t)s&3u)||!span(s,sizeof(*s))||!span(in,sizeof(*in))||
       !in->current||!in->source||!s->slots||(s->slots&(s->slots-1u))||s->slots>0x10000000u||
       s->sources>s->slots||(s->mode!=HHT_PRIMARY_KEY&&s->mode!=HHT_ALLOCATION_KEY))return HHT_ARGUMENT;
    for(i=s->slots;i>1;i>>=1)++bits;
    if(s->shift!=32u-bits||s->shift==32u)return HHT_ARGUMENT;
    bytes=s->slots*4u;outbytes=s->sources*4u;
    if(!span(shadow,bytes)||((uintptr_t)shadow&3u)||!span(written,4)||((uintptr_t)written&3u)||
       !apart(shadow,bytes,s,sizeof(*s))||!apart(shadow,bytes,in,sizeof(*in))||
       !apart(shadow,bytes,written,4)||!apart(written,4,s,sizeof(*s))||!apart(written,4,in,sizeof(*in))||
       (outbytes&&(!span(dest,outbytes)||((uintptr_t)dest&3u)||!apart(dest,outbytes,shadow,bytes)||
        !apart(dest,outbytes,written,4)||!apart(dest,outbytes,s,sizeof(*s))||!apart(dest,outbytes,in,sizeof(*in)))))return HHT_ARGUMENT;
    *written=0;
    for(i=0;i<s->slots;i++){
        if(!in->current(in->current_context,i,&cell))return HHT_READER;
        if(cell.flags&~(HHT_EXTENDED|HHT_RETIRED|HHT_LIVE_OUTSIDE))return HHT_FORMAT;
        tag=cell.flags&3u;
        if((!cell.primary&&(cell.secondary||cell.descriptor||cell.flags))||
           (cell.primary&&tag!=HHT_LIVE_OUTSIDE&&tag!=HHT_RETIRED))return HHT_FORMAT;
        shadow[i]=0;
    }
    for(i=0;i<s->sources;i++){
        if(!in->source(in->source_context,i,&source))return HHT_READER;
        if(!source.primary||(source.flags&~HHT_EXTENDED)||
           (s->mode==HHT_ALLOCATION_KEY&&!source.secondary))return HHT_FORMAT;
        start=((source.primary>>2)*0x9e3779b1u)>>s->shift;first=s->slots;target=s->slots;
        for(k=0;k<s->slots;k++){
            j=(start+k)&(s->slots-1u);
            if(shadow[j]){
                if(shadow[j]>i||!in->source(in->source_context,shadow[j]-1,&cell))return HHT_READER;
                if(key_equal(s,&source,&cell))return HHT_DUPLICATE;
                continue;
            }
            if(!in->current(in->current_context,j,&cell))return HHT_READER;
            if(key_equal(s,&source,&cell)){
                if((cell.flags&3u)==HHT_LIVE_OUTSIDE)return HHT_PROTECTED;
                target=j;break;
            }
            if((cell.flags&3u)==HHT_RETIRED&&first==s->slots)first=j;
            if(!cell.primary){target=first==s->slots?j:first;break;}
        }
        if(target==s->slots)target=first;
        if(target==s->slots)return HHT_SPACE;
        shadow[target]=i+1;
    }
    for(i=0;i<s->slots;i++)if(shadow[i])dest[shadow[i]-1]=i+1;
    *written=s->sources;return HHT_OK;
}
