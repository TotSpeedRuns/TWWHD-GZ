#include "generic_process_cut_native.h"
#include "owned_heap_native.h"
#include <stdint.h>
#include "opaque_chunks.h"

static void cp(void*a,const void*b,u32 n){u32 i;volatile unsigned char*x=a;
 const volatile unsigned char*y=b;for(i=0;i<n;i++)x[i]=y[i];}
static void ze(void*a,u32 n){u32 i;volatile unsigned char*x=a;for(i=0;i<n;i++)x[i]=0;}
static u32 eq(const void*a,const void*b,u32 n){u32 i;const volatile unsigned char*x=a,*y=b;
 for(i=0;i<n;i++)if(x[i]!=y[i])return 0;return 1;}
static u32 ap(const void*a,u32 n,const void*b,u32 m){uintptr_t p=(uintptr_t)a,q=(uintptr_t)b;
 return n<=UINTPTR_MAX-p&&m<=UINTPTR_MAX-q&&(p+n<=q||q+m<=p);}
static u32 has(u32 a,u32 n,u32 b,u32 m){return n&&m&&n<=0xffffffffu-a&&
 m<=0xffffffffu-b&&b>=a&&b-a<=n&&m<=n-(b-a);}
static u32 word(const unsigned char*p,u32 o){return ((u32)p[o]<<24)|((u32)p[o+1]<<16)|
 ((u32)p[o+2]<<8)|p[o+3];}
static u32 half(const unsigned char*p,u32 o){return ((u32)p[o]<<8)|p[o+1];}
static u32 hash(const unsigned char*p,u32 n){u32 h=2166136261u,i;for(i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}

u32 gpn_decode_base(const PoEntry*p,const unsigned char*b,u32 n,GpnBase*out){
 if(!p||!b||!out||n<0xb8||!ap(out,sizeof(*out),p,sizeof(*p))||!ap(out,sizeof(*out),b,n))return GPN_ARGUMENT;
 ze(out,sizeof(*out));
 if(!ghcp_process_format(p)||p->state!=PO_LIVE||p->flags||
  word(b,4)!=p->pid||half(b,0xe)!=p->profile||half(b,8)!=p->process_name||
  word(b,0x10)!=p->profile_entry)return GPN_NATIVE_HEADER;
 cp(out->base,b,0xb8);out->checksum=hash(b,0xb8);out->profile=half(b,0xe);
 out->layer=word(b,0x2c);out->methods=word(b,0xa8);out->create_request=word(b,0x14);out->append=word(b,0xac);
 if(n>=0xbcu){out->subtype_present=1;out->subtype=word(b,0xb8);}
 out->missing=GPN_MISSING_ALL;return GPN_OK;
}

static u32 base_from_cut(const GpnImage*image,const GrcCutView*v,u32 index,GpnBase*out){
 const GhcHeader*h=image->image;const GhcProcessRow*r;u32 off,n,base_bytes;unsigned char base[0xbc];
 if(index>=v->stamp->row_count)return GPN_PLAN;r=v->rows+index;
 if(r->binding!=GRC_BIND_LIVE||r->process.state!=PO_LIVE||
  !has(h->domain.start,h->domain.span,r->process.object,r->process.allocation_bytes))return GPN_CUT;
 off=r->process.object-h->domain.start;n=r->process.allocation_bytes;
 /* Historical pointers are only logical offsets. Read the bounded base from
  * the pinned PRIVATE lossless container, never from a saved native pointer. */
 if(h->version!=GHC_PROCESS_VERSION||h->data>image->bytes||h->data_bytes!=image->bytes-h->data||n<0xb8)return GPN_CUT;
 base_bytes=n>=sizeof(base)?sizeof(base):0xb8u;
 if(oc_read((const unsigned char*)image->image+h->data,h->data_bytes,h->domain.span,off,base,base_bytes)!=OC_OK)return GPN_CUT;
 if(gpn_decode_base(&r->process,base,base_bytes,out))return GPN_NATIVE_HEADER;
 out->index=index;out->raw_offset=off;return GPN_OK;
}

u32 gpn_build(const GpnImage*s,const GpnImage*c,const GrcCutStamp*expected,
 GpnWork*w,u32*sort,u32 sort_words,GrcPlan*plan,GrcPlanRow*pr,GpnAction*a,u32 cap,GpnReport*report){
 const void*ptr[11];u32 len[11],i,j,z,n,need=GPN_NEEDS_FPC_GLOBALS|GPN_NEEDS_NATIVE_LAYERS|
 GPN_NEEDS_REVERSE_REFS|GPN_NEEDS_NATIVE_RESOURCE_CLOSURE|GPN_NEEDS_GPU_AUDIO|GPN_NEEDS_REAL_WRITER|
 GPN_NEEDS_CONSTRUCTOR_HISTORY|GPN_NEEDS_CURRENT_CUT;
 if(!s||!c||!expected||!w||!sort||!plan||!pr||!a||!report||!cap||cap>2u*PO_CAP||
  sort_words>2u*PO_CAP||sort_words<1||!s->image||!c->image)return GPN_ARGUMENT;
 /* The descriptors and expected tag are protected too: writing an output
  * must not mutate a later input/expected comparison. Input images may alias
  * each other but every writable interval is pairwise disjoint and outside
  * BOTH images. No runtime heap/object address appears in this table. */
 ptr[0]=w;len[0]=sizeof(*w);ptr[1]=sort;len[1]=sort_words*4u;
 ptr[2]=plan;len[2]=sizeof(*plan);ptr[3]=pr;len[3]=cap*sizeof(*pr);
 ptr[4]=a;len[4]=cap*sizeof(*a);ptr[5]=report;len[5]=sizeof(*report);
 ptr[6]=s;len[6]=sizeof(*s);ptr[7]=c;len[7]=sizeof(*c);ptr[8]=expected;len[8]=sizeof(*expected);
 ptr[9]=s->image;len[9]=s->bytes;ptr[10]=c->image;len[10]=c->bytes;
 for(i=0;i<6;i++)for(j=i+1;j<11;j++)if(!ap(ptr[i],len[i],ptr[j],len[j]))return GPN_ARGUMENT;
 ze(report,sizeof(*report));report->magic=GPN_MAGIC;report->bytes=sizeof(*report);report->version=GPN_VERSION;
 report->missing=GPN_MISSING_ALL;report->requirements=need;
 z=grc_project_as8_private(s->image,s->bytes,s->identity,s->generation,&w->saved_stamp,&w->saved_view);
 if(z){report->status=GPN_CUT;return GPN_CUT;}
 z=grc_project_as8_private(c->image,c->bytes,c->identity,c->generation,&w->current_stamp,&w->current_view);
 if(z){report->status=GPN_CUT;return GPN_CUT;}
 z=grc_plan(&w->saved_view,&w->current_view,expected,sort,sort_words,plan,pr,cap);
 if(z!=GRC_OK&&z!=GRC_UNRESOLVED){report->status=GPN_PLAN;return GPN_PLAN;}
 if(!plan->valid||plan->count>cap||plan->missing!=GRC_MISSING_ALL||
  !eq(&plan->saved,&w->saved_stamp,sizeof(plan->saved))||
  !eq(&plan->current,&w->current_stamp,sizeof(plan->current))){report->status=GPN_PLAN;return GPN_PLAN;}
 n=plan->count;ze(a,n*sizeof(*a));
 for(i=0;i<n;i++){
  GpnAction*x=a+i;const GrcPlanRow*p=pr+i;const GrcProcess*sr=0,*cr=0;
  x->saved_index=p->saved_index;x->current_index=p->current_index;x->missing=GPN_MISSING_ALL;x->requirements=need;
  if(p->classification==GRC_MATCHING||p->classification==GRC_SAVED_ONLY||p->classification==GRC_REUSED_ADDRESS){
   z=base_from_cut(s,&w->saved_view,p->saved_index,&w->saved_base);if(z)goto fail;
   sr=w->saved_view.rows+p->saved_index;x->saved_offset=w->saved_base.raw_offset;
   x->saved_profile=sr->process.profile;x->saved_pid=sr->process.pid;x->saved_generation=sr->process.generation;
  }
  if(p->classification==GRC_MATCHING||p->classification==GRC_CURRENT_ONLY||p->classification==GRC_REUSED_ADDRESS){
   z=base_from_cut(c,&w->current_view,p->current_index,&w->current_base);if(z)goto fail;
   cr=w->current_view.rows+p->current_index;x->current_offset=w->current_base.raw_offset;
   x->current_profile=cr->process.profile;x->current_pid=cr->process.pid;x->current_generation=cr->process.generation;
  }
  switch(p->classification){
   case GRC_MATCHING:x->operation=GPN_DOMAIN_REWIND;++report->rewind;break;
   case GRC_CURRENT_ONLY:x->operation=GPN_CURRENT_CLEANUP_AND_DOMAIN_REWIND;
    x->native_delete_entry=0x025df904u;++report->current_cleanup;break;
   case GRC_SAVED_ONLY:x->operation=GPN_SAVED_DOMAIN_REINSTATE;++report->saved_reinstate;break;
   case GRC_REUSED_ADDRESS:x->operation=GPN_REUSED_ADDRESS_RECONCILE;
    x->native_delete_entry=0x025df904u;++report->reused;break;
   default:x->operation=GPN_AUDIT_ONLY;++report->audit_only;break;
  }
  /* Native standard Create is only a possible RECOVERY dependency. No old
   * append/callback/parent/layer is guessed, no Create invocation is emitted
   * for saved-only instances, and no new PID stands in for saved identity. */
  x->native_create_request_entry=0;
 }
 /* Reparse both immutable private images and match all actual stamp bytes;
  * no caller/current-frame snapshot can stand in for the real lease outside
  * this code. Expected-current is compared again by actual planner checks. */
 z=grc_project_as8_private(s->image,s->bytes,s->identity,s->generation,&w->saved_stamp,&w->saved_view);
 if(z||!eq(&plan->saved,&w->saved_stamp,sizeof(plan->saved))){z=GPN_CUT;goto fail;}
 z=grc_project_as8_private(c->image,c->bytes,c->identity,c->generation,&w->current_stamp,&w->current_view);
 if(z||!eq(&plan->current,&w->current_stamp,sizeof(plan->current))||
  !eq(expected,&w->current_stamp,sizeof(*expected))){z=GPN_CUT;goto fail;}
 report->count=n;report->status=GPN_INCOMPLETE;report->valid=1;
 /* native_eligible and replay_eligible remain the original literal zero. */
 return GPN_INCOMPLETE;
fail:report->status=z;return z;
}
