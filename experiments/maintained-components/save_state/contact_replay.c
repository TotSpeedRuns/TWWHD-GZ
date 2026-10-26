#include "observer_native_addresses.h"
/* Full LIVE-cohort contact relation participant. Saved object addresses are
 * keys only. Every destination is reconstructed from a current process owner
 * and the exact live collider/heap generation. The bounded WGP rebirth lane
 * independently binds original recipes and current physical actor allocations.
 * It restores reciprocal outgoing relations for the entire recorded cohort,
 * typed status/history and defined At/Tg vectors. Geometry, mass-manager and
 * callback side effects are separate participants. No ordinary commit here. */
/* The entire before image is private. Owners are deduplicated without
 * discarding generation/PID/extent identity. No source pointer is needed by
 * unpack, binding, current-value comparison or undo. */
extern u32 _gz_nrt_contact_scope(u32,u32,u32);
typedef struct {CoEntry collider;
 u32 owners,object_vt,status_vt,object_heap,status_heap,object_gen,status_gen,object_epoch,status_epoch,status_pointer,flags;
 u32 edges[3][6];u8 status_image[60],hit_vectors[2][36];
 u32 target[3][5];u8 status[40],vectors[2][36];u32 hash;} CgrRow;
typedef struct {u32 magic,bytes,valid,token,frame,phase,count,source,used,pin,packet,packet_bytes,stride,attempted,applied,undone;
 u32 before_hash,after_hash,rows_hash,seal,owner_count,reserved[11];CoHeader co;PoHeader po;} CgrPlan;
typedef char CgrRowSize[sizeof(CgrRow)==488?1:-1];
typedef char CgrPlanSize[sizeof(CgrPlan)==256?1:-1];
typedef char CgrRowsFit[sizeof(CgrPlan)+CGR_OWNERS*sizeof(PoEntry)+CGR_CAP*sizeof(CgrRow)+CGR_MANAGER_RESERVE<=CGR_SPACE?1:-1];
u32 _gz_cgr_report[32]={0x43475252u,128,1};
static const u16 cgr_base[3] __attribute__((section(".text.cgr_base"),aligned(4)))={0,0x18,0x2c};
static const u16 cgr_game[3] __attribute__((section(".text.cgr_game"),aligned(4)))={0x50,0x94,0xdc};
static const u16 cgr_status[][2] __attribute__((section(".text.cgr_status"),aligned(4)))={{0,12},{0x14,3},{0x20,3},{0x24,2},{0x28,16},{0x38,4}};
static CgrPlan*cgr_plan(void){return (CgrPlan*)_gz_cgw_space();}
static PoEntry*cgr_owners(CgrPlan*p){return (PoEntry*)((u32)p+sizeof(*p));}
static CgrRow*cgr_row(CgrPlan*p,u32 i){return (CgrRow*)((u32)p+sizeof(*p)+CGR_OWNERS*sizeof(PoEntry)+i*sizeof(CgrRow));}
static u32 cgr_factory(const CgrPlan*p){return p->reserved[0]==0x42434631u;}
static u32 cgr_missing(const CgrPlan*p,u32 i){return cgr_factory(p)&&!p->reserved[2]&&p->reserved[1]==i+1;}
static u32 bcf_key(const CgrPlan*,u32,u32);
static u8 bcf_pf_byte(const CgrPlan*,u32,u32,u8);
static u32 bcf_project(const CgrPlan*,u32,CgpRow*);
static u32 bcf_pid(CgrPlan*,u32,u32*,u32*);
static u32 cgr_rebound(const CgrPlan*),cgr_map_valid(CgrPlan*),cgr_owner_hash(CgrPlan*);
static const PoEntry*cgr_saved_pid(CgrPlan*,u32);
static u32 cgr_epoch(const CgrPlan*p,u32 f){return p->frame==f||_gz_nrt_prepared_scope(p->token,p->frame,f)||
 (cgr_factory(p)&&_gz_bcf_prepared_scope(p->token,p->frame,f));}
static u32 cgr_mask(const CgrRow*r){return (r->flags>>8)&3;}
static u32 cgr_owner_index(CgrPlan*p,const PoEntry*b){u32 i;PoEntry*o=cgr_owners(p);for(i=0;i<p->owner_count;i++)if(btr_equal(o+i,b,sizeof(*b)))return i;
 if(cgr_rebound(p)||p->owner_count==CGR_OWNERS)return BAD;btr_copy(o+p->owner_count,b,sizeof(*b));return p->owner_count++;}
static u32 cgr_pack(CgrPlan*p,CgrRow*r,const CgpRow*b){u32 a=cgr_owner_index(p,&b->owner),st=0xffffu;if(a==BAD)return CGR_BIND;
 if(b->info[5]){st=cgr_owner_index(p,&b->status_owner);if(st==BAD)return CGR_BIND;}r->owners=a|(st<<16);
 btr_copy(&r->collider,&b->collider,sizeof(CoEntry));r->object_vt=b->info[6];r->status_vt=b->info[9];r->object_heap=b->info[3];r->status_heap=b->info[14];
 r->object_gen=b->owner_heap_generation;r->status_gen=b->status_heap_generation;r->object_epoch=b->info[4];r->status_epoch=b->info[15];r->status_pointer=b->info[5];
 r->flags=(b->info[1]==2?1u<<16:0)|(b->info[10]==GZ_OB_STATUS_GET_GAME?1u<<17:0)|(b->info[2]<<20)|(b->info[13]<<22);
 btr_copy(r->edges,b->edges,sizeof(r->edges));btr_copy(r->status_image,b->status_image,60);btr_copy(r->hit_vectors,b->hit_vectors,72);return 0;}
static void cgr_unpack(CgrPlan*p,const CgrRow*r,CgpRow*b){bcl_zero(b,sizeof(*b));btr_copy(&b->collider,&r->collider,sizeof(CoEntry));
 btr_copy(&b->owner,cgr_owners(p)+(r->owners&0xffffu),sizeof(PoEntry));if(r->status_pointer)btr_copy(&b->status_owner,cgr_owners(p)+(r->owners>>16),sizeof(PoEntry));
 b->info[1]=r->flags&(1u<<16)?2:1;b->info[2]=(r->flags>>20)&3;b->info[3]=r->object_heap;b->info[4]=r->object_epoch;b->info[5]=r->status_pointer;b->info[6]=r->object_vt;
 b->info[7]=b->info[1]==2?GZ_OB_OBJECT_GET_GAME:GZ_OB_OBJECT_GET_BASE;b->info[9]=r->status_vt;b->info[10]=r->status_pointer?(r->flags&(1u<<17)?GZ_OB_STATUS_GET_GAME:GZ_OB_STATUS_GET_BASE):0;
 b->info[11]=R32((u32)r->status_image,0xc);b->info[12]=R32((u32)r->status_image,0x10);b->info[13]=(r->flags>>22)&3;b->info[14]=r->status_heap;b->info[15]=r->status_epoch;
 b->owner_heap_generation=r->object_gen;b->status_heap_generation=r->status_gen;btr_copy(b->edges,r->edges,sizeof(b->edges));btr_copy(b->status_image,r->status_image,60);btr_copy(b->hit_vectors,r->hit_vectors,72);
 b->vectors_mask=b->info[1]==2?((b->edges[0][0]&1)|((b->edges[1][0]&1)<<1)):0;}

static const CgpRow*cgr_source_row(const CgrPlan*p,u32 i){return (const CgpRow*)(p->packet+128+i*p->stride);}
static u32 cgr_hash(const void*p,u32 n,u32 skip){u32 i,h=2166136261u;for(i=0;i<n;i++)h=(h^(i>=skip&&i-skip<4?0:((const u8*)p)[i]))*16777619u;return h;}
static void cgr_seal(CgrPlan*p){p->seal=cgr_hash(p,sizeof(*p),76);}
static u32 cgr_rows(CgrPlan*p){u32 i,h;if(p->owner_count>CGR_OWNERS||(cgr_rebound(p)&&!cgr_map_valid(p)))return 0;h=cgr_owner_hash(p);for(i=0;i<p->count;i++){CgrRow*r=cgr_row(p,i);if(cgr_missing(p,i)){if(r->hash!=cgr_hash(r,sizeof(*r),484))return 0;h=(h^r->hash)*16777619u;continue;}if((r->owners&0xffffu)>=p->owner_count||(r->status_pointer?(r->owners>>16)>=p->owner_count:(r->owners>>16)!=0xffffu)||r->hash!=cgr_hash(r,sizeof(*r),484))return 0;h=(h^r->hash)*16777619u;}return h==p->rows_hash;}
u32 _gz_cgr_empty(void){return !_gz_cgw_active();}
u32 _gz_cgr_sealed(u32 t){CgrPlan*p=cgr_plan();u32 i;if(!p||!t||p->magic!=0x43475250u||p->bytes!=sizeof(*p)||p->valid!=1||p->token!=t||p->phase<1||p->phase>4||p->count>CGR_CAP||p->count>_gz_cgw_rows()||p->owner_count>CGR_OWNERS||p->attempted>p->count||p->applied>p->attempted||p->undone>p->attempted||p->seal!=cgr_hash(p,sizeof(*p),76))return 0;if(cgr_factory(p)){if(!p->reserved[1]||p->reserved[1]>p->count||p->reserved[2]>1||p->reserved[3]!=p->count-1)return 0;
 if(p->reserved[2]){if(!p->reserved[4]||!p->reserved[5]||p->reserved[4]==p->reserved[5]||
  p->reserved[8]!=p->reserved[6]+0x920||p->reserved[9]!=p->reserved[7]+0x920||p->reserved[10])return 0;
 }else for(i=4;i<11;i++)if(p->reserved[i])return 0;
 }else if(cgr_rebound(p)){if(!cgr_map_valid(p))return 0;}else for(i=0;i<11;i++)if(p->reserved[i])return 0;return 1;}
static u32 cgr_po_current(const PoEntry*old){if(_gz_po_lookup(old->object)||!_gz_po_observation.valid||_gz_po_observation.flags||!btr_equal(old,&_gz_po_observation.entry,sizeof(*old)))return CGR_BIND;
 return old->state==PO_LIVE&&!old->flags&&!old->end_frame&&old->generation&&R32(old->object,4)==old->pid&&R16(old->object,8)==old->process_name&&R8(old->object,12)==2?0:CGR_BIND;}
static u32 cgr_extent(const PoEntry*po,u32 audit,u32 n,u32 kind,u32 heap,u32 gen,u32 epoch,u32*out){HlHeap stamp;u32 off,z;
 if(kind==1){if(audit<po->object||(off=audit-po->object)>po->allocation_bytes||n>po->allocation_bytes-off||heap||gen||epoch)return CGR_BIND;*out=po->object+off;return 0;}
 if(kind!=2||!heap||!gen||R32(po->object,0xf4)!=heap)return CGR_BIND;
 z=krc_heap(0,heap,audit,n);if(z||krc_stamp(heap,&stamp)||stamp.generation!=gen||stamp.epoch!=epoch)return CGR_BIND;*out=audit;return 0;}
#include "contact_owner_mapping.c"
static u32 cgr_owner(const PoEntry*saved,PoEntry*out){CgrPlan*p=cgr_plan();u32 z;if(p&&cgr_rebound(p))return cgr_map_lookup(p,saved,out);
 z=pcp_actor(saved->pid,out);return z||!btr_equal(saved,out,sizeof(*out))?CGR_BIND:0;}
static u32 cgr_bind(const CgpRow*s,CgpRow*d){CgrPlan*p=cgr_plan();u32 z,a,st=0,vt,get,n,i,j;PoEntry po;
 bcl_zero(d,sizeof(*d));if(s->info[0]||s->collider.state!=CO_LIVE||!s->collider.generation||s->collider.dead_frame)return CGR_SOURCE;
 z=cgr_owner(&s->owner,&d->owner);if(z)return z;
 n=s->info[1]==2?0xf8:s->info[1]==1?0x50:0;if(!n)return CGR_SOURCE;
 z=cgr_map_extent(p,&s->owner,&d->owner,s->collider.object,n,s->info[2],s->info[3],s->owner_heap_generation,s->info[4],&a);if(z)return z;
 if(_gz_co_lookup(a)!=1||!_gz_co_observation.valid||_gz_co_observation.flags)return CGR_BIND;
 btr_copy(&d->collider,&_gz_co_observation.entry,sizeof(CoEntry));
 if(d->collider.state!=CO_LIVE||!d->collider.generation||d->collider.flags&(8u|16u)||d->collider.dead_frame||cgr_map_collider(p,s,d))return CGR_BIND;
 vt=R32(a,0x3c);if(vt!=s->info[6])return CGR_BIND;get=R32(vt,0x1c);if(get!=s->info[7]||R32(vt,0x2c)!=s->info[8]||
  (s->info[1]==2?(get!=GZ_OB_OBJECT_GET_GAME||R32(get,0)!=0x4e800020u):(get!=GZ_OB_OBJECT_GET_BASE||R32(get,0)!=0x38600000u||R32(get,4)!=0x4e800020u)))return CGR_BIND;
 btr_copy(d->info,s->info,sizeof(d->info));d->owner_heap_generation=s->owner_heap_generation;
 if(s->info[5]){z=cgr_owner(&s->status_owner,&d->status_owner);if(z)return z;n=s->info[10]==GZ_OB_STATUS_GET_GAME?0x3c:s->info[10]==GZ_OB_STATUS_GET_BASE?0x1c:0;if(!n)return CGR_SOURCE;
  z=cgr_map_extent(p,&s->status_owner,&d->status_owner,s->info[5],n,s->info[13],s->info[14],s->status_heap_generation,s->info[15],&st);if(z)return z;
  if(R32(st,0x18)!=s->info[9]||R32(s->info[9],0x1c)!=s->info[10]||R32(a,0x44)!=st)return CGR_BIND;
  if(s->info[11]){
   if(cgr_rebound(p)){const PoEntry*old=cgr_saved_pid(p,s->info[12]);if(!old||old->object!=s->info[11])return CGR_SOURCE;z=cgr_owner(old,&po);}
   else{z=pcp_actor(s->info[12],&po);if(!z&&po.object!=s->info[11])return CGR_BIND;}
   if(z||R32(st,0xc)!=po.object||R32(st,0x10)!=po.pid)return CGR_BIND;d->info[11]=po.object;d->info[12]=po.pid;}
  else if(R32(st,0xc)||R32(st,0x10)!=s->info[12])return CGR_BIND;
  d->status_heap_generation=s->status_heap_generation;btr_copy(d->status_image,(const void*)st,n);
  for(i=0;i<12;i+=4)if(!epj_finite(R32(st,i)))return CGR_SOURCE;
 }else if(R32(a,0x44))return CGR_BIND;
 d->info[5]=st;
 for(i=0;i<3;i++){d->edges[i][0]=R32(a+cgr_base[i],4);d->edges[i][1]=R32(a+cgr_base[i],8);d->edges[i][3]=BAD;
  if(s->info[1]==2){for(j=2;j<6;j++)d->edges[i][j]=R32(a+cgr_game[i],j==2?4:j==3?0xc:j==4?0x10:8);if(d->edges[i][5]!=s->edges[i][5])return CGR_BIND;}
 }
 if(s->info[1]==2)for(i=0;i<2;i++){d->vectors_mask|=(d->edges[i][0]&1)<<i;btr_copy(d->hit_vectors[i],(const void*)(a+cgr_game[i]+0x20),36);}
 return 0;}
static const PoEntry*cgr_saved_pid(CgrPlan*p,u32 pid){u32 w=p->source+R32(p->source,168),b=p->source+R32(p->source,192),i,n=R32(w,16);const PoEntry*found=0;
 if(n>VR_CAPACITY)return 0;for(i=0;i<n;i++)if(R32(w+80+i*48,4)==pid){if(found)return 0;found=(const PoEntry*)(b+96+i*192+16);}return found;}
static u32 cgr_pid(CgrPlan*p,u32 id,u32 historic,u32*out,u32*actor){PoEntry now;const PoEntry*old;
 *out=id;if(actor)*actor=0;if(id==BAD)return 0;
 if(cgr_rebound(p)&&historic){if(!id)return 0;old=cgr_saved_pid(p,id);return cgr_map_history(p,id,old);}
 if(cgr_factory(p)&&p->reserved[2]&&!bcf_pid(p,id,out,actor))return 0;old=cgr_saved_pid(p,id);
 if(!old){if(!id||historic){if(pcp_actor(id,&now)==PCP_ABSENT)return 0;}return CGR_SOURCE;}
 if(cgr_rebound(p)){
  if(cgr_map_lookup(p,old,&now))return CGR_BIND;
 }else if(pcp_actor(id,&now)||!btr_equal(old,&now,sizeof(now)))return CGR_BIND;
 *out=now.pid;if(actor)*actor=now.object;return 0;}
static u32 cgr_normalize(CgrPlan*p,u32 index){CgpRow view;const CgpRow*s=cgr_source_row(p,index);CgrRow*r=cgr_row(p,index);u32 i,j,k,found,obj,z,pid,actor,off=0,n;
 if(cgr_factory(p)){z=bcf_project(p,index,&view);if(z)return z;s=&view;}
 for(i=0;i<3;i++){obj=s->edges[i][1];pid=s->edges[i][3];r->target[i][0]=s->edges[i][0];r->target[i][2]=s->edges[i][2];r->target[i][3]=pid;
  if(obj){found=BAD;for(j=0;j<p->count;j++)if((cgr_factory(p)&&p->reserved[2]&&p->reserved[1]==j+1?cgr_row(p,j)->collider.object:cgr_source_row(p,j)->collider.object)==obj){if(found!=BAD)return CGR_SOURCE;found=j;}if(found==BAD)return CGR_SOURCE;r->target[i][1]=cgr_row(p,found)->collider.object;}
  if(pid==BAD){if(s->edges[i][4])return CGR_SOURCE;}
  else if(!pid&&!(s->edges[i][0]&1)&&!obj&&!s->edges[i][4]){}
  else {z=cgr_pid(p,pid,0,&r->target[i][3],&actor);if(z)return z;if(s->edges[i][4]){const PoEntry*old=cgr_saved_pid(p,pid);if((!old||old->object!=s->edges[i][4])&&!(cgr_factory(p)&&p->reserved[2]&&actor==s->edges[i][4]))return CGR_SOURCE;if(!actor)return CGR_SOURCE;r->target[i][4]=actor;}}
 }
 if(s->info[5])for(i=0;i<sizeof(cgr_status)/sizeof(cgr_status[0]);i++){
  if(i>=2&&s->info[10]!=GZ_OB_STATUS_GET_GAME)break;n=cgr_status[i][1];btr_copy(r->status+off,s->status_image+cgr_status[i][0],n);
  if(i==4)for(k=0;k<4;k++){z=cgr_pid(p,R32((u32)s->status_image,0x28+k*4),1,&pid,0);if(z)return z;*(u32*)(r->status+off+k*4)=pid;}off+=n;
 }
 r->flags|=(s->info[1]==2?((s->edges[0][0]&1)|((s->edges[1][0]&1)<<1)):0)<<8;
 if(cgr_mask(r)&&!_gz_bcm_contact_version(R32(p->packet,8)))return CGR_SOURCE;
 if(_gz_bcm_contact_version(R32(p->packet,8)))for(i=0;i<2;i++)if(cgr_mask(r)&(1u<<i))btr_copy(r->vectors[i],s->hit_vectors[i],36);
 return 0;}
/* PF and CGP are independently captured source domains. Join their named
 * collision fields before treating either as a complete pot projection. */
static u32 cgr_pot_overlay(CgrPlan*p,u32 index){const CgpRow*s=cgr_source_row(p,index);CgrRow*r=cgr_row(p,index);u32 bank,n,i,pf=0,z;
 if(s->owner.process_name!=453)return 0;bank=p->source+R32(p->source,184);n=R32(bank,16);
 if(n>64)return CGR_SOURCE;for(i=0;i<n;i++)if(R32(bank+64+i*2048,20)==s->owner.pid){if(pf)return CGR_SOURCE;pf=bank+64+i*2048;}
 if(!pf)return CGR_SOURCE;z=_gz_pfc_source_join(pf,s,R32(p->packet,20),R32(p->packet,8));if(z)return z;
 z=_gz_pfc_vectors(pf,r->vectors);if(z)return z;r->flags|=3u<<8;return 0;
}
static u32 cgr_binding_entries(CgrPlan*p,const CgrRow*r){CgpRow image;const CgpRow*b=&image;cgr_unpack(p,r,&image);u32 z,a=b->collider.object,st=b->info[5],i,q,n;
 z=cgr_map_physical(p,&b->owner);if(!z)z=cgr_po_current(&b->owner);if(z)return z;
 z=cgr_extent(&b->owner,a,b->info[1]==2?0xf8:0x50,b->info[2],b->info[3],b->owner_heap_generation,b->info[4],&q);if(z||q!=a)return CGR_BIND;
 if(_gz_co_lookup(a)!=1||!_gz_co_observation.valid||_gz_co_observation.flags||!btr_equal(&b->collider,&_gz_co_observation.entry,sizeof(CoEntry))||R32(a,0x3c)!=b->info[6]||R32(a,0x44)!=st)return CGR_BIND;
 if(st){z=cgr_map_physical(p,&b->status_owner);if(!z)z=cgr_po_current(&b->status_owner);if(z)return z;n=b->info[10]==GZ_OB_STATUS_GET_GAME?0x3c:0x1c;
  z=cgr_extent(&b->status_owner,st,n,b->info[13],b->info[14],b->status_heap_generation,b->info[15],&q);if(z||q!=st||R32(st,0x18)!=b->info[9]||R32(st,0xc)!=b->info[11]||R32(st,0x10)!=b->info[12])return CGR_BIND;}
 if(b->info[1]==2)for(i=0;i<3;i++)if(R32(a+cgr_game[i],8)!=b->edges[i][5])return CGR_BIND;
 return 0;}
static u32 cgr_binding(CgrPlan*p,const CgrRow*r){
 if(_gz_co_header.busy||_gz_po_header.busy||!btr_equal(&p->co,&_gz_co_header,sizeof(CoHeader))||!btr_equal(&p->po,&_gz_po_header,sizeof(PoHeader)))return CGR_CHANGED;
 return cgr_binding_entries(p,r);}
static u32 cgr_piece(u32 dst,const void*src,u32 n,u32 write){u32 i;
 if(write){if(!((dst|(u32)src|n)&3)){for(i=0;i<n;i+=4)*(volatile u32*)(dst+i)=R32((u32)src,i);}
  else btr_copy((void*)dst,src,n);return 0;}return btr_equal((const void*)dst,src,n)?0:CGR_CHANGED;}
static u32 cgr_store_piece(u32 dst,const void*src,u32 n,u32 write,u32 partial){
 if(write&&partial&&!btr_equal((const void*)dst,src,n)){cgr_piece(dst,src,n,1);_gz_cgr_report[20]=dst;_gz_cgr_report[21]=n;return CGR_PARTIAL;}return cgr_piece(dst,src,n,write);}
static u32 cgr_fold(u32 h,const void*p,u32 n){u32 i;for(i=0;i<n;i++)h=(h^((const volatile u8*)p)[i])*16777619u;return h;}
/* Hash the exact named native stores independently from the immutable plan.
 * mode0 reads native state; mode1/2 derive before/target expectations. */
static u32 cgr_domain_hash_except(CgrPlan*p,u32 mode,u32 omit){u32 i,j,h=2166136261u,a,g,off,n;const CgrRow*r;CgpRow image;const CgpRow*b;const void*v;const u32*q;
 for(i=0;i<p->count;i++){if(i==omit||cgr_missing(p,i))continue;r=cgr_row(p,i);cgr_unpack(p,r,&image);b=&image;a=b->collider.object;
  for(j=0;j<3;j++){g=a+cgr_game[j];q=mode==2?r->target[j]:b->edges[j];h=cgr_fold(h,mode?q:(const void*)(a+cgr_base[j]+4),8);
   if(b->info[1]==2){h=cgr_fold(h,mode?q+2:(const void*)(g+4),4);h=cgr_fold(h,mode?q+3:(const void*)(g+0xc),8);}}
  if(b->info[1]==2)for(j=0;j<2;j++){v=mode==2&&(cgr_mask(r)&(1u<<j))?r->vectors[j]:b->hit_vectors[j];h=cgr_fold(h,mode?v:(const void*)(a+cgr_game[j]+0x20),36);}
  if((r->flags&1)&&b->info[5])for(j=0,off=0;j<sizeof(cgr_status)/sizeof(cgr_status[0]);j++){
   if(j>=2&&b->info[10]!=GZ_OB_STATUS_GET_GAME)break;n=cgr_status[j][1];v=mode==2?r->status+off:b->status_image+cgr_status[j][0];h=cgr_fold(h,mode?v:(const void*)(b->info[5]+cgr_status[j][0]),n);off+=n;
  }
 }return h;}
static u32 cgr_domain_hash(CgrPlan*p,u32 mode){return cgr_domain_hash_except(p,mode,BAD);}
static u32 cgr_pair(u32 a,const void*before,const void*after,u32 n,u32 step){u32 i;for(i=0;i<n;i+=step)if(!btr_equal((const void*)(a+i),(const u8*)before+i,step)&&!btr_equal((const void*)(a+i),(const u8*)after+i,step))return CGR_CHANGED;return 0;}
static u32 cgr_mixture(CgrPlan*p,const CgrRow*r){CgpRow image;const CgpRow*b=&image;cgr_unpack(p,r,&image);u32 a=b->collider.object,i,z,g,off=0,n;
 for(i=0;i<3;i++){g=a+cgr_game[i];z=cgr_pair(a+cgr_base[i]+4,b->edges[i],r->target[i],8,4);if(z)return z;
  if(b->info[1]==2){z=cgr_pair(g+4,b->edges[i]+2,r->target[i]+2,4,4);if(z)return z;z=cgr_pair(g+0xc,b->edges[i]+3,r->target[i]+3,8,4);if(z)return z;}}
 if(b->info[1]==2)for(i=0;i<2;i++){z=cgr_pair(a+cgr_game[i]+0x20,b->hit_vectors[i],cgr_mask(r)&(1u<<i)?r->vectors[i]:b->hit_vectors[i],36,4);if(z)return z;}
 if((r->flags&1)&&b->info[5])for(i=0;i<sizeof(cgr_status)/sizeof(cgr_status[0]);i++){
  if(i>=2&&b->info[10]!=GZ_OB_STATUS_GET_GAME)break;n=cgr_status[i][1];z=cgr_pair(b->info[5]+cgr_status[i][0],b->status_image+cgr_status[i][0],r->status+off,n,n&3?1:4);if(z)return z;off+=n;
 }return 0;}
static u32 cgr_values(CgrPlan*p,const CgrRow*r,u32 target,u32 write,u32 partial){CgpRow image;const CgpRow*b=&image;cgr_unpack(p,r,&image);u32 i,z,a=b->collider.object,g,off=0,n;const u32*q;
 for(i=0;i<3;i++){q=target?r->target[i]:b->edges[i];g=a+cgr_game[i];
  z=cgr_store_piece(a+cgr_base[i]+4,q,8,write,partial);if(z)return z;
  if(b->info[1]==2){z=cgr_store_piece(g+4,q+2,4,write,partial);if(z)return z;z=cgr_store_piece(g+0xc,q+3,8,write,partial);if(z)return z;}
 }
 if(b->info[1]==2)for(i=0;i<2;i++)if(!target||(cgr_mask(r)&(1u<<i))){z=cgr_store_piece(a+cgr_game[i]+0x20,target?r->vectors[i]:b->hit_vectors[i],36,write,partial);if(z)return z;}
 if((r->flags&1)&&b->info[5])for(i=0;i<sizeof(cgr_status)/sizeof(cgr_status[0]);i++){
  if(i>=2&&b->info[10]!=GZ_OB_STATUS_GET_GAME)break;n=cgr_status[i][1];z=cgr_store_piece(b->info[5]+cgr_status[i][0],target?r->status+off:b->status_image+cgr_status[i][0],n,write,partial);if(z)return z;off+=n;
 }return 0;}
/* Before PTI, every field is the typed before image. After PTI, its two
 * embedded Mini spheres/statuses may already be the same desired projection.
 * Every other field must still be the before image. No arbitrary mixture. */
static u32 cgr_pre_forward(CgrPlan*p,const CgrRow*r){CgpRow image;const CgpRow*b=&image;cgr_unpack(p,r,&image);u32 i,n,off=0,z,a=b->collider.object,g;const u32*q;
 for(i=0;i<3;i++){q=(r->flags&2)?r->target[i]:b->edges[i];g=a+cgr_game[i];z=cgr_piece(a+cgr_base[i]+4,q,8,0);if(z)return z;
  if(b->info[1]==2){z=cgr_piece(g+4,q+2,4,0);if(z)return z;z=cgr_piece(g+0xc,q+3,8,0);if(z)return z;}
 }
 if(b->info[1]==2)for(i=0;i<2;i++){const void*v=(r->flags&2)&&(cgr_mask(r)&(1u<<i))?r->vectors[i]:b->hit_vectors[i];z=cgr_piece(a+cgr_game[i]+0x20,v,36,0);if(z)return z;}
 if((r->flags&1)&&b->info[5])for(i=0;i<sizeof(cgr_status)/sizeof(cgr_status[0]);i++){
  if(i>=2&&b->info[10]!=GZ_OB_STATUS_GET_GAME)break;n=cgr_status[i][1];z=cgr_piece(b->info[5]+cgr_status[i][0],(r->flags&4)?r->status+off:b->status_image+cgr_status[i][0],n,0);if(z)return z;off+=n;
 }return 0;}
#include "manager_contact_replay.c"
static u32 cgr_prepare_mode(u32 t,u32 f,u32 factory){CgrPlan*p;const TxObserveSource*v;u32 i=0,j,z,hash,src,n,missing=BAD;CgrRow*r;CgpRow image;const CgpRow*b=&image,*s,*q;
 if(!_gz_cgr_empty()||_gz_roh_gl_context(t,f,1)!=1||f!=_gz_pbac_frame||!(v=tx_observe_source(t,f)))return CGR_CONTEXT;
 for(i=3;i<32;i++)_gz_cgr_report[i]=0;
 src=v->source;if(R32(src,8)<29||_gz_cga_archive_validate(src,v->used,0)||_gz_pfb_archive_validate(src,v->used,0))return CGR_SOURCE;
 src+=R32(src,240);n=R32(src,4);if(R32(src,52)||R32(src,60))return CGR_SOURCE;
 if(factory){BcmPlan*m=bcm_plan();if(!m||!bsg_held(t,1)||m->h[3]!=1||m->h[4]!=t||m->h[5]!=f||!_gz_bcm_contact_version(R32(src,8)))return CGR_CONTEXT;
  for(i=0;i<R32(src,48);i++)if(R32(src+128+i*sizeof(CgpRow)+88,0)==m->h[12]){if(missing!=BAD)return CGR_SOURCE;missing=i;}
  if(missing==BAD)return CGR_SOURCE;}
 if(_gz_co_header.busy||_gz_co_header.flags||_gz_po_header.busy||_gz_po_header.flags||_gz_co_header.active+(factory?1:0)!=R32(src,48))return CGR_CHANGED;
 /* Authenticate the source tuple BEFORE the arena lease or any arena store.
  * The issued packet remains in its disjoint READ-pinned source allocation. */
 if(!tx_observe_matches(t,f,v->source,v->used,v->pin)||!(p=(CgrPlan*)_gz_cgw_begin(t,f,v->source,v->used,R32(src,48))))return CGR_CONTEXT;
 bcl_zero(p,sizeof(*p));p->magic=0x43475250u;p->bytes=sizeof(*p);p->token=t;p->frame=f;p->phase=1;p->count=R32(src,48);p->source=v->source;p->used=v->used;p->pin=v->pin;p->packet=src;p->packet_bytes=n;p->stride=R32(src,8)==1?396:sizeof(CgpRow);
 btr_copy(&p->co,&_gz_co_header,sizeof(CoHeader));btr_copy(&p->po,&_gz_po_header,sizeof(PoHeader));
 if(factory){p->reserved[0]=0x42434631u;p->reserved[1]=missing+1;p->reserved[3]=p->count-1;}
 if(!factory){z=cgr_map_prepare(p);if(z)goto fail;}
 for(i=0;i<p->count;i++){r=cgr_row(p,i);s=cgr_source_row(p,i);bcl_zero(r,sizeof(*r));if(cgr_missing(p,i)){r->hash=cgr_hash(r,sizeof(*r),484);continue;}z=cgr_bind(s,&image);if(z)goto fail;z=cgr_pack(p,r,b);if(z)goto fail;
  if(b->info[5])r->flags|=1;
  if(b->info[5]&&!epj_apart(b->info[5],b->info[10]==GZ_OB_STATUS_GET_GAME?0x3c:0x1c,b->collider.object,b->info[1]==2?0xf8:0x50)){z=CGR_ALIAS;goto fail;}
  if(b->owner.process_name==247&&(b->collider.object==b->owner.object+0x680||b->collider.object==b->owner.object+0x7ac))r->flags|=2;
  if(b->info[5]&&b->status_owner.process_name==247&&b->info[5]==b->status_owner.object+0x644)r->flags|=4;
  for(j=0;j<i;j++){CgrRow*c=cgr_row(p,j);if(cgr_missing(p,j))continue;u32 sz=c->flags&(1u<<16)?0xf8:0x50,ss=c->flags&(1u<<17)?0x3c:0x1c;q=cgr_source_row(p,j);
   if(!epj_apart(b->collider.object,b->info[1]==2?0xf8:0x50,c->collider.object,sz)){z=CGR_ALIAS;goto fail;}
   if((b->info[5]&&!epj_apart(b->info[5],b->info[10]==GZ_OB_STATUS_GET_GAME?0x3c:0x1c,c->collider.object,sz))||
    (c->status_pointer&&!epj_apart(c->status_pointer,ss,b->collider.object,b->info[1]==2?0xf8:0x50))){z=CGR_ALIAS;goto fail;}
   if(b->info[5]&&c->status_pointer&&b->info[5]!=c->status_pointer&&!epj_apart(b->info[5],b->info[10]==GZ_OB_STATUS_GET_GAME?0x3c:0x1c,c->status_pointer,ss)){z=CGR_ALIAS;goto fail;}
   if(b->info[5]&&b->info[5]==c->status_pointer){if(!btr_equal(b->status_image,c->status_image,60)||!btr_equal(s->status_image,q->status_image,60)){z=CGR_ALIAS;goto fail;}r->flags&=~1u;}
  }
 }
 hash=cgr_owner_hash(p);
 for(i=0;i<p->count;i++){r=cgr_row(p,i);if(!factory){z=cgr_normalize(p,i);if(!z)z=cgr_pot_overlay(p,i);if(z)goto fail;}r->hash=cgr_hash(r,sizeof(*r),484);hash=(hash^r->hash)*16777619u;}
 p->rows_hash=hash;p->valid=1;cgr_seal(p);if(!cgr_rows(p)){z=CGR_CHANGED;goto fail;}
 z=cgr_map_all(p);if(z)goto fail;
 for(i=0;i<p->count;i++){if(cgr_missing(p,i))continue;z=cgr_binding(p,cgr_row(p,i));if(z)goto fail;z=cgr_values(p,cgr_row(p,i),0,0,0);if(z)goto fail;}
 if(!tx_observe_matches(t,f,p->source,p->used,p->pin)){z=CGR_SOURCE;goto fail;}
 p->before_hash=cgr_domain_hash(p,1);if(cgr_domain_hash(p,0)!=p->before_hash){z=CGR_CHANGED;goto fail;}cgr_seal(p);
 z=cgm_prepare(p,factory);if(z)goto fail;
 _gz_cgr_report[3]=t;_gz_cgr_report[4]=0;_gz_cgr_report[5]=f;_gz_cgr_report[6]=p->count;_gz_cgr_report[7]=sizeof(CgrRow);_gz_cgr_report[15]=p->before_hash;_gz_cgr_report[23]=p->owner_count;return 0;
fail:_gz_cgr_report[4]=z;_gz_cgr_report[8]=i;bcl_zero(p,sizeof(*p));_gz_cgw_end(t);return z;}
u32 _gz_cgr_prepare(u32 t,u32 f){return cgr_prepare_mode(t,f,0);}
/* Refresh every process/collider/status/heap binding before a retained
 * caller follows any previously collected actor/resource span. */
u32 _gz_cgr_prepared_before(u32 t,u32 f){CgrPlan*p=cgr_plan();u32 i,z;
 if(!_gz_cgr_sealed(t)||p->phase!=1||!cgr_epoch(p,f)||!(_gz_nrt_contact_scope(t,f,2)||_gz_wgp_contact_scope(t,f,1))||!cgr_rows(p))return CGR_CONTEXT;
 z=cgr_map_all(p);if(z)return z;
 for(i=0;i<p->count;i++){z=cgr_binding(p,cgr_row(p,i));if(!z)z=cgr_values(p,cgr_row(p,i),0,0,0);if(z)return z;}
 return cgm_before(p);
}
u32 _gz_cgr_apply(u32 t,u32 f,u32 partial){CgrPlan*p=cgr_plan();u32 i=0,z;
 if(partial>2||!_gz_cgr_sealed(t)||p->phase!=1||!cgr_epoch(p,f)||!(_gz_wct_forward_scope(t,f,8)||_gz_nrt_contact_scope(t,f,2)||_gz_bcf_contact_scope(t,f,2))||!cgr_rows(p))return CGR_CONTEXT;
 z=cgr_map_all(p);if(z)goto fail;
 for(i=0;i<p->count;i++){z=cgr_binding(p,cgr_row(p,i));if(!z)z=cgr_pre_forward(p,cgr_row(p,i));if(z)goto fail;}
 z=cgm_before(p);if(z)goto fail;
 p->phase=2;cgr_seal(p);
 for(i=0;i<p->count;i++){p->attempted=i+1;cgr_seal(p);z=cgr_binding(p,cgr_row(p,i));if(!z)z=cgr_values(p,cgr_row(p,i),1,1,partial==1);if(z)goto fail;
  z=cgr_values(p,cgr_row(p,i),1,0,0);if(z)goto fail;p->applied++;cgr_seal(p);
 }
 z=cgm_apply(p,f,partial==2);if(z)goto fail;
 if(!(_gz_wct_forward_scope(t,f,8)||_gz_nrt_contact_scope(t,f,2)||_gz_bcf_contact_scope(t,f,2))){z=CGR_CONTEXT;goto fail;}p->after_hash=cgr_domain_hash(p,0);cgr_seal(p);_gz_cgr_report[16]=p->after_hash;_gz_cgr_report[17]=cgr_domain_hash(p,2);
 if(p->after_hash!=_gz_cgr_report[17]){z=CGR_CHANGED;goto fail;}p->phase=3;cgr_seal(p);_gz_cgr_report[9]=p->applied;_gz_cgr_report[22]=partial?1:0;return 0;
fail:_gz_cgr_report[10]=z;_gz_cgr_report[11]=i;return z;}
u32 _gz_cgr_undo(u32 t,u32 f){CgrPlan*p=cgr_plan();u32 i,z;if(!_gz_cgr_sealed(t)||!cgr_epoch(p,f)||!(_gz_wct_recovery_scope(t,f)||_gz_nrt_contact_scope(t,f,3)||_gz_bcf_contact_scope(t,f,3))||!cgr_rows(p))return CGR_CONTEXT;
 z=cgr_map_all(p);if(z)return z;
 if(p->phase==4)return cgm_before(p);
 z=cgm_undo(p,f);if(z){_gz_cgr_report[12]=z;return z;}
 for(i=p->attempted-p->undone;i;i--){z=cgr_binding(p,cgr_row(p,i-1));if(!z)z=cgr_mixture(p,cgr_row(p,i-1));if(!z)z=cgr_values(p,cgr_row(p,i-1),0,1,0);if(z){_gz_cgr_report[12]=z;return z;}p->undone++;cgr_seal(p);}
 p->phase=4;cgr_seal(p);_gz_cgr_report[13]=p->undone;return 0;}
u32 _gz_cgr_before(u32 t,u32 f){CgrPlan*p=cgr_plan();u32 i,z;if(!_gz_cgr_sealed(t)||!cgr_epoch(p,f)||p->phase!=4||p->undone!=p->attempted||!(_gz_wct_recovery_scope(t,f)||_gz_nrt_contact_scope(t,f,3)||_gz_bcf_contact_scope(t,f,3))||!cgr_rows(p))return CGR_CONTEXT;
 z=cgr_map_all(p);if(z)return z;
 z=cgm_before(p);if(z)return z;
 for(i=0;i<p->count;i++){z=cgr_binding(p,cgr_row(p,i));if(!z)z=cgr_values(p,cgr_row(p,i),0,0,0);if(z)return z;}_gz_cgr_report[18]=cgr_domain_hash(p,0);if(_gz_cgr_report[18]!=p->before_hash)return CGR_CHANGED;_gz_cgr_report[14]=1;return 0;}
void _gz_cgr_finish(u32 t){CgrPlan*p=cgr_plan();if(p&&p->token==t){bcl_zero(p,sizeof(*p));_gz_cgw_end(t);}}
u32 _gz_cgr_forget(u32 t){CgrPlan*p=cgr_plan();if(_gz_cgr_empty())return 0;if(!_gz_cgr_sealed(t)||p->phase!=1||p->attempted||p->applied||p->undone)return CGR_CONTEXT;_gz_cgr_finish(t);return 0;}

u32 _gz_cgr_target_check(u32 t,u32 f){CgrPlan*p=cgr_plan();u32 i,z;
 if(!(_gz_wct_target_scope(t,f)||_gz_nrt_contact_scope(t,f,2)||_gz_bcf_contact_scope(t,f,4)||_gz_wgp_contact_scope(t,f,4))||!_gz_cgr_sealed(t)||!cgr_epoch(p,f)||p->phase!=3||
  p->attempted!=p->count||p->applied!=p->count||p->undone||!cgr_rows(p))return CGR_CONTEXT;
 z=cgr_map_all(p);if(z)return z;
 for(i=0;i<p->count;i++){z=cgr_binding(p,cgr_row(p,i));if(!z)z=cgr_values(p,cgr_row(p,i),1,0,0);if(z)return z;}
 z=cgm_target(p);if(z)return z;
 return p->after_hash==cgr_domain_hash(p,0)&&p->after_hash==cgr_domain_hash(p,2)?0:CGR_CHANGED;
}

/* Contact proof consumed only by the genuine NRT coordinator. It does not
 * turn a dependency bit into a capability. Current process/collider/status
 * generations, the real physical hold and this operation's typed graph are
 * revalidated before a pot scalar path may leave these fields to CGR. */
static u32 cgr_nrt_scope(u32 t,u32 f){return _gz_nrt_contact_scope(t,f,1)||_gz_nrt_contact_scope(t,f,2)||_gz_nrt_contact_scope(t,f,3)||_gz_bcf_pot_mode(t,f)||_gz_wgp_pot_mode(t,f);}
static u32 cgr_pot_byte(const CgrRow*r,u32 off,u32 target,u8*out){u32 i,base,delta,k=0;const u32*q;const u8*b;
 for(i=0;i<3;i++){
  q=target?r->target[i]:r->edges[i];base=0x624u+pfc_base[i]+4;
  if(off>=base&&off-base<8){*out=((const u8*)q)[off-base];return 1;}
  base=0x624u+pfc_game[i]+4;
  if(off>=base&&off-base<4){*out=((const u8*)(q+2))[off-base];return 1;}
  base=0x624u+pfc_game[i]+12;
  if(off>=base&&off-base<8){*out=((const u8*)(q+3))[off-base];return 1;}
 }
 for(i=0;i<2;i++){base=0x624u+pfc_game[i]+0x20;if(off>=base&&off-base<36){
  b=target&&(cgr_mask(r)&(1u<<i))?r->vectors[i]:r->hit_vectors[i];*out=b[off-base];return 1;}}
 for(i=0;i<6;i++){base=0x754u+pfc_status[i][0];delta=off-base;
  if(off>=base&&delta<pfc_status[i][1]){*out=target?r->status[k+delta]:r->status_image[pfc_status[i][0]+delta];return 1;}k+=pfc_status[i][1];}
 return 0;
}
static u32 cgr_pot_packet_values(const CgrPlan*p,const CgrRow*r,u32 packet,u32 target){u32 i,j,k=128,w,off,n;u8 b;
 for(i=0;i<_gz_pf_field_count();i++){w=_gz_pf_field_word(i);off=w>>16;n=w&65535;
  for(j=0;j<n;j++)if(_gz_pfc_owned_byte(off+j)&&(!cgr_pot_byte(r,off+j,target,&b)||b!=(target?bcf_pf_byte(p,packet,off+j,R8(packet,k+j)):R8(packet,k+j))))return CGR_SOURCE;k+=n;}
 for(i=0;i<10;i++){
  if(i>=4&&i<7){if(R32(packet,1152+i*4)!=BAD)return CGR_SOURCE;continue;}
  for(j=0;j<4;j++)if(!cgr_pot_byte(r,pfc_refs[i]+j,target,&b)||b!=(target?bcf_pf_byte(p,packet,pfc_refs[i]+j,R8(packet,1152+i*4+j)):R8(packet,1152+i*4+j)))return CGR_SOURCE;
 }
 for(i=0;i<3;i++){const u32*q=target?r->target[i]:r->edges[i];
  if((target?bcf_key(p,3,R32(packet,1200+i*4)):R32(packet,1200+i*4))!=q[1]||(target?bcf_key(p,2,R32(packet,1200+(i+3)*4)):R32(packet,1200+(i+3)*4))!=q[4]||R32(packet,1200+(i+6)*4)!=r->edges[i][5])return CGR_SOURCE;}
 return 0;
}
/* The caller still owns genuine WGP/HG exclusion. This query is restricted to
 * the completed PREPARE receipt; it never creates or extends a correspondence.
 * The source is read only here, while the issued transaction pin is held. */
u32 _gz_cgr_pot_rebound_mode(u32 token,u32 frame)
{
 CgrPlan *plan=cgr_plan();
 return plan&&cgr_rebound(plan)&&plan->phase==1&&plan->frame==frame&&
        _gz_wgp_contact_scope(token,frame,1)&&_gz_cgr_sealed(token)&&
        cgr_rows(plan);
}

u32 _gz_cgr_pot_source_owner(u32 token,u32 frame,const PoEntry *source,
                             u32 source_index,PoEntry *current)
{
 CgrPlan *plan=cgr_plan();
 const BbEntry *birth;
 PoEntry live;
 CgrRow *row;
 u32 i,found=0,result;
 if(!_gz_cgr_pot_rebound_mode(token,frame)||!source||!current||
    source->profile!=453||source->process_name!=453)return CGR_CONTEXT;
 birth=cgr_map_source(plan,source);
 if(!birth||birth->index!=source_index)return CGR_SOURCE;
 result=cgr_map_lookup(plan,source,&live);
 if(result)return result;
 if(live.profile!=453||live.process_name!=453)return CGR_BIND;
 for(i=0;i<plan->count;i++){
  row=cgr_row(plan,i);
  if(!btr_equal(cgr_owners(plan)+(row->owners&65535),&live,sizeof(live)))continue;
  if(++found!=1||row->collider.object!=live.object+0x624||
     row->status_pointer!=live.object+0x754||!(row->flags&(1u<<16))||
     !(row->flags&(1u<<17))||cgr_mask(row)!=3||!(row->flags&1))return CGR_BIND;
  result=cgr_binding(plan,row);
  if(result)return result;
 }
 if(found!=1||!_gz_cgr_pot_rebound_mode(token,frame))return CGR_BIND;
 btr_copy(current,&live,sizeof(live));
 return 0;
}

/* Source identities are immutable scalar audits in the sealed map. Forward
 * and recovery never follow the source bank to qualify a saved PF2 packet. */
static u32 cgr_pot_packet_identity(CgrPlan *plan,const PoEntry *current,
                                  u32 packet,u32 target)
{
 u32 i;
 if(R32(packet,20)==current->pid&&R32(packet,64)==current->object)return 1;
 if(target!=1||!cgr_rebound(plan)||R32(packet,8)!=2||R32(packet,68))return 0;
 for(i=0;i<plan->owner_count;i++){
  const CgrMappedOwner *mapping=cgr_maps(plan)+i;
  if(btr_equal(cgr_owners(plan)+i,current,sizeof(*current)))
   return mapping->state==2&&mapping->source_pid==R32(packet,20)&&
          mapping->source_actor==R32(packet,64);
 }
 return 0;
}

u32 _gz_cgr_pot_packet(u32 t,u32 f,u32 actor,u32 pid,u32 packet,u32 target){CgrPlan*p=cgr_plan();u32 i,index=BAD,z;CgrRow*r;PoEntry*owner;
 if(target>1||!cgr_nrt_scope(t,f)||!_gz_cgr_sealed(t)||!cgr_epoch(p,f)||!cgr_rows(p)||_gz_pf_packet_check(packet)||
  R32(packet,16)!=453)return CGR_CONTEXT;
 for(i=0;i<p->count;i++){r=cgr_row(p,i);owner=cgr_owners(p)+(r->owners&65535);
  if(owner->pid==pid&&owner->process_name==453){if(index!=BAD)return CGR_SOURCE;index=i;}}
 if(index==BAD)return CGR_SOURCE;r=cgr_row(p,index);owner=cgr_owners(p)+(r->owners&65535);
 if(!cgr_pot_packet_identity(p,owner,packet,target))return CGR_CONTEXT;
 if(owner->object!=actor||r->collider.object!=actor+0x624||r->status_pointer!=actor+0x754||
  !(r->flags&(1u<<16))||!(r->flags&(1u<<17))||cgr_mask(r)!=3||!(r->flags&1))return CGR_SOURCE;
 z=_gz_bcf_contact_scope(t,f,5)?cgr_binding_entries(p,r):cgr_binding(p,r);if(z)return z;
 /* The pot's other three child PID fields are not contact graph fields.
  * They remain unsupported rather than being hidden by RELATED_PID. */
 for(i=4;i<7;i++)if(R32(actor,pfc_refs[i])!=BAD)return CGR_BIND;
 if(p->phase==1||p->phase==4)z=cgr_values(p,r,0,0,0);else if(p->phase==3)z=cgr_values(p,r,1,0,0);else return CGR_CONTEXT;
 return z?z:cgr_pot_packet_values(p,r,packet,target);
}
u32 _gz_cgr_span_count(u32 t,u32 f){CgrPlan*p=cgr_plan();return _gz_nrt_contact_scope(t,f,2)&&_gz_cgr_sealed(t)&&cgr_epoch(p,f)&&cgr_rows(p)?p->count*2:BAD;}
u32 _gz_cgr_span_word(u32 t,u32 f,u32 index,u32 part){CgrPlan*p=cgr_plan();CgrRow*r;
 if(part>1||!_gz_nrt_contact_scope(t,f,2)||!_gz_cgr_sealed(t)||!cgr_epoch(p,f)||index>=p->count*2||!cgr_rows(p))return BAD;
 r=cgr_row(p,index/2);if(cgr_binding(p,r))return BAD;
 if(index&1)return part?(r->status_pointer?(r->flags&(1u<<17)?0x3c:0x1c):0):r->status_pointer;
 return part?(r->flags&(1u<<16)?0xf8:0x50):r->collider.object;
}
