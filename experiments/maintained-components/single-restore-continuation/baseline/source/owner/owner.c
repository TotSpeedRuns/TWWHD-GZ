#include "owner.h"
#include "source_work.h"
#include "codec.h"
/* Appended source-active guard declaration; definition follows owner functions. */
U so_active_write_apart(U,U);
typedef struct {U mode,pointer,capacity,stored,heap,used,identity,generation,origin,hash,codec,valid;SoPresentation presentation;U seal;} Row;
typedef struct {U pointer,capacity,heap,used,slot,identity,generation,hash,seal;} Cache;
typedef struct {U active,serial,slot,pointer,capacity,used,identity,generation,heap;} Lease;
typedef struct {U magic,version,serial,inverse,entered,writer,quarantine;Row rows[10];Cache cache;Lease lease;U view[10];} Owner;
typedef struct {U active,slot,frame,used,origin,identity,generation,raw,raw_capacity,raw_heap,raw_freeable,encoded,encoded_capacity,encoded_heap,encoded_freeable,encoded_used,raw_receipt,encoded_receipt;Row old;SoPresentation presentation;} Tx;
/* Immutable stack witness across callbacks. Includes the exact issuing
 * receipts, every logical row, both serializer-plan representations and RAW
 * aliases. No callback may rewrite these private fields as an admission. */
typedef struct {Owner owner;Tx tx;U snapshot[120],live[120],views[10],origins[20],origin;} Boundary;
static Owner owner={.magic=0x534f5731,.version=1,.serial=1,.inverse=~1u};
static Tx tx;
static U span(U p,U n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static U apart(U p,U n,U q,U m){return n<=0xffffffffu-p&&m<=0xffffffffu-q&&(p+n<=q||q+m<=p);}
static U cap(U n){return n>=216&&n<=SO_MAX&&!(n&3)?(n+31u)&~31u:0;}
static void copy(void*d,const void*s,U n){U i;for(i=0;i<n;i++)((unsigned char*)d)[i]=((const unsigned char*)s)[i];}
static void zero(void*p,U n){U i;for(i=0;i<n;i++)((unsigned char*)p)[i]=0;}
static U same(const void*a,const void*b,U n){U i;for(i=0;i<n;i++)if(((const unsigned char*)a)[i]!=((const unsigned char*)b)[i])return 0;return 1;}
static U hash(const void*p,U n){U h=2166136261u,i;for(i=0;i<n;i++)h=(h^((const unsigned char*)p)[i])*16777619u;return h;}
static U pshape(const SoPresentation*p){U i;if(p->label_bytes>127||p->detail_bytes>127||p->room>255||p->layer>255)return 0;
 if(p->label[p->label_bytes]||p->detail[p->detail_bytes])return 0;
 for(i=p->label_bytes+1;i<128;i++)if(p->label[i])return 0;
 for(i=p->detail_bytes+1;i<128;i++)if(p->detail[i])return 0;return 1;}
static U row_ok(const Row*r){
 if(!r->mode){const U*q=(const U*)r;U i;for(i=0;i<sizeof(*r)/4;i++)if(q[i])return 0;return 1;}
 return (r->mode==SO_RAW||r->mode==SO_RLE)&&r->valid==1&&r->identity&&r->generation&&cap(r->used)&&
  r->stored&&r->stored<=r->capacity&&r->capacity==((r->stored+31u)&~31u)&&span(r->pointer,r->capacity)&&!(r->pointer&31)&&span(r->heap,4)&&
  ((r->mode==SO_RAW&&r->stored==r->used&&!r->codec)||(r->mode==SO_RLE&&r->codec==1&&r->stored>=32&&r->stored<r->used&&r->capacity<cap(r->used)))&&
  pshape(&r->presentation)&&r->seal==hash(r,sizeof(*r)-4);
}
/* This set is pure metadata. Foreign admission is a separate boundary below:
 * an external private-set query can itself call the OSA whole-owner accessor,
 * so calling it while validating a retained row would recurse. Issued rows
 * retain their admission receipt; foreign allocators must exclude our complete
 * set before publishing any new region of their own. */
static U meta_apart(U p,U n){return apart(p,n,(U)&swc,sizeof(swc))&&apart(p,n,(U)&owner,sizeof(owner))&&apart(p,n,(U)&tx,sizeof(tx))&&
 apart(p,n,(U)_gz_slots_plan,480)&&apart(p,n,(U)&_gz_slot_origin_captured,4)&&
 apart(p,n,(U)_gz_slots_ptrs,40)&&apart(p,n,(U)_gz_slot_origin_sidecars,80)&&apart(p,n,(U)_gz_osa_plan_snapshot,480);}
static U all_ok(void){U i,j;Cache*c=&owner.cache;
 if(owner.magic!=0x534f5731||owner.version!=1||!owner.serial||owner.inverse!=~owner.serial||owner.quarantine||owner.entered>1||owner.writer>1||owner.lease.active>1||tx.active!=owner.writer)return 0;
 for(i=0;i<10;i++){const Row*r=&owner.rows[i];if(!row_ok(r))return 0;
  if(r->mode&&!meta_apart(r->pointer,r->capacity))return 0;
  if(owner.view[i]!=(owner.lease.active&&owner.lease.slot==i?owner.lease.pointer:0))return 0;
  for(j=0;j<i;j++)if(r->mode&&owner.rows[j].mode&&!apart(r->pointer,r->capacity,owner.rows[j].pointer,owner.rows[j].capacity))return 0;
 }
 if(c->pointer){if(!span(c->pointer,c->capacity)||(c->pointer&31)||!span(c->heap,4)||c->capacity!=cap(c->used)||!meta_apart(c->pointer,c->capacity)||c->seal!=hash(c,sizeof(*c)-4))return 0;
  for(i=0;i<10;i++)if(owner.rows[i].mode&&!apart(c->pointer,c->capacity,owner.rows[i].pointer,owner.rows[i].capacity))return 0;
 }else if(c->capacity|c->heap|c->used|c->slot|c->identity|c->generation|c->hash|c->seal)return 0;
 if(tx.raw){
  if(tx.raw_freeable!=1||tx.raw_capacity!=cap(tx.used)||!span(tx.raw,tx.raw_capacity)||(tx.raw&31)||!span(tx.raw_heap,4)||!meta_apart(tx.raw,tx.raw_capacity)||tx.raw_receipt!=hash(&tx.raw,16))return 0;
  for(i=0;i<10;i++)if(owner.rows[i].mode&&!apart(tx.raw,tx.raw_capacity,owner.rows[i].pointer,owner.rows[i].capacity))return 0;
  if(c->pointer&&!apart(tx.raw,tx.raw_capacity,c->pointer,c->capacity))return 0;
 }else if(tx.raw_capacity|tx.raw_heap|tx.raw_freeable|tx.raw_receipt)return 0;
 if(tx.encoded){
  if(!tx.raw||tx.encoded_freeable!=1||tx.encoded_capacity<32||tx.encoded_capacity>=tx.raw_capacity||(tx.encoded_capacity&31)||!span(tx.encoded,tx.encoded_capacity)||(tx.encoded&31)||!span(tx.encoded_heap,4)||!meta_apart(tx.encoded,tx.encoded_capacity)||tx.encoded_receipt!=hash(&tx.encoded,16)||tx.encoded_used>tx.encoded_capacity)return 0;
  if(!apart(tx.encoded,tx.encoded_capacity,tx.raw,tx.raw_capacity))return 0;
  for(i=0;i<10;i++)if(owner.rows[i].mode&&!apart(tx.encoded,tx.encoded_capacity,owner.rows[i].pointer,owner.rows[i].capacity))return 0;
  if(c->pointer&&!apart(tx.encoded,tx.encoded_capacity,c->pointer,c->capacity))return 0;
 }else if(tx.encoded_capacity|tx.encoded_heap|tx.encoded_freeable|tx.encoded_used|tx.encoded_receipt)return 0;
 return 1;
}
/* Called only after all_ok; does not recursively validate a candidate as an
 * external span. New allocations are admitted against every OTHER owner. */
static U fresh(U p,U n){U i;if(!span(p,n)||(p&31)||!meta_apart(p,n))return 0;
 for(i=0;i<10;i++)if(owner.rows[i].mode&&!apart(p,n,owner.rows[i].pointer,owner.rows[i].capacity))return 0;
 if(owner.cache.pointer&&!apart(p,n,owner.cache.pointer,owner.cache.capacity))return 0;
 if(tx.raw&&!apart(p,n,tx.raw,tx.raw_capacity))return 0;
 if(tx.encoded&&!apart(p,n,tx.encoded,tx.encoded_capacity))return 0;
 for(i=0;i<30;i++)if(tx.active&&_gz_osa_plan_snapshot[4*i+1]&&!apart(p,n,_gz_osa_plan_snapshot[4*i],_gz_osa_plan_snapshot[4*i+1]))return 0;
 /* Callback is reached only after every owned/plan overlap is rejected. It
  * may inspect our pure physical set but cannot admit a source or mutation.
  * The caller remains entered until an immutable issuing receipt is stored. */
 return so_native_private(p,n);
}
static U total(const volatile U*p){U i,n,a,t=256;
 for(i=0;i<30;i++){a=p[4*i];n=p[4*i+1];if((n&3)||n>p[4*i+3]||n>SO_MAX-t||(n&&!span(a,n)))return 0;t+=n;}return t;}
static U current(void){U i;if(!tx.active||tx.slot>9||!same(&tx.old,&owner.rows[tx.slot],sizeof(Row))||tx.origin!=_gz_slot_origin_captured)return 0;
 for(i=0;i<120;i++)if(_gz_osa_plan_snapshot[i]!=_gz_slots_plan[i])return 0;return total(_gz_osa_plan_snapshot)==tx.used;}
static U writer(U slot,U frame){return all_ok()&&owner.writer&&tx.active&&slot==tx.slot&&frame==tx.frame&&slot<10&&!owner.lease.active&&current();}
static U issue(void){owner.serial++;owner.inverse=~owner.serial;return owner.serial;}
static void boundary_take(Boundary*b){copy(&b->owner,&owner,sizeof(owner));copy(&b->tx,&tx,sizeof(tx));
 copy(b->snapshot,_gz_osa_plan_snapshot,480);copy(b->live,(const void*)_gz_slots_plan,480);
 copy(b->views,(const void*)_gz_slots_ptrs,40);copy(b->origins,(const void*)_gz_slot_origin_sidecars,80);b->origin=_gz_slot_origin_captured;}
static U boundary_same(const Boundary*b){return same(&b->owner,&owner,sizeof(owner))&&same(&b->tx,&tx,sizeof(tx))&&
 same(b->snapshot,_gz_osa_plan_snapshot,480)&&same(b->live,(const void*)_gz_slots_plan,480)&&
 same(b->views,(const void*)_gz_slots_ptrs,40)&&same(b->origins,(const void*)_gz_slot_origin_sidecars,80)&&b->origin==_gz_slot_origin_captured;}
static U boundary_fault(U p){owner.quarantine=p?p:0xffffffffu;return SO_BAD;}
static U evict(Boundary*b){U p=owner.cache.pointer,h=owner.cache.heap;
 /* Keep the exact issued cache represented through the final free callback.
  * owner.entered excludes readers/writers; pure foreign span queries must
  * still reject this retired storage until its issuing free returns. */
 if(p){boundary_take(b);so_native_free(h,p);if(!boundary_same(b))return boundary_fault(p);}
 zero(&owner.cache,sizeof(Cache));return 0;}
U so_physical_apart(U p,U n){U i;if(!all_ok()||!span(p,n)||!meta_apart(p,n))return 0;
 for(i=0;i<10;i++)if(owner.rows[i].mode&&!apart(p,n,owner.rows[i].pointer,owner.rows[i].capacity))return 0;
 return (!owner.cache.pointer||apart(p,n,owner.cache.pointer,owner.cache.capacity))&&(!tx.raw||apart(p,n,tx.raw,tx.raw_capacity))&&(!tx.encoded||apart(p,n,tx.encoded,tx.encoded_capacity));}
U so_begin(U slot,U frame,U presentation){U i,n,z;Boundary held;SoPresentation*m=(SoPresentation*)presentation;
 if(!all_ok()||slot>9||!frame||(frame&7)||!span(presentation,sizeof(*m))||!so_physical_apart(presentation,sizeof(*m)))return SO_BAD;
 if(owner.entered||owner.writer||owner.lease.active)return SO_BUSY;
 boundary_take(&held);z=so_native_guard(1);if(!boundary_same(&held))return boundary_fault(0);if(z)return SO_BUSY;
 if(owner.serial>0xfffffffcu)return SO_SERIAL;
 if(!(n=total(_gz_slots_plan))||!pshape(m))return SO_BAD;
 owner.entered=1;if(evict(&held))return SO_BAD;zero(&tx,sizeof(tx));tx.slot=slot;tx.frame=frame;tx.used=n;tx.origin=_gz_slot_origin_captured;
 copy(&tx.old,&owner.rows[slot],sizeof(Row));copy(&tx.presentation,m,sizeof(*m));
 for(i=0;i<120;i++)_gz_osa_plan_snapshot[i]=_gz_slots_plan[i];tx.identity=issue();tx.generation=issue();tx.active=owner.writer=1;
 if(!current()){zero(&tx,sizeof(tx));owner.writer=owner.entered=0;return SO_BAD;}owner.entered=0;return 0;
}
U so_prepare(U slot,U frame){U h,p,c,z;Boundary held;if(!writer(slot,frame)||owner.entered||tx.raw)return SO_BAD;
 owner.entered=1;c=cap(tx.used);boundary_take(&held);h=so_native_base(1);
 if(!boundary_same(&held))return boundary_fault(0);
 if(!span(h,4)){owner.entered=0;return h?SO_BAD:SO_NOMEM;}
 p=so_native_allocate(h,c,32);if(!boundary_same(&held))return boundary_fault(p);
 if(!p){owner.entered=0;return SO_NOMEM;}
 z=fresh(p,c);if(!boundary_same(&held))return boundary_fault(p);
 if(!z)return boundary_fault(p);
 tx.raw=p;tx.raw_heap=h;tx.raw_capacity=c;tx.raw_freeable=1;tx.raw_receipt=hash(&tx.raw,16);owner.entered=0;return current()?0:SO_BAD;
}
U so_destination(U slot,U frame){return writer(slot,frame)&&!owner.entered&&tx.raw_freeable?tx.raw:0;}
U so_writer_bytes(U slot,U frame){return writer(slot,frame)&&!owner.entered&&tx.raw_freeable?tx.used:0;}
U so_writer_capacity(U slot,U frame){return writer(slot,frame)&&!owner.entered&&tx.raw_freeable?tx.raw_capacity:0;}
U so_present(U slot,U frame,U presentation){SoPresentation*m=(SoPresentation*)presentation;
 if(!writer(slot,frame)||owner.entered||!tx.raw_freeable||!span(presentation,sizeof(*m))||
  !so_physical_apart(presentation,sizeof(*m))||!pshape(m))return SO_BAD;
 copy(&tx.presentation,m,sizeof(*m));return 0;
}
U so_present_raw(U slot,U frame,U raw,U used){Boundary held;SoPresentation m;U z,o;
 if(!writer(slot,frame)||owner.entered||!tx.raw_freeable||raw!=tx.raw||used!=tx.used||used<256)return SO_BAD;
 owner.entered=1;boundary_take(&held);z=so_native_validate(raw,used,held.tx.raw_capacity);
 if(!boundary_same(&held))return boundary_fault(0);
 if(z||((const U*)raw)[5]!=20){owner.entered=0;return SO_BAD;}
 o=((const U*)raw)[4];zero(&m,sizeof(m));
 /* Bounded descriptor data only: label/callback audit pointers are omitted. */
 m.stage0=((const U*)(raw+o))[1];m.stage1=((const U*)(raw+o))[2];
 m.room=(((const U*)(raw+o))[3]>>8)&255;m.layer=((const U*)(raw+o))[3]&255;
 copy(&tx.presentation,&m,sizeof(m));owner.entered=0;return 0;
}
U so_plan_bounds(void){
 if(!all_ok()||owner.entered||owner.writer||owner.lease.active)return SO_BAD;
 return total(_gz_slots_plan)?0:SO_BAD;
}
static U image(U p,U n,U c){return cap(n)&&c==cap(n)&&!so_native_validate(p,n,c);}
U so_publish(U slot,U frame){U z,encoded=0,written=0,valid=0,h,p,c,rh,oldp,oldh,rawp,rawh;Row next,*r;Boundary held;
 if(!writer(slot,frame)||owner.entered||!tx.raw_freeable)return SO_BAD;
 owner.entered=1;boundary_take(&held);
 /* Only already admitted RAW storage/extent is hashed. This immutable local
  * receipt, not a callback-mutated tx field, supplies every later body access. */
 rh=sc_hash((void*)held.tx.raw,held.tx.used);
 z=image(held.tx.raw,held.tx.used,held.tx.raw_capacity);
 if(!boundary_same(&held))return boundary_fault(0);if(!z)goto bad;
 z=so_native_guard(1);if(!boundary_same(&held))return boundary_fault(0);if(z||!current())goto bad;
 z=sc_measure((void*)held.tx.raw,held.tx.used,&encoded);
 if(!boundary_same(&held))return boundary_fault(0);
 if(z!=SC_OK&&z!=SC_KEEP_RAW)goto bad;
 if(z==SC_OK&&((encoded+31u)&~31u)<held.tx.raw_capacity){
  c=(encoded+31u)&~31u;h=so_native_base(1);
  if(!boundary_same(&held))return boundary_fault(0);
  if(h&&!span(h,4))goto bad;
  p=h?so_native_allocate(h,c,32):0;
  if(!boundary_same(&held))return boundary_fault(p);
  if(p){
   z=fresh(p,c);if(!boundary_same(&held))return boundary_fault(p);if(!z)return boundary_fault(p);
   tx.encoded=p;tx.encoded_heap=h;tx.encoded_capacity=c;tx.encoded_freeable=1;tx.encoded_receipt=hash(&tx.encoded,16);
   boundary_take(&held);
   z=sc_encode((void*)held.tx.raw,held.tx.used,(void*)p,c,&written);
   if(!boundary_same(&held))return boundary_fault(0);if(z!=SC_OK||written!=encoded)goto bad;
   /* Verification may destroy the unpublished stage on malformed output.
    * Every failure after this point aborts cleanly; never RAW-fallback from
    * a damaged stage. Existing old backing remains untouched throughout. */
   z=sc_decode((void*)p,written,(void*)held.tx.raw,held.tx.raw_capacity,held.tx.used,rh,&valid);
   if(!boundary_same(&held))return boundary_fault(0);if(z!=SC_OK||valid!=1)goto bad;
   tx.encoded_used=written;
   boundary_take(&held);
  }
 }
 z=image(held.tx.raw,held.tx.used,held.tx.raw_capacity);
 if(!boundary_same(&held))return boundary_fault(0);if(!z)goto bad;
 z=so_native_guard(1);if(!boundary_same(&held))return boundary_fault(0);
 if(z||!current()||sc_hash((void*)held.tx.raw,held.tx.used)!=rh)goto bad;
 zero(&next,sizeof(next));next.mode=held.tx.encoded?SO_RLE:SO_RAW;next.pointer=held.tx.encoded?held.tx.encoded:held.tx.raw;
 next.capacity=held.tx.encoded?held.tx.encoded_capacity:held.tx.raw_capacity;next.stored=held.tx.encoded?held.tx.encoded_used:held.tx.used;
 next.heap=held.tx.encoded?held.tx.encoded_heap:held.tx.raw_heap;next.used=held.tx.used;next.identity=held.tx.identity;next.generation=held.tx.generation;
 next.origin=held.tx.origin;next.hash=rh;next.codec=held.tx.encoded?1:0;next.valid=1;copy(&next.presentation,&held.tx.presentation,sizeof(next.presentation));next.seal=hash(&next,sizeof(next)-4);
 oldp=held.tx.old.pointer;oldh=held.tx.old.heap;rawp=held.tx.encoded?held.tx.raw:0;rawh=held.tx.raw_heap;r=&owner.rows[slot];
 /* Nonfallible serialized commit. Readers see BUSY through each store/free.
  * No raw table pointer is left outside an active lease. */
 r->valid=0;{U i;for(i=0;i<sizeof(next)/4;i++)if(i!=11)((U*)r)[i]=((U*)&next)[i];}
 __asm__ volatile("sync":::"memory");r->valid=1;
 zero(&tx,sizeof(tx));boundary_take(&held);
 /* Retiring storage remains globally excluded while writer is set and tx is
  * cleared. A free callback cannot reseal a different published backing. */
 if(oldp){so_native_free(oldh,oldp);if(!boundary_same(&held))return boundary_fault(oldp);}
 if(rawp){so_native_free(rawh,rawp);if(!boundary_same(&held))return boundary_fault(rawp);}
 owner.writer=owner.entered=0;return 0;
bad:owner.entered=0;return SO_BAD;
}
U so_finish(U status,U frame){U p,h,q,k;Boundary held;if(!all_ok())return SO_BAD;
 if(!owner.writer||tx.frame!=frame)return status;if(owner.entered)return SO_BUSY;
 owner.entered=1;p=tx.raw;h=tx.raw_heap;q=tx.encoded;k=tx.encoded_heap;zero(&tx,sizeof(tx));boundary_take(&held);
 if(p){so_native_free(h,p);if(!boundary_same(&held))return boundary_fault(p);}
 if(q){so_native_free(k,q);if(!boundary_same(&held))return boundary_fault(q);}
 owner.writer=owner.entered=0;return status?status:SO_BAD;
}
U so_acquire(U slot){Row before,*r;U p,h,c,z,valid=0,serial;Boundary held;
 if(!all_ok()||slot>9)return SO_BAD;
 if(owner.entered||owner.writer||owner.lease.active)return SO_BUSY;
 boundary_take(&held);z=so_native_guard(0);if(!boundary_same(&held))return boundary_fault(0);if(z)return SO_BUSY;
 if(owner.serial==0xffffffffu)return SO_SERIAL;r=&owner.rows[slot];if(!r->mode)return SO_BAD;
 owner.entered=1;copy(&before,r,sizeof(before));if(evict(&held))return SO_BAD;
 p=before.pointer;h=before.heap;c=before.capacity;boundary_take(&held);
 if(before.mode==SO_RLE){
  h=so_native_base(1);if(!boundary_same(&held))return boundary_fault(0);
  if(!span(h,4)){owner.entered=0;return h?SO_BAD:SO_NOMEM;}
  c=cap(before.used);p=so_native_allocate(h,c,32);if(!boundary_same(&held))return boundary_fault(p);
  if(!p){owner.entered=0;return SO_NOMEM;}
  z=fresh(p,c);if(!boundary_same(&held))return boundary_fault(p);if(!z)return boundary_fault(p);
  owner.cache.pointer=p;owner.cache.capacity=c;owner.cache.heap=h;owner.cache.used=before.used;owner.cache.seal=hash(&owner.cache,sizeof(Cache)-4);
  boundary_take(&held);
  z=sc_decode((void*)before.pointer,before.stored,(void*)p,c,before.used,before.hash,&valid);
  if(!boundary_same(&held))return boundary_fault(0);if(z!=SC_OK||valid!=1)goto fail;
 }
 z=image(p,before.used,c);if(!boundary_same(&held))return boundary_fault(0);if(!z)goto fail;
 z=so_native_guard(0);if(!boundary_same(&held))return boundary_fault(0);
 if(z||!same(&before,r,sizeof(before))||(_gz_svd_note(SVD_SO_ACQUIRE_FNV,before.used),sc_hash((void*)p,before.used))!=before.hash)goto fail;
 serial=issue();owner.lease.slot=slot;owner.lease.serial=serial;owner.lease.pointer=p;owner.lease.capacity=c;owner.lease.used=before.used;
 owner.lease.identity=before.identity;owner.lease.generation=before.generation;owner.lease.heap=h;
 if(before.mode==SO_RLE){owner.cache.slot=slot;owner.cache.identity=before.identity;owner.cache.generation=before.generation;owner.cache.hash=before.hash;owner.cache.seal=hash(&owner.cache,sizeof(Cache)-4);}
 owner.view[slot]=p;__asm__ volatile("sync":::"memory");owner.lease.active=1;owner.entered=0;return 0;
fail:if(evict(&held))return SO_BAD;owner.entered=0;return SO_BAD;
}
U so_validate(U pin){Lease*l=&owner.lease;Row*r;Cache*c=&owner.cache;
 if(!all_ok()||owner.entered||owner.writer||!l->active||!pin||l->serial!=pin||l->slot>9)return SO_BAD;
 r=&owner.rows[l->slot];if(!r->mode||r->identity!=l->identity||r->generation!=l->generation||r->used!=l->used||l->capacity!=cap(r->used)||owner.view[l->slot]!=l->pointer)return SO_BAD;
 if(r->mode==SO_RAW)return r->pointer==l->pointer&&r->heap==l->heap&&r->capacity==l->capacity?0:SO_BAD;
 return c->pointer==l->pointer&&c->heap==l->heap&&c->capacity==l->capacity&&c->slot==l->slot&&c->identity==l->identity&&c->generation==l->generation&&c->hash==r->hash?0:SO_BAD;
}
U so_pin(void){return owner.lease.active?owner.lease.serial:0;}
U so_source(U pin){return so_validate(pin)?0:owner.lease.pointer;}
U so_bytes(U pin){return so_validate(pin)?0:owner.lease.used;}
U so_capacity(U pin){return so_validate(pin)?0:owner.lease.capacity;}
U so_identity(U pin){return so_validate(pin)?0:owner.lease.identity;}
U so_generation(U pin){return so_validate(pin)?0:owner.lease.generation;}
U so_lease_slot(U pin){return so_validate(pin)?0xffffffffu:owner.lease.slot;}
U so_origin(U pin){return so_validate(pin)?0xffffffffu:owner.rows[owner.lease.slot].origin;}
U so_raw_hash(U pin){return so_validate(pin)?0:owner.rows[owner.lease.slot].hash;}
void so_release_leaf(void){U slot=owner.lease.slot;if(slot<10)owner.view[slot]=0;owner.lease.active=0;}
U so_raw_match(U pin,U slot,U p,U n){return !so_validate(pin)&&owner.lease.slot==slot&&owner.lease.pointer==p&&owner.lease.used==n;}
U so_intent_match(U slot,U id,U gen,U used,U h){Row*r;if(!all_ok()||owner.entered||owner.writer||slot>9)return 0;r=&owner.rows[slot];return r->mode&&r->identity==id&&r->generation==gen&&r->used==used&&r->hash==h;}
U so_info(U slot,U output){Row*r;SoInfo*i=(SoInfo*)output;
 if(!all_ok()||owner.entered||owner.writer||slot>9||!so_physical_apart(output,sizeof(*i)))return SO_BAD;
 r=&owner.rows[slot];zero(i,sizeof(*i));i->mode=r->mode;if(!r->mode)return 0;
 i->raw_used=r->used;i->identity=r->identity;i->generation=r->generation;i->origin=r->origin;i->hash=r->hash;copy(&i->presentation,&r->presentation,sizeof(i->presentation));return 0;
}

#include "source_active_write_guard.inc"
