/* Pure metadata cases only. No owner/native/SDK/API result fixtures.
 * Exercises the actual exported pure metadata policy header. */
#include <assert.h>
#include <string.h>
#include "candidate/source/owner/source_intent_policy.h"

static SoIntentState initial(void){
 SoIntentState s={0};s.magic=SOI_MAGIC;s.bytes=SOI_BYTES;s.version=SOI_VERSION;
 s.inverse=~0u;s.seal=soi_seal_value(&s);return s;
}
static void reseal(SoIntentState*s){s->seal=soi_seal_value(s);}
int main(void){
 SoIntentState s=initial(),before,bad;uint32_t first,next,i;
 assert(sizeof(s)==56&&s.seal==0x220f0a7eu&&soi_shape(&s));
 for(i=0;i<14;i++){bad=s;((uint32_t*)&bad)[i]^=1u;assert(!soi_shape(&bad));}
 before=s;assert(!soi_arm(&s,0,0,1,1,216,0));assert(!memcmp(&s,&before,sizeof(s)));
 assert(!soi_arm(&s,1,10,1,1,216,0));assert(!soi_arm(&s,1,0,0,1,216,0));
 assert(!soi_arm(&s,1,0,1,0,216,0));assert(!soi_arm(&s,1,0,1,1,212,0));
 assert(!soi_arm(&s,1,0,1,1,217,0));assert(!soi_arm(&s,1,0,1,1,0x200004u,0));
 assert(!memcmp(&s,&before,sizeof(s)));
 first=soi_arm(&s,7,0,41,42,216,0);assert(first&&soi_shape(&s));
 assert(soi_matches(&s,first,7,0,41,42,216,0));
 assert(!soi_matches(&s,first+1,7,0,41,42,216,0));
 assert(!soi_matches(&s,first,8,0,41,42,216,0));
 assert(!soi_matches(&s,first,7,1,41,42,216,0));
 assert(!soi_matches(&s,first,7,0,43,42,216,0));
 assert(!soi_matches(&s,first,7,0,41,43,216,0));
 assert(!soi_matches(&s,first,7,0,41,42,220,0));
 assert(!soi_matches(&s,first,7,0,41,42,216,1));
 assert(!soi_write_allowed(&s,0));for(i=1;i<10;i++)assert(soi_write_allowed(&s,i));
 before=s;assert(!soi_arm(&s,8,9,43,44,0x200000u,99));assert(!memcmp(&s,&before,sizeof(s)));
 for(i=0;i<14;i++){bad=s;((uint32_t*)&bad)[i]^=1u;assert(!soi_shape(&bad));}
 bad=s;bad.slot=10;reseal(&bad);assert(!soi_shape(&bad));
 bad=s;bad.identity=0;reseal(&bad);assert(!soi_shape(&bad));
 bad=s;bad.generation=0;reseal(&bad);assert(!soi_shape(&bad));
 bad=s;bad.request=0;reseal(&bad);assert(!soi_shape(&bad));
 bad=s;bad.flags=1;reseal(&bad);assert(!soi_shape(&bad));
 bad=s;bad.cookie++;reseal(&bad);assert(!soi_shape(&bad));
 soi_clear_leaf(&s);assert(soi_shape(&s)&&s.serial==first&&s.inverse==~first);
 assert(!soi_matches(&s,first,7,0,41,42,216,0));
 for(i=0;i<10;i++)assert(soi_write_allowed(&s,i));
 next=soi_arm(&s,8,9,43,44,0x200000u,99);assert(next>first);
 assert(soi_matches(&s,next,8,9,43,44,0x200000u,99));
 assert(!soi_matches(&s,first,7,0,41,42,216,0));
 soi_clear_leaf(&s);s.serial=0xfffffffeu;s.inverse=~s.serial;reseal(&s);
 assert(soi_shape(&s));before=s;assert(!soi_arm(&s,9,9,45,46,216,1));
 assert(!memcmp(&s,&before,sizeof(s)));
 s.serial=0xffffffffu;s.inverse=~s.serial;reseal(&s);assert(!soi_shape(&s));
 return 0;
}
