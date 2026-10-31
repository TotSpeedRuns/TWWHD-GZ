#include "codec.h"
typedef uint32_t U;typedef unsigned char B;
#define MAGIC 0x53524331u
static U get(const B*p){return ((U)p[0]<<24)|((U)p[1]<<16)|((U)p[2]<<8)|p[3];}
static void put(B*p,U v){p[0]=(B)(v>>24);p[1]=(B)(v>>16);p[2]=(B)(v>>8);p[3]=(B)v;}
static U range(const void*p,U n){uintptr_t a=(uintptr_t)p;return a&&a<=UINTPTR_MAX-n;}
static U apart(const void*a,U n,const void*b,U m){uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y>=m:y-x>=n;}
static U output(const void*s,U n,void*d,U cap,U*out){return n<=SC_MAX&&cap<=SC_MAX&&range(s,n)&&range(d,cap)&&
 range(out,4)&&!((uintptr_t)out&3)&&apart(s,n,d,cap)&&apart(out,4,s,n)&&apart(out,4,d,cap);}
static U addhash(U h,B b){return (h^b)*16777619u;}
U sc_hash(const void*p,U n){const B*s=(const B*)p;U h=2166136261u,i;
 if(n>SC_MAX||!range(p,n))return 0;
 for(i=0;i<n;i++)h=addhash(h,s[i]);
 return h;}
/* For zero bytes FNV-1a is multiplication by the prime. Unsigned
 * arithmetic preserves the original modulo-2^32 result exactly. */
static U zerohash(U h,U bytes){U factor=16777619u;
 while(bytes){if(bytes&1u)h*=factor;bytes>>=1;factor*=factor;}
 return h;}
static U run(const B*s,U words,U at){U n=1,v=get(s+at*4);while(n<words-at&&get(s+(at+n)*4)==v)n++;return n;}
static void block(const B*s,U words,U at,U*count,U*kind){U n=run(s,words,at),end;
 if(!get(s+at*4)&&n>=2){*count=n;*kind=2;return;}
 if(n>=3){*count=n;*kind=1;return;}
 end=at+n;while(end<words){n=run(s,words,end);if((!get(s+end*4)&&n>=2)||n>=3)break;end+=n;}
 *count=end-at;*kind=0;}
U sc_encode(const void*source,U n,void*destination,U cap,U*written){const B*s=(const B*)source;B*d=(B*)destination;
 U words,i=0,count,kind,payload=0,need,total,j,at;
 if(!output(source,n,destination,cap,written))return SC_BAD_ARGUMENT;
 *written=0;if(n&3)return SC_KEEP_RAW;words=n/4;
 while(i<words){block(s,words,i,&count,&kind);payload+=4+(kind==0?count*4:kind==1?4:0);i+=count;}
 if(payload>=n||32>=n-payload)return SC_KEEP_RAW;
 total=32+payload;
 if(total>cap)return SC_SPACE;
 i=0;at=32;
 while(i<words){block(s,words,i,&count,&kind);need=4+(kind==0?count*4:kind==1?4:0);
  if(at>total||need>total-at)return SC_CORRUPT;
  put(d+at,(kind<<30)|count);at+=4;
  if(kind==0){for(j=0;j<count*4;j++)d[at+j]=s[i*4+j];at+=count*4;}
  else if(kind==1){for(j=0;j<4;j++)d[at+j]=s[i*4+j];at+=4;}
  i+=count;
 }
 if(at!=total)return SC_CORRUPT;
 put(d,MAGIC);put(d+4,1);put(d+8,n);put(d+12,payload);put(d+16,sc_hash(s,n));put(d+20,n^0xffffffffu);
 put(d+24,sc_hash(d+32,payload));put(d+28,sc_hash(d,28));
 *written=total;return SC_OK;
}
/* Parse and hash before the first output store. On a stable malformed input,
 * destination is unchanged. Repeat all bounds during emission as protection
 * against a caller violating the immutable-input contract; validity stays0. */
static U walk(const B*s,U encoded,B*d,U expected,U emit,U*hash){U at=32,out=0,token,kind,count,bytes,j,k,v,h=2166136261u;
 while(at<encoded){if(encoded-at<4)return 0;token=get(s+at);at+=4;kind=token>>30;count=token&0x3fffffffu;
  if(!count||kind>2||count>(expected-out)/4)return 0;
  bytes=count*4;
  if(kind==0){if(bytes>encoded-at)return 0;
   for(j=0;j<bytes;j++){if(emit)d[out+j]=s[at+j];else h=addhash(h,s[at+j]);}at+=bytes;
  }else if(kind==1){if(encoded-at<4)return 0;v=get(s+at);at+=4;
   for(j=0;j<count;j++)for(k=0;k<4;k++){B b=(B)(v>>(24-k*8));if(emit)d[out+j*4+k]=b;else h=addhash(h,b);}
  }else if(emit){for(j=0;j<bytes;j++)d[out+j]=0;}else h=zerohash(h,bytes);
  out+=bytes;
 }
 if(at!=encoded||out!=expected)return 0;
 *hash=h;return 1;
}
U sc_decode(const void*source,U encoded,void*destination,U cap,U expected,U expected_hash,U*valid){const B*s=(const B*)source;
 U h=0,ignored;
 if(!output(source,encoded,destination,cap,valid))return SC_BAD_ARGUMENT;
 *valid=0;
 if(expected>SC_MAX||!expected||(expected&3)||encoded<32||encoded>=expected)return SC_CORRUPT;
 if(cap<expected)return SC_SPACE;
 if(get(s)!=MAGIC||get(s+4)!=1||get(s+8)!=expected||get(s+12)!=encoded-32||get(s+16)!=expected_hash||
    get(s+20)!=(expected^0xffffffffu)||get(s+24)!=sc_hash(s+32,encoded-32)||get(s+28)!=sc_hash(s,28))return SC_CORRUPT;
 if(!walk(s,encoded,(B*)destination,expected,0,&h)||h!=expected_hash)return SC_CORRUPT;
 if(!walk(s,encoded,(B*)destination,expected,1,&ignored))return SC_CORRUPT;
 *valid=1;return SC_OK;
}

/* Same block grammar as sc_encode, without a guessed destination allocation.
 * The at-most2MiB source limits every count/product and total below2^32.
 * This grants no source authority and never changes an immutable input. */
U sc_measure(const void*source,U n,U*encoded){const B*s=(const B*)source;
 U words,i=0,count,kind,payload=0;
 if(n>SC_MAX||!range(source,n)||!range(encoded,4)||((uintptr_t)encoded&3)||!apart(encoded,4,source,n))return SC_BAD_ARGUMENT;
 *encoded=0;
 if(n&3)return SC_KEEP_RAW;
 words=n/4;
 while(i<words){block(s,words,i,&count,&kind);payload+=4+(kind==0?count*4:kind==1?4:0);i+=count;}
 if(payload>=n||32>=n-payload)return SC_KEEP_RAW;
 *encoded=32+payload;return SC_OK;
}

/* Pure format-only validation of the existing SRC1 grammar. No output stores,
 * decoder scratch allocation, native pointer following or authority. */
U sc_validate(const void*source,U encoded,U expected,U expected_hash){
 const B*s=(const B*)source;U h=0;
 if(expected>SC_MAX||!expected||(expected&3)||encoded<32||encoded>=expected||
  encoded>SC_MAX||!range(source,encoded))return SC_CORRUPT;
 if(get(s)!=MAGIC||get(s+4)!=1||get(s+8)!=expected||get(s+12)!=encoded-32||
  get(s+16)!=expected_hash||get(s+20)!=(expected^0xffffffffu)||
  get(s+24)!=sc_hash(s+32,encoded-32)||get(s+28)!=sc_hash(s,28))return SC_CORRUPT;
 return walk(s,encoded,0,expected,0,&h)&&h==expected_hash?SC_OK:SC_CORRUPT;
}
/* Bounded projection/equality over the same existing token grammar. Source
 * immutability is the caller's contract, not something a codec can grant. */
U sc_read_range(const void*source,U encoded,U expected,U expected_hash,U offset,void*destination,U bytes){
 const B*s=source;B*d=destination;U at=32,out=0,token,kind,count,n,j,begin,end;
 if(!bytes||offset>expected||bytes>expected-offset||!range(destination,bytes)||
  !apart(source,encoded,destination,bytes))return SC_BAD_ARGUMENT;
 if(sc_validate(source,encoded,expected,expected_hash)!=SC_OK)return SC_CORRUPT;
 while(at<encoded){
  token=get(s+at);at+=4;kind=token>>30;count=token&0x3fffffffu;n=count*4;
  begin=out>offset?out:offset;end=out+n<offset+bytes?out+n:offset+bytes;
  for(j=begin;j<end;j++)d[j-offset]=kind==0?s[at+j-out]:kind==1?s[at+(j-out)%4]:0;
  at+=kind==0?n:kind==1?4:0;out+=n;
 }
 return SC_OK;
}
U sc_matches(const void*source,U encoded,U expected,U expected_hash,const void*raw){
 const B*s=source;const volatile B*r=raw;U at=32,out=0,token,kind,count,n,j;
 if(!range(raw,expected)||!apart(source,encoded,raw,expected))return SC_BAD_ARGUMENT;
 if(sc_validate(source,encoded,expected,expected_hash)!=SC_OK)return SC_CORRUPT;
 while(at<encoded){
  token=get(s+at);at+=4;kind=token>>30;count=token&0x3fffffffu;n=count*4;
  for(j=0;j<n;j++)if(r[out+j]!=(kind==0?s[at+j]:kind==1?s[at+j%4]:0))return 6;
  at+=kind==0?n:kind==1?4:0;out+=n;
 }
 return SC_OK;
}
