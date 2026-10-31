/* Independently written TWWHD ExpHeap/SolidHeap experiment, informed by the
 * allocator+live-block design in OoT GZ. No third-party implementation copied.
 * Original native allocate/free code is executed by the separate test harness.
 * No game entry point, engine lease, admission change or native hook is added.
 */
#include "arena_snapshot.h"
#include "native_abi.h"
extern unsigned int forest_domain(const AsDomain*);
extern unsigned int forest_meta(const AsDomain*,unsigned int);
extern unsigned int forest_begin(const AsDomain*,unsigned int);
extern unsigned int forest_end(const AsDomain*,unsigned int);
extern void forest_read(void*,unsigned int,unsigned int);
extern void forest_write(unsigned int,const void*,unsigned int);
extern unsigned int forest_equal(unsigned int,const void*,unsigned int);
typedef unsigned char U8;
typedef AsU32 U32;
enum { AS_MAGIC=0x41534831u, AS_VERSION=3, AS_HEADER=128, AS_ROW=20,
       AS_EXP_TABLE=0x101459f0u, AS_SOLID_TABLE=0x10145b48u, AS_UNIT_TABLE=0x10145eb8u,
       AS_EXP_HEADER=0xb8, AS_SOLID_HEADER=0x9c, AS_UNIT_HEADER=0xac, AS_MUTABLE=0x94 };
typedef struct {
 U32 magic,version,bytes,heap,start,span,heap_generation,session;
 U32 kind,blocks,used,free,data_bytes,layout,checksum,valid;
 U32 mutable_words[9],reserved[7];
} AsHeader;
typedef struct {U32 offset,extent,stored,kind,data;} AsRow;
typedef char HeaderSize[sizeof(AsHeader)==AS_HEADER?1:-1];
typedef char RowSize[sizeof(AsRow)==AS_ROW?1:-1];
static AsRow rows[AS_MAX_BLOCKS];
static U32 busy;
#define R32(p,o) (*(volatile const U32*)((p)+(o)))
static void copy(void*d,const void*s,U32 n){U32 i;for(i=0;i<n;i++)((U8*)d)[i]=((const U8*)s)[i];}
static void zero(void*d,U32 n){U32 i;for(i=0;i<n;i++)((U8*)d)[i]=0;}
static U32 equal(const void*a,const void*b,U32 n){U32 i;for(i=0;i<n;i++)if(((const U8*)a)[i]!=((const U8*)b)[i])return 0;return 1;}
static U32 heap_header(U32 kind){return kind==1?AS_EXP_HEADER:kind==2?AS_SOLID_HEADER:kind==3?AS_UNIT_HEADER:0;}
static U32 heap_kind(U32 vt){return vt==AS_EXP_TABLE?1:vt==AS_SOLID_TABLE?2:vt==AS_UNIT_TABLE?3:0;}
static U32 mutable_count(U32 kind){return kind==1?9:kind==2?2:6;}
static U32 span(U32 p,U32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static U32 apart(U32 a,U32 n,U32 b,U32 m){return a>=b+m||b>=a+n;}
static U32 private_span(U32 p,U32 n,const AsDomain*d){
 return _gz_abi_metadata_span(p,n)&&apart(p,n,d->start,d->span)&&apart(p,n,(U32)rows,sizeof(rows))&&apart(p,n,(U32)&busy,4);
}
static U32 domain(const AsDomain*d){U32 i;
 if(!d||!_gz_abi_metadata_span((U32)d,sizeof(*d))||!span(d->heap,AS_EXP_HEADER)||!span(d->start,d->span)||
    !apart((U32)d,sizeof(*d),(U32)rows,sizeof(rows))||!apart((U32)d,sizeof(*d),(U32)&busy,4))return AS_ARGUMENT;
 if(!d->heap_generation||!d->session||!d->token||d->obligations!=AS_REQUIRED)return AS_SCOPE;
 for(i=0;i<3;i++)if(d->reserved[i])return AS_ARGUMENT;
 /* The forest owns forward/reverse bounds and ordered heap/disposer
  * topology. Source validation uses immutable graph metadata; capture and
  * replay still require separately established live ownership. */
 if(d->span<AS_EXP_HEADER+16||
    !heap_kind(forest_meta(d,0xc))||forest_meta(d,0x1c)!=d->start||forest_meta(d,0x20)!=d->span||
  (forest_meta(d,0x50)!=1&&forest_meta(d,0x50)!=0xffffffffu)||(forest_meta(d,0x90)&2)||!forest_domain(d))return AS_LAYOUT;
 if(!apart(d->start,d->span,(U32)rows,sizeof(rows))||!apart(d->start,d->span,(U32)&busy,4)||
    !apart((U32)d,sizeof(*d),d->start,d->span))return AS_ARGUMENT;
 return 0;
}
static U32 checksum(const void*p,U32 n){const U8*b=(const U8*)p;U32 i,h=2166136261u;
 for(i=0;i<n;i++)h=(h^((i>=56&&i<60)?0:b[i]))*16777619u;
 return h;
}
static U32 block(const AsDomain*d,U32 at,U32 kind,AsRow*r){U32 size,pad;
 if(at<forest_begin(d,AS_EXP_HEADER)||at>forest_end(d,AS_EXP_HEADER)-16||(at&3))return AS_LAYOUT;
 size=R32(at,8);pad=R32(at,12);
 if((size&3)||(pad&3)||size>d->span-16||pad>d->span-16-size||16+pad+size>forest_end(d,AS_EXP_HEADER)-at||
    (!kind&&pad))return AS_LAYOUT;
 r->offset=at-d->start;r->extent=16+pad+size;r->stored=kind?r->extent:16;r->kind=kind;r->data=0;
 if(kind&&pad&&R32(at,12+pad)!=(at|1u))return AS_LAYOUT;
 return 0;
}
/* In-place heapsort: bounded scratch and O(n log n), including fragmented
 * native heaps with more than 100000 allocations. No allocator/API calls. */
static void as_sift(U32 root,U32 n){AsRow value=rows[root];U32 child;
 while(root<n/2){child=root*2+1;if(child+1<n&&rows[child].offset<rows[child+1].offset)child++;
  if(value.offset>=rows[child].offset)break;rows[root]=rows[child];root=child;
 }rows[root]=value;
}
static void as_sort(U32 n){U32 start=n/2,end=n;AsRow temp;
 while(start){--start;as_sift(start,n);}
 while(end>1){--end;temp=rows[0];rows[0]=rows[end];rows[end]=temp;as_sift(0,end);}
}
/* Native fixed-pool layout: ctor02756B7C/init02756C78, allocate02756F94,
 * free027570F8. Only free slots contain a next pointer. Copy their four-byte
 * allocator link, not stale free payload, and copy every allocated slot.
 * The source never supplies OS/mutex bytes or executable callbacks. */
static U32 unit_shape(const AsDomain*d,const AsHeader*h,U32*count){
 U32 stride=h->mutable_words[0],first=h->mutable_words[1],raw=h->mutable_words[2],free=h->mutable_words[3];
 U32 begin=forest_begin(d,AS_UNIT_HEADER),end=forest_end(d,AS_UNIT_HEADER);
 if(forest_meta(d,0x50)!=1||stride<4||(stride&3)||(first&3)||first<begin||first>end||raw!=end-first||
  !raw||stride>raw||h->mutable_words[5]!=first||free>raw||free%stride)return AS_LAYOUT;
 *count=raw/stride;if(!*count||*count>AS_MAX_BLOCKS||free/stride>*count)return AS_LIMIT;
 return 0;
}
static U32 unit_plan(const AsDomain*d,AsHeader*h){U32 i,n,node,at,next,free_count,z,first,stride;
 h->kind=3;h->layout=AS_UNIT_HEADER;
 for(i=0;i<6;i++)h->mutable_words[i]=R32(d->heap,AS_MUTABLE+4*i);
 z=unit_shape(d,h,&n);if(z)return z;first=h->mutable_words[1];stride=h->mutable_words[0];free_count=h->mutable_words[3]/stride;
 for(i=0;i<n;i++){rows[i].offset=first-d->start+i*stride;rows[i].extent=stride;rows[i].stored=stride;rows[i].kind=3;rows[i].data=0;}
 node=h->mutable_words[4];
 for(i=0;i<free_count;i++){
  if(node<first||node-first>=n*stride||(node-first)%stride)return AS_LAYOUT;
  at=(node-first)/stride;if(!rows[at].kind)return AS_LAYOUT;
  rows[at].kind=0;rows[at].stored=4;next=R32(node,0);node=next;
 }
 if(node)return AS_LAYOUT;
 h->blocks=n;h->free=free_count;h->used=n-free_count;h->data_bytes=h->used*stride+free_count*4;
 if(h->data_bytes>0xffffffffu-AS_HEADER-n*AS_ROW)return AS_LIMIT;
 h->bytes=AS_HEADER+n*AS_ROW+h->data_bytes;h->valid=1;at=AS_HEADER+n*AS_ROW;
 for(i=0;i<n;i++){rows[i].data=at;at+=rows[i].stored;}
 return 0;
}
static U32 current_plan(const AsDomain*d,AsHeader*h){U32 k,count,head,node,prev,i,n=0,z,j,at=forest_begin(d,AS_EXP_HEADER)-d->start;AsRow tmp;
 zero(h,sizeof(*h));h->magic=AS_MAGIC;h->version=AS_VERSION;h->heap=d->heap;h->start=d->start;h->span=d->span;
 h->heap_generation=d->heap_generation;h->session=d->session;h->kind=1;h->layout=AS_EXP_HEADER;h->reserved[0]=R32(d->heap,0x10);
 if(R32(d->heap,0xc)==AS_UNIT_TABLE)return unit_plan(d,h);
 if(R32(d->heap,0xc)==AS_SOLID_TABLE){
  U32 front=R32(d->heap,0x94),back=R32(d->heap,0x98),begin=forest_begin(d,AS_SOLID_HEADER),end=forest_end(d,AS_SOLID_HEADER);
  if((front&3)||(back&3)||front<begin||front>back||back>end)return AS_LAYOUT;
  h->kind=2;h->layout=AS_SOLID_HEADER;h->mutable_words[0]=front;h->mutable_words[1]=back;
  if(front>begin){rows[n].offset=begin-d->start;rows[n].extent=front-begin;rows[n].stored=front-begin;rows[n].kind=2;n++;}
  if(back<end){rows[n].offset=back-d->start;rows[n].extent=end-back;rows[n].stored=end-back;rows[n].kind=2;n++;}
  h->blocks=n;h->used=n;h->data_bytes=(front-begin)+(end-back);h->bytes=AS_HEADER+n*AS_ROW+h->data_bytes;h->valid=1;
  at=AS_HEADER+n*AS_ROW;
  for(i=0;i<n;i++){rows[i].data=at;at+=rows[i].stored;}
  return 0;
 }
 for(i=0;i<9;i++)h->mutable_words[i]=R32(d->heap,AS_MUTABLE+i*4);
 for(k=0;k<2;k++){
  head=d->heap+(k?0xa8:0x98);count=h->mutable_words[k?7:3];
  if(count>AS_MAX_BLOCKS-n||h->mutable_words[k?8:4])return AS_LIMIT;
  if(k)h->used=count;else h->free=count;
  node=h->mutable_words[k?6:2];prev=head;
  for(i=0;i<count;i++){
   z=block(d,node,k,rows+n);if(z)return z;
   if(R32(node,0)!=prev)return AS_LAYOUT;
   prev=node;node=R32(node,4);n++;
  }
  if(node!=head||prev!=h->mutable_words[k?5:1])return AS_LAYOUT;
 }
 if(!n)return AS_LAYOUT;
 /* Sort physical extents, independently of the two list orders. Requiring
  * an exact partition catches hidden, overlapping and unlisted blocks. */
 as_sort(n);
 for(i=0;i<n;i++){
  if(rows[i].offset!=at||rows[i].extent>d->span-at)return AS_LAYOUT;
  at+=rows[i].extent;
  if(rows[i].stored>0xffffffffu-h->data_bytes)return AS_LIMIT;
  h->data_bytes+=rows[i].stored;
 }
 if(at!=forest_end(d,AS_EXP_HEADER)-d->start||h->data_bytes>0xffffffffu-AS_HEADER-n*AS_ROW)return AS_LAYOUT;
 h->blocks=n;h->bytes=AS_HEADER+n*AS_ROW+h->data_bytes;h->valid=1;
 at=AS_HEADER+n*AS_ROW;
 for(i=0;i<n;i++){rows[i].data=at;at+=rows[i].stored;}
 return 0;
}
static U32 capture_locked(const AsDomain*d,void*out,U32 capacity,U32*written){AsHeader h;U32 z,i;
 z=domain(d);if(z)return z;
 if(!private_span((U32)out,capacity,d)||!private_span((U32)written,4,d)||
    !apart((U32)out,capacity,(U32)written,4)||!apart((U32)out,capacity,(U32)d,sizeof(*d))||
    !apart((U32)written,4,(U32)d,sizeof(*d)))return AS_ARGUMENT;
 *written=0;z=current_plan(d,&h);if(z)return z;if(h.bytes>capacity)return AS_SPACE;
 copy(out,&h,sizeof(h));copy((U8*)out+AS_HEADER,rows,h.blocks*AS_ROW);
 for(i=0;i<h.blocks;i++)forest_read((U8*)out+rows[i].data,d->start+rows[i].offset,rows[i].stored);
 if(domain(d))return AS_CHANGED;
 for(i=0;i<mutable_count(h.kind);i++)if(R32(d->heap,AS_MUTABLE+i*4)!=h.mutable_words[i])return AS_CHANGED;
 ((AsHeader*)out)->checksum=checksum(out,h.bytes);*written=h.bytes;return 0;
}
static AsRow* find(U32 offset,U32 n){U32 lo=0,hi=n,mid;
 while(lo<hi){mid=lo+(hi-lo)/2;if(rows[mid].offset==offset)return rows+mid;
 if(rows[mid].offset<offset)lo=mid+1;else hi=mid;}return 0;
}
static U32 validate_solid(const AsDomain*d,const U8*b,const AsHeader*h){U32 n=0,i,front=h->mutable_words[0],back=h->mutable_words[1];
 U32 begin=forest_begin(d,AS_SOLID_HEADER),end=forest_end(d,AS_SOLID_HEADER),data=AS_HEADER+h->blocks*AS_ROW;
 if((front&3)||(back&3)||front<begin||front>back||back>end||h->blocks>2||h->used!=h->blocks||h->free)return AS_CORRUPT;
 for(i=2;i<9;i++)if(h->mutable_words[i])return AS_CORRUPT;
 if(front>begin){
  if(n>=h->blocks||rows[n].offset!=begin-d->start||rows[n].extent!=front-begin)return AS_CORRUPT;
  n++;
 }
 if(back<end){
  if(n>=h->blocks||rows[n].offset!=back-d->start||rows[n].extent!=end-back)return AS_CORRUPT;
  n++;
 }
 if(n!=h->blocks)return AS_CORRUPT;
 for(i=0;i<n;i++){
  if(rows[i].kind!=2||rows[i].data!=data||rows[i].stored!=rows[i].extent||rows[i].stored>h->bytes-data)return AS_CORRUPT;
  data+=rows[i].stored;
 }
 (void)b;
 return data==h->bytes?0:AS_CORRUPT;
}
static U32 validate_unit(const AsDomain*d,const U8*b,const AsHeader*h){U32 n,i,z,at,node,k,free=0,first,stride;
 z=unit_shape(d,h,&n);if(z)return AS_CORRUPT;
 if(h->blocks!=n||h->used+h->free!=n||h->free!=h->mutable_words[3]/h->mutable_words[0])return AS_CORRUPT;
 for(i=6;i<9;i++)if(h->mutable_words[i])return AS_CORRUPT;
 first=h->mutable_words[1];stride=h->mutable_words[0];at=AS_HEADER+n*AS_ROW;
 for(i=0;i<n;i++){
  AsRow*r=rows+i;
  if(r->offset!=first-d->start+i*stride||r->extent!=stride||(r->kind!=0&&r->kind!=3)||
   r->stored!=(r->kind?stride:4)||r->data!=at||r->stored>h->bytes-at)return AS_CORRUPT;
  if(!r->kind)free++;
  at+=r->stored;
 }
 if(free!=h->free||at!=h->bytes)return AS_CORRUPT;
 node=h->mutable_words[4];
 for(i=0;i<free;i++){
  if(node<first||node-first>=n*stride||(node-first)%stride)return AS_CORRUPT;
  k=(node-first)/stride;if(rows[k].kind)return AS_CORRUPT;
  node=R32(b+rows[k].data,0);
 }
 return node?AS_CORRUPT:0;
}
static U32 validate_locked(const AsDomain*d,const void*image,U32 bytes,AsHeader*out){
 AsHeader h;U32 z,i,at=forest_begin(d,AS_EXP_HEADER)-d->start,data,kind,count,node,head,previous,j,size,pad;AsRow*r;const U8*b=(const U8*)image;
 z=domain(d);if(z)return z;
 if(bytes<AS_HEADER||!private_span((U32)image,bytes,d)||!apart((U32)image,bytes,(U32)d,sizeof(*d)))return AS_ARGUMENT;
 copy(&h,image,sizeof(h));
 if(h.magic!=AS_MAGIC||h.version!=AS_VERSION||h.valid!=1||h.bytes!=bytes||h.heap!=d->heap||h.start!=d->start||
  h.span!=d->span||h.heap_generation!=d->heap_generation||h.session!=d->session||
  h.kind!=heap_kind(forest_meta(d,0xc))||h.layout!=heap_header(h.kind)||
  (h.kind==1&&!h.blocks)||h.blocks>AS_MAX_BLOCKS||h.used>h.blocks||h.free!=h.blocks-h.used||h.blocks>(bytes-AS_HEADER)/AS_ROW||
  h.data_bytes!=bytes-AS_HEADER-h.blocks*AS_ROW||h.checksum!=checksum(image,bytes))return AS_CORRUPT;
 for(i=1;i<7;i++)if(h.reserved[i])return AS_CORRUPT;
 copy(rows,b+AS_HEADER,h.blocks*AS_ROW);data=AS_HEADER+h.blocks*AS_ROW;
 if(h.kind==2){z=validate_solid(d,b,&h);if(!z&&out)copy(out,&h,sizeof(h));return z;}
 if(h.kind==3){z=validate_unit(d,b,&h);if(!z&&out)copy(out,&h,sizeof(h));return z;}
 for(i=0;i<h.blocks;i++){
  r=rows+i;
  if(r->offset!=at||r->kind>1||r->extent<16||r->extent>d->span-at||r->data!=data||
   r->stored!=(r->kind?r->extent:16)||r->stored>bytes-data)return AS_CORRUPT;
  size=R32(b+r->data,8);pad=R32(b+r->data,12);
  if((size&3)||(pad&3)||size>r->extent-16||pad!=r->extent-16-size||(!r->kind&&pad))return AS_CORRUPT;
  if(r->kind&&pad&&R32(b+r->data,12+pad)!=((d->start+r->offset)|1u))return AS_CORRUPT;
  at+=r->extent;data+=r->stored;
 }
 if(at!=forest_end(d,AS_EXP_HEADER)-d->start||data!=bytes)return AS_CORRUPT;
 for(kind=0;kind<2;kind++){
  head=d->heap+(kind?0xa8:0x98);count=h.mutable_words[kind?7:3];
  if(count!=(kind?h.used:h.free)||h.mutable_words[kind?8:4])return AS_CORRUPT;
  node=h.mutable_words[kind?6:2];previous=head;
  for(j=0;j<count;j++){
   if(node<d->start)return AS_CORRUPT;
   r=find(node-d->start,h.blocks);
   if(!r||r->kind!=kind||R32(b+r->data,0)!=previous)return AS_CORRUPT;
   previous=node;node=R32(b+r->data,4);
  }
  if(node!=head||previous!=h.mutable_words[kind?5:1])return AS_CORRUPT;
 }
 if(out)copy(out,&h,sizeof(h));
 return 0;
}
static void apply(const AsDomain*d,const void*image,const AsHeader*h){U32 i;const U8*b=(const U8*)image;const AsRow*r=(const AsRow*)(b+AS_HEADER);
 for(i=0;i<h->blocks;i++)forest_write(d->start+r[i].offset,b+r[i].data,r[i].stored);
 /* The owner keeps the existing heap mutex, OS state and external registries.
  * Only original allocator policy/list words are republished, last. */
 for(i=0;i<mutable_count(h->kind);i++)*(volatile U32*)(d->heap+AS_MUTABLE+i*4)=h->mutable_words[i];
 __sync_synchronize();
}
static U32 matches(const AsDomain*d,const void*image,const AsHeader*h){U32 i;const U8*b=(const U8*)image;const AsRow*r=(const AsRow*)(b+AS_HEADER);
 for(i=0;i<mutable_count(h->kind);i++)if(R32(d->heap,AS_MUTABLE+i*4)!=h->mutable_words[i])return 0;
 for(i=0;i<h->blocks;i++)if(!forest_equal(d->start+r[i].offset,b+r[i].data,r[i].stored))return 0;
 return domain(d)==0;
}
