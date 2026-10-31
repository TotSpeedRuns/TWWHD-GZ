#include "opaque_chunks.h"
#include "codec.h"
typedef uint32_t U;
typedef unsigned char B;
enum {OC_MAGIC=0x4f484331u,OC_HEADER=64,OC_ROW=32};
static U get(const B*p){return ((U)p[0]<<24)|((U)p[1]<<16)|((U)p[2]<<8)|p[3];}
static void put(B*p,U v){p[0]=(B)(v>>24);p[1]=(B)(v>>16);p[2]=(B)(v>>8);p[3]=(B)v;}
static U range(const void*p,U n){uintptr_t at=(uintptr_t)p;return at&&at<=UINTPTR_MAX-n;}
static U apart(const void*a,U n,const void*b,U m){uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y>=m:y-x>=n;}
static U pages(U n,U page){return n&&!(n&3u)&&n<=OC_MAX&&
 (page==OC_ENCODE_PAGE||page==OC_PAGE)?((n-1u)>>(page==OC_ENCODE_PAGE?16u:21u))+1u:0;}
static U aligned(U n){return (n+31u)&~31u;}
static U smaller(U a,U b){return a<b?a:b;}
static void zero(B*p,U n){U i;for(i=0;i<n;i++)p[i]=0;}
static void copy(B*d,const B*s,U n){U i;for(i=0;i<n;i++)d[i]=s[i];}
static U outword(const void*s,U n,U*out){return range(s,n)&&range(out,4)&&!((uintptr_t)out&3u)&&apart(s,n,out,4);}
static U kind_size(const B*s,U n,U*kind,U*stored){U measured=0,z=sc_measure(s,n,&measured);
 if(z==SC_OK){*kind=1;*stored=measured;return 1;}
 if(z==SC_KEEP_RAW){*kind=0;*stored=n;return 1;}return 0;}
U oc_measure(const void*source,U n,U*written){const B*s=source;U count=pages(n,OC_ENCODE_PAGE),i,total,kind,stored,raw;
 if(!count||!outword(source,n,written))return OC_ARGUMENT;
 *written=0;total=aligned(OC_HEADER+count*OC_ROW);
 for(i=0;i<count;i++){
  raw=smaller(OC_ENCODE_PAGE,n-i*OC_ENCODE_PAGE);
  if(!kind_size(s+i*OC_ENCODE_PAGE,raw,&kind,&stored))return OC_CORRUPT;
  total=aligned(total);if(stored>OC_MAX-total)return OC_SPACE;total+=stored;
 }
 *written=total;return OC_OK;}
U oc_encode(const void*source,U n,void*destination,U cap,U*written){const B*s=source;B*d=destination;
 U total=0,count=pages(n,OC_ENCODE_PAGE),i,z,kind,stored,raw,offset,next,encoded=0;B*row;
 if(!count||cap>OC_MAX||!outword(source,n,written)||!range(destination,cap)||
  !apart(source,n,destination,cap)||!apart(written,4,destination,cap))return OC_ARGUMENT;
 *written=0;z=oc_measure(source,n,&total);if(z!=OC_OK)return z;
 if(total>cap)return OC_SPACE;
 offset=aligned(OC_HEADER+count*OC_ROW);zero(d,offset);
 for(i=0;i<count;i++){
  next=aligned(offset);if(next>total)return OC_CORRUPT;zero(d+offset,next-offset);offset=next;
  raw=smaller(OC_ENCODE_PAGE,n-i*OC_ENCODE_PAGE);
  if(!kind_size(s+i*OC_ENCODE_PAGE,raw,&kind,&stored)||stored>total-offset)return OC_CORRUPT;
  if(kind){z=sc_encode(s+i*OC_ENCODE_PAGE,raw,d+offset,smaller(OC_ENCODE_PAGE,total-offset),&encoded);
   if(z!=SC_OK||encoded!=stored)return OC_CORRUPT;
  }else copy(d+offset,s+i*OC_ENCODE_PAGE,raw);
  row=d+OC_HEADER+i*OC_ROW;
  put(row,i*OC_ENCODE_PAGE);put(row+4,raw);put(row+8,offset);put(row+12,stored);
  put(row+16,kind);put(row+20,sc_hash(s+i*OC_ENCODE_PAGE,raw));put(row+24,sc_hash(d+offset,stored));
  offset+=stored;
 }
 if(offset!=total)return OC_CORRUPT;
 put(d,OC_MAGIC);put(d+4,1);put(d+8,n);put(d+12,OC_ENCODE_PAGE);put(d+16,count);
 put(d+20,OC_HEADER);put(d+24,aligned(OC_HEADER+count*OC_ROW));put(d+28,total);
 put(d+32,sc_hash(d+OC_HEADER,count*OC_ROW));put(d+48,1);
 put(d+60,sc_hash(d,60));*written=total;return OC_OK;}
static U header(const B*s,U bytes,U expected){U count,page;
 if(bytes<OC_HEADER||bytes>OC_MAX||!range(s,bytes))return 0;
 page=get(s+12);count=pages(expected,page);if(!count)return 0;
 if(get(s)!=OC_MAGIC||get(s+4)!=1||get(s+8)!=expected||get(s+12)!=page||
  get(s+16)!=count||get(s+20)!=OC_HEADER||get(s+24)!=aligned(OC_HEADER+count*OC_ROW)||
  get(s+28)!=bytes||get(s+24)>bytes||get(s+48)!=1||get(s+36)||get(s+40)||get(s+44)||
  get(s+52)||get(s+56)||get(s+60)!=sc_hash(s,60)||
  get(s+32)!=sc_hash(s+OC_HEADER,count*OC_ROW))return 0;
 return count;}
static U row_format(const B*s,U bytes,U expected,U index,U previous){const B*r=s+OC_HEADER+index*OC_ROW;
 U raw=get(r+4),offset=get(r+8),stored=get(r+12),kind=get(r+16),i,page=get(s+12);
 if(get(r)!=index*page||raw!=smaller(page,expected-index*page)||
  offset!=aligned(previous)||offset<get(s+24)||offset>bytes||stored>bytes-offset||get(r+28)||kind>1||
  (!kind&&stored!=raw)||(kind&&(stored<32||stored>=raw)))return 0;
 for(i=previous;i<offset;i++)if(s[i])return 0;
 if(get(r+24)!=sc_hash(s+offset,stored))return 0;
 if(!kind){if(get(r+20)!=get(r+24))return 0;}
 else if(sc_validate(s+offset,stored,raw,get(r+20))!=SC_OK)return 0;
 return 1;}
U oc_validate(const void*source,U bytes,U expected){const B*s=source;U count=header(s,bytes,expected),i,previous;
 if(!count)return OC_CORRUPT;previous=get(s+24);
 for(i=0;i<count;i++){
  const B*r=s+OC_HEADER+i*OC_ROW;
  if(!row_format(s,bytes,expected,i,previous))return OC_CORRUPT;
  previous=get(r+8)+get(r+12);
 }
 return previous==bytes?OC_OK:OC_CORRUPT;}
U oc_decode_page(const void*source,U bytes,U expected,U index,void*destination,U cap,U*written){
 const B*s=source,*row;U count,previous,raw,offset,stored,kind,z,valid=0;
 if(!outword(source,bytes,written)||cap>OC_PAGE||!range(destination,cap)||
  !apart(source,bytes,destination,cap)||!apart(written,4,destination,cap))return OC_ARGUMENT;
 *written=0;count=header(s,bytes,expected);if(!count||index>=count)return OC_CORRUPT;
 row=s+OC_HEADER+index*OC_ROW;
 if(index){U at=get(row-OC_ROW+8),n=get(row-OC_ROW+12);
  if(at>bytes||n>bytes-at)return OC_CORRUPT;previous=at+n;
 }else previous=get(s+24);
 if(previous>bytes||!row_format(s,bytes,expected,index,previous))return OC_CORRUPT;
 raw=get(row+4);offset=get(row+8);stored=get(row+12);kind=get(row+16);
 if(cap<raw)return OC_SPACE;
 if(kind){z=sc_decode(s+offset,stored,destination,cap,raw,get(row+20),&valid);
  if(z!=SC_OK||valid!=1)return OC_CORRUPT;
 }else copy(destination,s+offset,raw);
 *written=raw;return OC_OK;}
static U page_valid(const B*s,U bytes,U expected,U index){const B*row=s+OC_HEADER+index*OC_ROW;U previous;
 if(index){U at=get(row-OC_ROW+8),n=get(row-OC_ROW+12);
  if(at>bytes||n>bytes-at)return 0;previous=at+n;
 }else previous=get(s+24);
 return previous<=bytes&&row_format(s,bytes,expected,index,previous);
}
U oc_read(const void*source,U bytes,U expected,U offset,void*destination,U wanted){
 const B*s=source;B*d=destination;U count=header(s,bytes,expected),first,last,i,part,within,done=0,z;
 if(!count||!wanted||offset>expected||wanted>expected-offset||!range(destination,wanted)||
  !apart(source,bytes,destination,wanted))return OC_ARGUMENT;
 first=offset>>(get(s+12)==OC_ENCODE_PAGE?16u:21u);
 last=(offset+wanted-1u)>>(get(s+12)==OC_ENCODE_PAGE?16u:21u);
 for(i=first;i<=last;i++)if(!page_valid(s,bytes,expected,i))return OC_CORRUPT;
 for(i=first;i<=last;i++){
  const B*row=s+OC_HEADER+i*OC_ROW;within=(offset+done)-i*get(s+12);
  part=smaller(wanted-done,get(row+4)-within);
  if(get(row+16)){
   z=sc_read_range(s+get(row+8),get(row+12),get(row+4),get(row+20),within,d+done,part);
   if(z!=SC_OK)return OC_CORRUPT;
  }else copy(d+done,s+get(row+8)+within,part);
  done+=part;
 }
 return done==wanted?OC_OK:OC_CORRUPT;
}
U oc_matches(const void*source,U bytes,U expected,const void*raw){
 const B*s=source;const volatile B*r=raw;U count=header(s,bytes,expected),i,j,z;
 if(!count||!range(raw,expected)||!apart(source,bytes,raw,expected))return OC_ARGUMENT;
 if(oc_validate(source,bytes,expected)!=OC_OK)return OC_CORRUPT;
 for(i=0;i<count;i++){
  const B*row=s+OC_HEADER+i*OC_ROW;
  if(get(row+16)){
   z=sc_matches(s+get(row+8),get(row+12),get(row+4),get(row+20),(const void*)(r+get(row)));
   if(z!=SC_OK)return z==6?OC_DIFFERENT:OC_CORRUPT;
  }else for(j=0;j<get(row+4);j++)if(r[get(row)+j]!=s[get(row+8)+j])return OC_DIFFERENT;
 }
 return OC_OK;
}
