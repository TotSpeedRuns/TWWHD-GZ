/* Original TWWHD intrusive-list layout verified in owned EUR instructions:
 * base ctor02756158, child add027560D4/remove02756460,
 * disposer add02756560/remove027565E4 and disposal02756370.
 * Ordered metadata prerequisite for nested arena replay. No raw OS replay. */
#include "heap_graph.h"
#include "native_abi.h"
typedef unsigned char U8;typedef AgU32 U32;
enum { MAGIC=0x41474831,VERSION=1,HEADER=64 };
typedef struct {U32 magic,version,bytes,session,root,parent,heaps,objects,
 checksum,valid,reserved[6];} Header;
typedef char HeaderSize[sizeof(Header)==64?1:-1];
static AgHeap heaps[AG_MAX_HEAPS];static AgObject objects[AG_MAX_OBJECTS];
static U32 busy;
#define R(p,o) (*(volatile const U32*)((p)+(o)))
static void copy(void*d,const void*s,U32 n){U32 i;for(i=0;i<n;i++)((U8*)d)[i]=((const U8*)s)[i];}
static void zero(void*d,U32 n){U32 i;for(i=0;i<n;i++)((U8*)d)[i]=0;}
static U32 equal(const void*a,const void*b,U32 n){U32 i;for(i=0;i<n;i++)if(((const U8*)a)[i]!=((const U8*)b)[i])return 0;return 1;}
static U32 span(U32 a,U32 n){return !(a&3)&&a>=0x10000000u&&n&&n<=0xbffe0000u-a&&a<0xbffe0000u;}
static U32 apart(U32 a,U32 n,U32 b,U32 m){return a>=b+m||b>=a+n;}
static U32 private(U32 a,U32 n){return span(a,n)&&apart(a,n,(U32)heaps,sizeof(heaps))&&
 apart(a,n,(U32)objects,sizeof(objects))&&apart(a,n,(U32)&busy,4);}
static U32 meta_private(U32 a,U32 n){return _gz_abi_metadata_span(a,n)&&apart(a,n,(U32)heaps,sizeof(heaps))&&
 apart(a,n,(U32)objects,sizeof(objects))&&apart(a,n,(U32)&busy,4);}
static U32 domain(const AgDomain*d){U32 i;
 if(!d||!meta_private((U32)d,sizeof(*d))||!private(d->root,0xb8)||(d->parent&&!private(d->parent,0xb8)))return AG_ARGUMENT;
 if(!d->session||!d->token||d->obligations!=AG_REQUIRED)return AG_SCOPE;
 for(i=0;i<3;i++)if(d->reserved[i])return AG_ARGUMENT;
 return 0;
}
static U32 hash(const void*p,U32 n){U32 i,h=2166136261u;const U8*b=p;
 for(i=0;i<n;i++)h=(h^((i>=32&&i<36)?0:b[i]))*16777619u;
 return h;
}
static U32 kind(U32 table){return table==0x101459f0u?1:table==0x10145b48u?2:table==0x10145eb8u?3:0;}
static U32 contains(U32 start,U32 size,U32 p,U32 n){return p>=start&&n<=size&&p-start<=size-n;}
static U32 probe(const AgDomain*d,Header*out){U32 at=0,n=1,total=0,i,j,node,head,prev,count,h,object;AgHeap*r;
 zero(out,sizeof(*out));out->magic=MAGIC;out->version=VERSION;out->session=d->session;out->root=d->root;out->parent=d->parent;
 zero(heaps,sizeof(heaps));heaps[0].heap=d->root;heaps[0].parent=d->parent;
 while(at<n){
  r=heaps+at;h=r->heap;if(!private(h,0xb8)||R(h,0x24)!=r->parent)return AG_LAYOUT;
  r->vtable=R(h,12);r->kind=kind(r->vtable);r->start=R(h,0x1c);r->span=R(h,0x20);
  r->direction=R(h,0x50);r->flags=R(h,0x90);
  if(!private(r->start,r->span)||(r->direction!=1&&r->direction!=0xffffffffu)||r->flags&2)return AG_LAYOUT;
  if(!apart((U32)d,sizeof(*d),h,0xb8)||!apart((U32)d,sizeof(*d),r->start,r->span))return AG_ARGUMENT;
  head=h+0x28;count=R(h,0x30);
  if(count>AG_MAX_HEAPS-n||R(h,0x34)!=0x38)return AG_LIMIT;
  r->child_first=n;r->child_count=count;node=R(head,4);prev=head;
  for(i=0;i<count;i++){
   if(!span(node,8)||node<0x10000038u||node==head||R(node,0)!=prev)return AG_LAYOUT;
   for(j=0;j<n;j++)if(heaps[j].heap==node-0x38)return AG_LAYOUT;
   heaps[n].heap=node-0x38;heaps[n++].parent=h;prev=node;node=R(node,4);
  }
  if(node!=head||prev!=R(head,0))return AG_LAYOUT;
  head=h+0x40;count=R(h,0x48);
  if(count>AG_MAX_OBJECTS-total||R(h,0x4c)!=4)return AG_LIMIT;
  r->object_first=total;r->object_count=count;node=R(head,4);prev=head;
  for(i=0;i<count;i++){
   if(!span(node,12)||node<0x10000004u||node==head||R(node,0)!=prev)return AG_LAYOUT;
   object=node-4;if(!private(object,16)||R(object,0)!=h)return AG_LAYOUT;
   for(j=0;j<total;j++)if(objects[j].object==object)return AG_LAYOUT;
   objects[total].object=object;objects[total].owner=h;objects[total].vtable=R(object,12);
   objects[total++].in_own_heap=contains(r->start,r->span,object,16);
   prev=node;node=R(node,4);
  }
  if(node!=head||prev!=R(head,0))return AG_LAYOUT;
  at++;
 }
 out->heaps=n;out->objects=total;out->bytes=HEADER+n*sizeof(AgHeap)+total*sizeof(AgObject);out->valid=1;
 return 0;
}
static U32 private_graph(U32 a,U32 n,const AgDomain*d,U32 count,U32 object_count){U32 i;
 if(!meta_private(a,n)||!apart(a,n,(U32)d,sizeof(*d)))return 0;
 for(i=0;i<count;i++)if(!apart(a,n,heaps[i].heap,0xb8)||!apart(a,n,heaps[i].start,heaps[i].span))return 0;
 for(i=0;i<object_count;i++)if(!apart(a,n,objects[i].object,16))return 0;
 return 1;
}
static U32 image(const AgDomain*d,const void*p,U32 bytes,Header*out){Header h;const AgHeap*hs;const AgObject*os;U32 z,i,j,next=1,objects_at=0;
 z=domain(d);if(z)return z;
 if(bytes<HEADER||!meta_private((U32)p,bytes)||!apart((U32)p,bytes,(U32)d,sizeof(*d)))return AG_ARGUMENT;
 copy(&h,p,sizeof(h));
 if(h.magic!=MAGIC||h.version!=VERSION||h.valid!=1||h.bytes!=bytes||h.root!=d->root||h.parent!=d->parent||h.session!=d->session||
  !h.heaps||h.heaps>AG_MAX_HEAPS||h.objects>AG_MAX_OBJECTS||bytes!=HEADER+h.heaps*sizeof(AgHeap)+h.objects*sizeof(AgObject)||h.checksum!=hash(p,bytes))return AG_CORRUPT;
 for(i=0;i<6;i++)if(h.reserved[i])return AG_CORRUPT;
 hs=(const AgHeap*)((const U8*)p+HEADER);os=(const AgObject*)(hs+h.heaps);
 for(i=0;i<h.heaps;i++){
  const AgHeap*r=hs+i;
  if(i>=next)return AG_CORRUPT;
  if(!private(r->heap,0xb8)||!private(r->start,r->span)||r->kind!=kind(r->vtable)||r->flags&2||
   (r->direction!=1&&r->direction!=0xffffffffu)||r->child_first!=next||r->child_count>h.heaps-next||
   r->object_first!=objects_at||r->object_count>h.objects-objects_at||
   !apart((U32)p,bytes,r->heap,0xb8)||!apart((U32)p,bytes,r->start,r->span))return AG_CORRUPT;
  if(i==0&&(r->heap!=d->root||r->parent!=d->parent))return AG_CORRUPT;
  for(j=0;j<i;j++)if(hs[j].heap==r->heap)return AG_CORRUPT;
  for(j=0;j<r->child_count;j++)if(hs[next+j].parent!=r->heap)return AG_CORRUPT;
  for(j=0;j<r->object_count;j++){
   const AgObject*o=os+objects_at+j;U32 k;
   if(!private(o->object,16)||o->owner!=r->heap||o->in_own_heap!=contains(r->start,r->span,o->object,16)||
    !apart((U32)p,bytes,o->object,16))return AG_CORRUPT;
   for(k=0;k<objects_at+j;k++)if(os[k].object==o->object)return AG_CORRUPT;
  }
  next+=r->child_count;objects_at+=r->object_count;
 }
 if(next!=h.heaps||objects_at!=h.objects)return AG_CORRUPT;
 if(out)copy(out,&h,sizeof(h));
 return 0;
}
AgU32 _gz_ag_capture(const AgDomain*d,void*out,U32 capacity,U32*written){Header h,after;U32 z;
 if(!_gz_image_write_allowed((U32)out,capacity)||!_gz_image_write_allowed((U32)written,4))return AG_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&busy,0,1))return AG_BUSY;
 z=domain(d);if(z)goto done;z=probe(d,&h);if(z)goto done;
 if(!private_graph((U32)out,capacity,d,h.heaps,h.objects)||!private_graph((U32)written,4,d,h.heaps,h.objects)||!apart((U32)out,capacity,(U32)written,4)){z=AG_ARGUMENT;goto done;}
 *written=0;if(h.bytes>capacity){z=AG_SPACE;goto done;}
 copy(out,&h,HEADER);copy((U8*)out+HEADER,heaps,h.heaps*sizeof(AgHeap));copy((U8*)out+HEADER+h.heaps*sizeof(AgHeap),objects,h.objects*sizeof(AgObject));
 z=probe(d,&after);if(z||h.heaps!=after.heaps||h.objects!=after.objects||
  !equal((U8*)out+HEADER,heaps,h.heaps*sizeof(AgHeap))||!equal((U8*)out+HEADER+h.heaps*sizeof(AgHeap),objects,h.objects*sizeof(AgObject))){z=AG_CHANGED;goto done;}
 ((Header*)out)->checksum=hash(out,h.bytes);*written=h.bytes;
done:__sync_synchronize();busy=0;return z;
}
AgU32 _gz_ag_validate(const AgDomain*d,const void*p,U32 bytes){return image(d,p,bytes,0);}
AgU32 _gz_ag_matches_live(const AgDomain*d,const void*p,U32 bytes){Header h,live;U32 z;
 if(!__sync_bool_compare_and_swap(&busy,0,1))return AG_BUSY;
 z=image(d,p,bytes,&h);if(z)goto done;z=probe(d,&live);if(z)goto done;
 if(h.heaps!=live.heaps||h.objects!=live.objects||!equal((const U8*)p+HEADER,heaps,h.heaps*sizeof(AgHeap))||
  !equal((const U8*)p+HEADER+h.heaps*sizeof(AgHeap),objects,h.objects*sizeof(AgObject)))z=AG_CHANGED;
done:__sync_synchronize();busy=0;return z;
}
/* Images are validated BFS trees. Walk to an earlier parent by its child
 * interval; equal address spans still belong to the deepest registered heap. */
static U32 ancestor(const AgHeap*hs,U32 a,U32 b){U32 i;
 while(b>a){
  for(i=0;i<b;i++)if(b>=hs[i].child_first&&b-hs[i].child_first<hs[i].child_count)break;
  if(i==b)return 0;
  b=i;
 }
 return a==b;
}
AgU32 _gz_ag_owner(const AgDomain*d,const void*p,U32 bytes,U32 object,U32 size,U32*out){Header h;U32 z,i,best=0xffffffffu;
 if(!_gz_image_write_allowed((U32)out,4))return AG_ARGUMENT;
 z=image(d,p,bytes,&h);if(z)return z;
 if(!span(object,size)||!meta_private((U32)out,4)||!apart((U32)out,4,(U32)p,bytes)||!apart((U32)out,4,(U32)d,sizeof(*d)))return AG_ARGUMENT;
 const AgHeap*hs=(const AgHeap*)((const U8*)p+HEADER);
 const AgObject*os=(const AgObject*)(hs+h.heaps);
 for(i=0;i<h.objects;i++)if(!apart((U32)out,4,os[i].object,16))return AG_ARGUMENT;
 for(i=0;i<h.heaps;i++){
  if(!apart((U32)out,4,hs[i].heap,0xb8)||!apart((U32)out,4,hs[i].start,hs[i].span))return AG_ARGUMENT;
  if(contains(hs[i].start,hs[i].span,object,size)){
   if(best==0xffffffffu||ancestor(hs,best,i))best=i;
   else if(!ancestor(hs,i,best))return AG_CORRUPT;
  }
 }
 if(best==0xffffffffu)return AG_SCOPE;
 *out=best;return 0;
}

AgU32 _gz_ag_buffer_private(AgU32 p,AgU32 n){return meta_private(p,n);}
