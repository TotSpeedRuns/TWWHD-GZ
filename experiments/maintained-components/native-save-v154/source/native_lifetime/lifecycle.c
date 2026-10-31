/* Original EUR instructions: Exp ctor02753530, Solid ctor027552B8,
 * base02756158; native initialize/reset Exp02754D9C/Solid02755380;
 * non-deallocating destructors02755150/02755E48 with flag2. Destruction
 * removes embedded JKRDisposer and mutex registrations through native code.
 * No source heap mutex/thread bytes are copied into a new heap incarnation.
 */
#include "lifecycle.h"
#include "native_abi.h"
#ifdef GZ_JOINT_OBSERVATION_FIXTURE
extern void _gz_test_aj_constructed(AlU32,AlU32);
#endif
typedef AlU32 U32;
#define R(p,o) (*(volatile const U32*)((p)+(o)))
extern U32 _gz_al_native_exp_ctor(U32,const U32*,U32,U32,U32,U32,U32);
extern U32 _gz_al_native_solid_ctor(U32,const U32*,U32,U32,U32,U32,U32);
extern U32 _gz_al_native_unit_ctor(U32,const U32*,U32,U32,U32,U32,U32);
extern void _gz_al_native_unit_init(U32,U32,U32),_gz_al_native_unit_destroy(U32,U32);
extern void _gz_al_native_exp_reset(U32),_gz_al_native_solid_reset(U32);
extern void _gz_al_native_exp_destroy(U32,U32),_gz_al_native_solid_destroy(U32,U32);
static U32 span(U32 p,U32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static U32 apart(U32 a,U32 n,U32 b,U32 m){return a>=b+m||b>=a+n;}
static U32 contains(U32 a,U32 n,U32 p,U32 m){return p>=a&&m<=n&&p-a<=n-m;}
static U32 table(U32 kind){return kind==1?0x101459f0u:kind==2?0x10145b48u:kind==3?0x10145eb8u:0;}
static U32 header(U32 kind){return kind==1?0xb8:kind==2?0x9c:kind==3?0xac:0;}
AlU32 _gz_al_policy_supported(U32 kind,U32 direction,U32 flags){
 if(!table(kind)||(direction!=1&&direction!=0xffffffffu)||(kind==3&&direction!=1))return 0;
 return flags==4||flags==5||(kind==2&&direction==1&&flags==0);
}
static U32 known(U32 p){U32 t;if(!span(p,0xb8))return 0;t=R(p,12);return (t==table(1)||t==table(2)||t==table(3))&&span(R(p,0x1c),R(p,0x20))&&!(R(p,0x90)&2);}
static U32 check(const AlDomain*d,const AlRecipe*r){U32 p,n=0,header;
 if(!d||!r||!_gz_abi_metadata_span((U32)d,sizeof(*d))||!_gz_abi_metadata_span((U32)r,sizeof(*r))||!span(r->heap,0xb8)||!span(r->start,r->span)||!r->parent||
  !_gz_al_policy_supported(r->kind,r->direction,r->flags)||!apart((U32)d,sizeof(*d),r->start,r->span)||
  !apart((U32)r,sizeof(*r),r->start,r->span))return AL_ARGUMENT;
 if(!d->root||!d->session||!d->token||d->obligations!=31)return AL_SCOPE;
 header=r->kind==1?0xb8:r->kind==2?0x9c:0xac;
 if(r->kind==3&&r->direction!=1)return AL_LAYOUT;
 if(r->span<header+16||(r->direction==1?r->heap!=r->start:r->heap!=r->start+r->span-header))return AL_LAYOUT;
 p=r->parent;
 while(p&&n++<1024){
  if(!known(p)||!apart((U32)d,sizeof(*d),p,0xb8)||!apart((U32)r,sizeof(*r),p,0xb8))return AL_LAYOUT;
  if(p==d->root)break;
  p=R(p,0x24);
 }
 if(p!=d->root)return AL_SCOPE;
 if(!contains(R(r->parent,0x1c),R(r->parent,0x20),r->start,r->span))return AL_SCOPE;
 if(!apart(r->start,r->span,r->parent,R(r->parent,12)==table(1)?0xb8:R(r->parent,12)==table(2)?0x9c:0xac)||
  !apart((U32)d,sizeof(*d),R(r->parent,0x1c),R(r->parent,0x20))||
  !apart((U32)r,sizeof(*r),R(r->parent,0x1c),R(r->parent,0x20)))return AL_ARGUMENT;
 return 0;
}
/* Validate native list links without relying on the saved source header.
 * The caller must separately certify the restored parent backing allocation
 * and lifetime generation before invoking this internal adapter. */
static U32 membership(U32 parent,U32 child,U32*found){U32 head=parent+0x28,node=R(head,4),previous=head,i,n=R(parent,0x30);*found=0;
 if(n>1024||R(parent,0x34)!=0x38)return AL_LAYOUT;
 for(i=0;i<n;i++){
  if(!span(node,8)||node==head||R(node,0)!=previous)return AL_LAYOUT;
  if(node==child+0x38)*found=1;
  previous=node;node=R(node,4);
 }
 return node==head&&R(head,0)==previous?0:AL_LAYOUT;
}
AlU32 _gz_al_construct(const AlDomain*d,const AlRecipe*r){U32 z,found,name,result;
 z=check(d,r);if(z)return z;z=membership(r->parent,r->heap,&found);if(z)return z;if(found)return AL_EXISTS;
 if(r->kind==3)return AL_ARGUMENT;
 name=r->name;
 if(r->kind==1){result=_gz_al_native_exp_ctor(r->heap,&name,r->parent,r->start,r->span,r->direction,r->flags&1);if(result==r->heap)_gz_al_native_exp_reset(result);}
 else{result=_gz_al_native_solid_ctor(r->heap,&name,r->parent,r->start,r->span,r->direction,r->flags&1);if(result==r->heap)_gz_al_native_solid_reset(result);}
 /* Mirror only the verified post-factory adjustment. Never mask unexpected
  * flags or copy an old heap header/mutex into a new incarnation. */
 if(result==r->heap&&r->kind==2&&r->direction==1&&r->flags==0){
  if(R(result,0x90)!=4)return AL_NATIVE;
  *(volatile U32*)(result+0x90)=R(result,0x90)&~4u;
 }
 if(result!=r->heap||R(result,12)!=table(r->kind)||R(result,0x1c)!=r->start||R(result,0x20)!=r->span||R(result,0x24)!=r->parent||
  R(result,0x50)!=r->direction||R(result,0x90)!=r->flags||R(result,0x10)!=r->name)return AL_NATIVE;
 z=membership(r->parent,r->heap,&found);
#ifdef GZ_JOINT_OBSERVATION_FIXTURE
 if(!z&&found)_gz_test_aj_constructed(r->heap,r->kind);
#endif
 return !z&&found?0:AL_NATIVE;
}
AlU32 _gz_al_unit_construct(const AlDomain*d,const AlUnitRecipe*u){U32 z,found,name,result,first,raw,bytes;const AlRecipe*r;
 if(!u||!_gz_abi_metadata_span((U32)u,sizeof(*u)))return AL_ARGUMENT;
 r=&u->heap;z=check(d,r);if(z)return z;
 if(r->kind!=3||u->reserved||u->unit_size<4||(u->unit_size&3)||u->alignment<4||u->alignment>0x10000||
  (u->alignment&(u->alignment-1))||u->unit_size%u->alignment||u->extra_gap>1||
  !apart((U32)u,sizeof(*u),r->start,r->span))return AL_ARGUMENT;
 first=(r->heap+0xac+u->alignment-1)&~(u->alignment-1);
 if(u->extra_gap&&first==r->heap+0xac)first+=u->alignment;
 if(first<r->heap+0xac||first>=r->start+r->span||u->unit_size>r->start+r->span-first)return AL_LAYOUT;
 raw=r->start+r->span-first;bytes=(raw/u->unit_size)*u->unit_size;
 z=membership(r->parent,r->heap,&found);if(z)return z;if(found)return AL_EXISTS;
 name=r->name;result=_gz_al_native_unit_ctor(r->heap,&name,r->parent,r->start,r->span,u->unit_size,r->flags&1);
 if(result!=r->heap)return AL_NATIVE;
 _gz_al_native_unit_init(result,u->alignment,u->extra_gap);
 if(R(result,12)!=table(3)||R(result,0x1c)!=r->start||R(result,0x20)!=r->span||R(result,0x24)!=r->parent||
  R(result,0x50)!=1||R(result,0x90)!=r->flags||R(result,0x10)!=r->name||R(result,0x94)!=u->unit_size||
  R(result,0x98)!=first||R(result,0x9c)!=raw||R(result,0xa0)!=bytes||R(result,0xa4)!=first||R(result,0xa8)!=first)return AL_NATIVE;
 z=membership(r->parent,r->heap,&found);
#ifdef GZ_JOINT_OBSERVATION_FIXTURE
 if(!z&&found)_gz_test_aj_constructed(r->heap,r->kind);
#endif
 return !z&&found?0:AL_NATIVE;
}
AlU32 _gz_al_destroy(const AlDomain*d,const AlRecipe*r){U32 z,found;
 z=check(d,r);if(z)return z;z=membership(r->parent,r->heap,&found);if(z)return z;
 if(!found)return AL_SCOPE;
 if(R(r->heap,12)!=table(r->kind)||R(r->heap,0x1c)!=r->start||R(r->heap,0x20)!=r->span||R(r->heap,0x24)!=r->parent||R(r->heap,0x50)!=r->direction||R(r->heap,0x90)!=r->flags)return AL_LAYOUT;
 if(r->kind==1)_gz_al_native_exp_destroy(r->heap,2);else if(r->kind==2)_gz_al_native_solid_destroy(r->heap,2);else _gz_al_native_unit_destroy(r->heap,2);
 z=membership(r->parent,r->heap,&found);return !z&&!found?0:AL_NATIVE;
}
