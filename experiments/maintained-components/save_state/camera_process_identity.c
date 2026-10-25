/* CPI1 occupies unused padding in the existing2048-byte camera bank.
 * Pure numeric packet tests grant no native allocation or replay authority.
 * A source packet never supplies a current pointer or heap destination. */
enum {CPI_OFFSET=1760,CPI_BYTES=96,CPI_MAGIC=0x43504931u};
typedef struct {u32 magic,bytes,version,reserved;PoEntry process;u32 session,serial,frame,checksum;} CpiPacket;
typedef char CpiSize[sizeof(CpiPacket)==CPI_BYTES?1:-1];
static u32 cpi_hash(const CpiPacket*p){u32 i,h=2166136261u;const u8*b=(const u8*)p;
 for(i=0;i<CPI_BYTES;i++)h=(h^(i>=92?0:b[i]))*16777619u;return h;}
u32 _gz_cpi_validate(const CpiPacket*p,u32 camera,u32 pid,u32 frame){const PoEntry*e=&p->process;
 if(p->magic!=CPI_MAGIC||p->bytes!=CPI_BYTES||p->version!=1||p->reserved||!p->frame||p->frame!=frame||
  p->checksum!=cpi_hash(p)||!p->serial||e->generation>p->serial)return 45;
 if((camera&3)||camera<0x10000000u||camera>=0xbffe0000u||e->allocation_bytes>0xbffe0000u-camera||
  e->object!=camera||e->pid!=pid||e->state!=PO_LIVE||!e->generation||e->flags||e->end_frame||
  e->pid>=0xfffffffeu||e->process_name!=476||e->profile!=476||!e->profile_entry||
  e->allocation_bytes<0xb28u||e->allocation_bytes>0x100000u||!e->base_bytes||
  e->base_bytes+e->other_bytes<e->base_bytes||e->base_bytes+e->other_bytes!=e->allocation_bytes||
  frame-e->born_frame>=0x80000000u)return 45;
 return 0;
}
u32 _gz_cpi_relation(const CpiPacket*s,const PoEntry*l,u32 lf,u32 se,u32 le){const PoEntry*e=&s->process;u32 i;
 if(_gz_cpi_validate(s,e->object,e->pid,s->frame)||!lf||!se||se>0x00ffffffu||le<se||le>0x00ffffffu||
  lf-s->frame>=0x80000000u||l->state!=PO_LIVE||l->flags||l->end_frame||!l->generation||
  l->process_name!=476||l->profile!=476||l->profile_entry!=e->profile_entry||
  (l->object&3)||l->object<0x10000000u||l->object>=0xbffe0000u||l->allocation_bytes>0xbffe0000u-l->object||
  l->pid>=0xfffffffeu||l->base_bytes!=e->base_bytes||l->other_bytes!=e->other_bytes||
  l->allocation_bytes!=e->allocation_bytes||lf-l->born_frame>=0x80000000u)return 0;
 if(e->object==l->object&&e->pid==l->pid&&e->generation==l->generation){
  for(i=0;i<sizeof(*l)/4;i++)if(((const u32*)e)[i]!=((const u32*)l)[i])return 0;return 1;}
 return le>se&&l->generation>e->generation&&l->pid>e->pid&&l->born_frame!=s->frame&&
  l->born_frame-s->frame<0x80000000u?2:0;
}
