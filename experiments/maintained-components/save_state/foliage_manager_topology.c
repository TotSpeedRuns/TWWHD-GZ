/* Pure serialized manager-topology comparison. This grants NO native lifetime,
 * object, resource, replay, source-pin or heap authority. Old pointers are
 * opaque index bases. No old address is dereferenced and no pointer is copied
 * into a native object. A future participant must independently bind every
 * current allocation and its real game-thread/world transaction.
 *
 * Numeric matrices/flags/timers may differ: they are state to rewind, not
 * constructor identity. This compares immutable placement and room membership.
 * Separate plant actors, renderer resources and environment closure are NOT
 * validated here. Slots must retain the same immutable placement ordering;
 * arbitrary placement-index permutations require a later bijection provider.
 * A matching membership set with different counted room order returns
 * FGT_ORDER_REPLAY, not ordinary READY. No native pointer array may be treated
 * as restored until a separately owned writer and inverse replay that order.
 */
typedef unsigned int u32;typedef unsigned char u8;
enum {FGT_BYTES=251568,FGT_BAD=1600,FGT_ROOT=1601,FGT_PLACEMENT=1602,
 FGT_LINK=1603,FGT_ROOM=1604,FGT_DUPLICATE=1605,FGT_UNREGISTERED=1606,FGT_ORDER_REPLAY=1607};
/* Registered-slot masks are source/current logical topology evidence only.
 * An integrating participant MUST NOT replay unregistered unit payloads. */
typedef struct {u32 magic,bytes,status,group,index,roots,placements,links,room_members,relocated_roots,plants_source,plants_live,order_rooms,order_members,order_group_mask,reserved,registered[63];} FgtReport;
typedef char FgtSize[sizeof(FgtReport)==316?1:-1];
static u32 fgt_u(const u8*p,u32 o){return *(const u32*)(p+o);}
static const u32 fgt_rows[4]={32,108128,145552,165504};
static const u32 fgt_counts[4]={1500,64,200,200};
static const u32 fgt_native_stride[4]={68,168,68,584};
static const u32 fgt_record_stride[4]={68,168,68,120};
static const u32 fgt_pool[4]={156,156,156,152};
static const u32 fgt_sizes[4]={0x66408,0x2de90,0x1c654,0x7d3d0};
static const u32 fgt_masks[4]={0,47,49,56};
static u32 fgt_pointer(u32 p,u32 bytes){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&bytes<=0xbffe0000u-p;}
static u32 fgt_index(u32 pointer,u32 root,u32 group,u32*out){u32 base=root+fgt_pool[group],stride=fgt_native_stride[group],d;
 if(!pointer){*out=0xffffffffu;return 1;}
 if(pointer<base)return 0;d=pointer-base;
 if(d%stride||d/stride>=fgt_counts[group])return 0;*out=d/stride;return 1;
}
static u32 fgt_mark(u32*bits,u32 index){u32 mask=1u<<(index&31);if(bits[index>>5]&mask)return 0;bits[index>>5]|=mask;return 1;}
static void fgt_clear(volatile u32*p,u32 n){u32 i;for(i=0;i<n;i++)p[i]=0;}
static u32 fgt_placement(const u8*s,const u8*l,u32 g,u32 index,FgtReport*r){u32 k;
 const u8*a=s+16+index*fgt_record_stride[g],*b=l+16+index*fgt_record_stride[g];r->index=index;
 for(k=g==3?0:4;k<(g==3?12u:16u);k+=4)if(fgt_u(a,k)!=fgt_u(b,k))return FGT_PLACEMENT;
 r->placements++;return 0;
}
static u32 fgt_linked(const u8*s,const u8*l,u32 sr,u32 lr,u32 g,FgtReport*r){u32 z,room,idx,next,other,steps;
 u32*bits=r->registered+fgt_masks[g];
 u32 rooms=16+fgt_counts[g]*68+(g==0?5824:4032);
 for(room=0;room<64;room++){
  r->index=room;
  if(!fgt_index(fgt_u(s,rooms+room*4),sr,g,&idx)||!fgt_index(fgt_u(l,rooms+room*4),lr,g,&other)||idx!=other)return FGT_LINK;
  r->links++;steps=0;
  while(idx!=0xffffffffu){
   r->index=idx;if(++steps>fgt_counts[g]||!fgt_mark(bits,idx))return FGT_DUPLICATE;r->room_members++;
   z=fgt_placement(s,l,g,idx,r);if(z)return z;
   if(!fgt_index(fgt_u(s,16+idx*68+64),sr,g,&next)||!fgt_index(fgt_u(l,16+idx*68+64),lr,g,&other)||next!=other)return FGT_LINK;
   r->links++;idx=next;
  }
 }
 return 0;
}
static u32 fgt_bounded_rooms(const u8*s,const u8*l,u32 sr,u32 lr,u32 g,FgtReport*r){
 u32 i,j,k,n,cap,ptr,index,saved_index,z,order_changed,sc[7],lc[7];u32*seen=r->registered+fgt_masks[g];
 u32 room_record=g==1?20272:34096,room_native=g==1?20412:127032,stride=g==1?268:812,capacity=g==1?64:200;
 for(i=0;i<64;i++){
  const u8*a=s+room_record+i*stride,*b=l+room_record+i*stride;r->index=i;
  n=fgt_u(a,0);cap=fgt_u(a,4);
  if(n!=fgt_u(b,0)||cap!=fgt_u(b,4)||n>capacity||cap!=capacity||
   fgt_u(a,8)!=sr+room_native+i*stride+12||fgt_u(b,8)!=lr+room_native+i*stride+12)return FGT_ROOM;
  for(k=0;k<7;k++)sc[k]=lc[k]=0;order_changed=0;
  for(j=0;j<n;j++){
   ptr=fgt_u(a,12+j*4);if(!ptr||!fgt_index(ptr,sr,g,&index)||!fgt_mark(sc,index)||!fgt_mark(seen,index))return FGT_DUPLICATE;
   saved_index=index;
   z=fgt_placement(s,l,g,index,r);if(z)return z;
   ptr=fgt_u(b,12+j*4);if(!ptr||!fgt_index(ptr,lr,g,&index)||!fgt_mark(lc,index))return FGT_DUPLICATE;
   if(index!=saved_index)order_changed=1;
   r->room_members++;
  }
  for(k=0;k<7;k++)if(sc[k]!=lc[k])return FGT_ROOM;
  if(order_changed){r->order_rooms++;r->order_members+=n;r->order_group_mask|=1u<<g;}
 }
 return 0;
}
u32 _gz_fgt_manager_topology(const u8*source,const u8*live,u32 bytes,FgtReport*r){u32 i,g,z=0,sr,lr,off;
 fgt_clear((volatile u32*)r,sizeof(*r)/4);r->magic=0x46475431u;r->bytes=sizeof(*r);r->status=FGT_BAD;
 if(bytes!=FGT_BYTES||fgt_u(source,0)!=1||fgt_u(live,0)!=1||fgt_u(source,12)!=1||fgt_u(live,12)!=1||
  fgt_u(source,8)||fgt_u(live,8)||fgt_u(source,4)>64||fgt_u(live,4)>64)return FGT_BAD;
 r->plants_source=fgt_u(source,4);r->plants_live=fgt_u(live,4);
 for(g=0;g<4;g++){
  const u8*s=source+fgt_rows[g],*l=live+fgt_rows[g];r->group=g;r->index=0;sr=fgt_u(s,0);lr=fgt_u(l,0);
  if(!sr&&!lr)continue;
  if(!sr||!lr||!fgt_pointer(sr,fgt_sizes[g])||!fgt_pointer(lr,fgt_sizes[g])||
   !fgt_u(s,4)||fgt_u(s,4)!=fgt_u(l,4)||fgt_u(s,8)!=fgt_u(l,8)||fgt_u(s,12)||fgt_u(l,12)||
   (g<3?fgt_u(s,8)>=fgt_counts[g]:fgt_u(s,8)!=1)){z=FGT_ROOT;break;}
  r->roots++;if(sr!=lr)r->relocated_roots++;
  z=(g==0||g==2)?fgt_linked(s,l,sr,lr,g,r):fgt_bounded_rooms(s,l,sr,lr,g,r);if(z)break;
  /* Source/live registered masks already agree. Unregistered storage is
   * opaque, not permission to compare/replay old uninitialized position or
   * matrix bytes. Unexpected nonzero state remains an explicit refusal. */
  for(i=0;i<fgt_counts[g];i++)if(!(r->registered[fgt_masks[g]+(i>>5)]&(1u<<(i&31)))){
   r->index=i;off=16+i*fgt_record_stride[g];
   if(g==3?(fgt_u(s,off+16)||fgt_u(l,off+16)):(s[off]||l[off])){z=FGT_UNREGISTERED;break;}
  }
  if(z)break;
 }
 if(!z&&r->order_rooms)z=FGT_ORDER_REPLAY;r->status=z;return z;
}
