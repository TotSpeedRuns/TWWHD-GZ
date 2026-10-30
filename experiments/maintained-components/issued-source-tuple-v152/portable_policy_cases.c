#include <stdint.h>
#include <stdio.h>
typedef uint32_t u32;
typedef unsigned long long GzSourceDigest;
enum {GZ_DIGEST_FULL=0,GZ_DIGEST_ISSUED=1,GZ_DIGEST_REFUSE=2};
static struct {u32 token,frame,revision,pin,source,used,identity,generation;} wct;
static u32 wct_entered;
static volatile u32 _gz_pbac_frame,_gz_roh_registry_revision;
static u32 mode,low_hash,native_calls,digest_calls,getter_calls,fail_getter,changed_native,changed_revision,changed_frame;
static u32 _gz_wct_native_owned(void){return ++native_calls==2&&changed_native?10:9;}
static GzSourceDigest _gz_osa_verified_source_digest(u32 p,u32 s,u32 n,u32 id,u32 gen,u32 rev){
 (void)p;(void)s;(void)n;(void)id;(void)gen;(void)rev;++digest_calls;
 if(changed_revision)++_gz_roh_registry_revision;if(changed_frame)++_gz_pbac_frame;
 return ((GzSourceDigest)mode<<32)|low_hash;
}
static u32 next_getter(void){return getter_calls++!=fail_getter;}
static u32 _gz_osa_validate(u32 p){(void)p;return !next_getter();}
static u32 _gz_osa_source(u32 p){(void)p;return next_getter()?wct.source:0;}
static u32 _gz_osa_bytes(u32 p){(void)p;return next_getter()?wct.used:0;}
static u32 _gz_osa_pin_identity(u32 p){(void)p;return next_getter()?wct.identity:0;}
static u32 _gz_osa_pin_generation(u32 p){(void)p;return next_getter()?wct.generation:0;}

static u32 wct_source_tuple_current(u32 token,u32 frame)
{
 u32 parent;
 if(wct_entered==1&&token&&wct.token==token&&wct.frame==frame&&
  frame==_gz_pbac_frame&&wct.revision&&
  wct.revision==*(volatile u32*)&_gz_roh_registry_revision&&
  (parent=_gz_wct_native_owned())!=0){
  GzSourceDigest value=_gz_osa_verified_source_digest(wct.pin,wct.source,wct.used,
      wct.identity,wct.generation,wct.revision);
  /* Only the upper state selects the aggregate result. Hash0 is valid;
   * the old tuple clause never compared a sourcebody hash here. */
  if((u32)(value>>32)==GZ_DIGEST_ISSUED&&_gz_wct_native_owned()==parent&&
   frame==_gz_pbac_frame&&wct.frame==frame&&
   wct.revision==*(volatile u32*)&_gz_roh_registry_revision)return 1;
 }
 return !_gz_osa_validate(wct.pin)&&_gz_osa_source(wct.pin)==wct.source&&
  _gz_osa_bytes(wct.pin)==wct.used&&_gz_osa_pin_identity(wct.pin)==wct.identity&&
  _gz_osa_pin_generation(wct.pin)==wct.generation;
}

static u32 checks,failed;
#define CHECK(x) do{++checks;if(!(x)){++failed;printf("FAIL line %d\n",__LINE__);}}while(0)
static void reset(void){
 wct.token=3;wct.frame=5;wct.revision=7;wct.pin=2;wct.source=0x20000000;wct.used=1024;wct.identity=11;wct.generation=13;
 wct_entered=1;_gz_pbac_frame=5;_gz_roh_registry_revision=7;mode=1;low_hash=0;
 native_calls=digest_calls=getter_calls=changed_native=changed_revision=changed_frame=0;fail_getter=99;
}
int main(void){u32 i,j,z;
 reset();z=wct_source_tuple_current(3,5);CHECK(z==1&&digest_calls==1&&native_calls==2&&!getter_calls);
 reset();low_hash=0xffffffff;z=wct_source_tuple_current(3,5);CHECK(z==1&&!getter_calls);
 for(i=0;i<4;i++)if(i!=1){reset();mode=i;z=wct_source_tuple_current(3,5);CHECK(z==1&&getter_calls==5&&native_calls==1&&digest_calls==1);}
 for(i=0;i<4;i++)for(j=0;j<5;j++)if(i!=1){reset();mode=i;fail_getter=j;z=wct_source_tuple_current(3,5);CHECK(z==0&&getter_calls==j+1);}
 reset();wct.revision=0;z=wct_source_tuple_current(3,5);CHECK(z==1&&getter_calls==5&&!digest_calls&&!native_calls);
 reset();_gz_roh_registry_revision=8;z=wct_source_tuple_current(3,5);CHECK(z==1&&getter_calls==5&&!digest_calls);
 reset();wct_entered=0;z=wct_source_tuple_current(3,5);CHECK(z==1&&getter_calls==5&&!digest_calls);
 reset();z=wct_source_tuple_current(4,5);CHECK(z==1&&getter_calls==5&&!digest_calls);
 reset();z=wct_source_tuple_current(3,6);CHECK(z==1&&getter_calls==5&&!digest_calls);
 reset();changed_native=1;z=wct_source_tuple_current(3,5);CHECK(z==1&&getter_calls==5&&native_calls==2);
 reset();changed_revision=1;z=wct_source_tuple_current(3,5);CHECK(z==1&&getter_calls==5&&native_calls==2);
 reset();changed_frame=1;z=wct_source_tuple_current(3,5);CHECK(z==1&&getter_calls==5&&native_calls==2);
 printf("checks=%u failed=%u\n",checks,failed);return failed?1:0;
}
