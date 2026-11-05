#include "hash_index.h"
static HxU home(HxU p){return ((p>>2)*0x9e3779b1u)>>14;}
static HxU state(const HxRow*r){return r->policy>>29;}
static HxU from(HxU bit){return bit<32?~0u<<bit:0;}
static HxU first(HxU bits){return 31u-(HxU)__builtin_clz(bits&(0u-bits));}
static HxU bit_next_linear(const HxBits*b,HxU start){
 HxU word=start>>5,v=b->leaf[word]&from(start&31),mw,uw;
 if(v)return (word<<5)+first(v);
 mw=word>>5;v=b->middle[mw]&from((word&31)+1);
 if(v){word=(mw<<5)+first(v);return b->leaf[word]?(word<<5)+first(b->leaf[word]):HX_BAD_ROW;}
 uw=mw>>5;v=b->upper[uw]&from((mw&31)+1);
 if(v){mw=(uw<<5)+first(v);if(!b->middle[mw])return HX_BAD_ROW;word=(mw<<5)+first(b->middle[mw]);return b->leaf[word]?(word<<5)+first(b->leaf[word]):HX_BAD_ROW;}
 v=b->root&from(uw+1);if(!v)return HX_NONE;
 uw=first(v);if(!b->upper[uw])return HX_BAD_ROW;mw=(uw<<5)+first(b->upper[uw]);
 if(!b->middle[mw])return HX_BAD_ROW;word=(mw<<5)+first(b->middle[mw]);
 return b->leaf[word]?(word<<5)+first(b->leaf[word]):HX_BAD_ROW;
}
static HxU bit_next(const HxBits*b,HxU start){HxU v;
 if(b->root&~255u)return HX_BAD_ROW;
 if(start>=HX_ROWS||!b->root)return HX_NONE;
 v=bit_next_linear(b,start);return v==HX_NONE?bit_next_linear(b,0):v;
}
static void bit_assign(HxBits*b,HxU row,HxU set){
 HxU w=row>>5,m=w>>5,u=m>>5,mask=1u<<(row&31),v;
 v=b->leaf[w];b->leaf[w]=set?v|mask:v&~mask;
 mask=1u<<(w&31);v=b->middle[m];b->middle[m]=b->leaf[w]?v|mask:v&~mask;
 mask=1u<<(m&31);v=b->upper[u];b->upper[u]=b->middle[m]?v|mask:v&~mask;
 mask=1u<<u;v=b->root;b->root=b->upper[u]?v|mask:v&~mask;
}
static void hierarchy(HxBits*b){HxU i;
 for(i=0;i<8192;i++)if(b->leaf[i])b->middle[i>>5]|=1u<<(i&31);
 for(i=0;i<256;i++)if(b->middle[i])b->upper[i>>5]|=1u<<(i&31);
 for(i=0;i<8;i++)if(b->upper[i])b->root|=1u<<i;
}
HxU _gz_hx_rebuild(HxIndex*x,const HxRow*rows){HxU i,epoch,*words=(HxU*)x;
 if(!x->magic&&!x->version&&!x->ready&&!x->generation)epoch=1;
 else if(x->magic==HX_MAGIC&&x->version==HX_VERSION&&x->ready==1&&x->generation&&x->generation!=0xffffffffu)epoch=x->generation+1;
 else{x->ready=0;return 0;}
 /* Both arrays are already owned observer metadata. Rebuilding them under the
  * common lock also handles arbitrary held journal forward/inverse row writes.
  * There is no assumption that an old live count names all enrolled rows. */
 for(i=0;i<sizeof(*x)/4;i++)words[i]=0;
 for(i=0;i<HX_ROWS;i++){
  HxU p=rows[i].pointer,bit=1u<<(i&31);
  if(p){HxU bucket=home(p);x->next[i]=x->buckets[bucket];x->buckets[bucket]=i+1;x->occupied++;}
  else x->empty.leaf[i>>5]|=bit;
  if(state(rows+i)==3)x->dead.leaf[i>>5]|=bit;
 }
 hierarchy(&x->empty);hierarchy(&x->dead);
 x->magic=HX_MAGIC;x->bytes=sizeof(*x);x->version=HX_VERSION;x->generation=epoch;
 __sync_synchronize();x->ready=1;return 1;
}
static HxU header(const HxIndex*x){return x->magic==HX_MAGIC&&x->bytes==sizeof(*x)&&
 x->version==HX_VERSION&&x->ready==1&&x->occupied<=HX_ROWS&&
 x->generation&&!x->reserved[0]&&!x->reserved[1];}
HxU _gz_hx_next_empty(const HxIndex*x,HxU start){return header(x)?bit_next(&x->empty,start):HX_BAD_ROW;}
HxU _gz_hx_next_dead(const HxIndex*x,HxU start){return header(x)?bit_next(&x->dead,start):HX_BAD_ROW;}
/* Only the invalid pointer0 case needs the old loop. Keeping it exact avoids
 * changing behavior even for internal calls whose outer admission later fails.
 */
static HxU zero_find(const HxRow*r,HxU heap,HxU desc,HxU custom,HxU create){
 HxU i,dead=HX_NONE;for(i=0;i<HX_ROWS;i++){
  if(!r[i].pointer&&r[i].heap==heap&&(!custom||(((r[i].policy>>26)&7)==4&&r[i].block==desc)))return i+1;
  if(state(r+i)==3&&dead==HX_NONE)dead=i;
  if(!r[i].pointer)return create?(dead==HX_NONE?i:dead)+1:0;
 }return create&&dead!=HX_NONE?dead+1:0;
}
HxU _gz_hx_find(const HxIndex*x,const HxRow*r,HxU p,HxU heap,HxU desc,HxU custom,HxU create){
 HxU start,node,visited=0,empty,edge,best=HX_NONE,id,dist,dead;
 if(!header(x))return HX_BAD_ROW;
 if(!p)return zero_find(r,heap,desc,custom,create);
 start=home(p);empty=bit_next(&x->empty,start);if(empty==HX_BAD_ROW)return HX_BAD_ROW;
 edge=empty==HX_NONE?HX_ROWS:(empty-start)&HX_MASK;
 node=x->buckets[start];
 while(node){
  const HxRow*a;
  if(node>HX_ROWS||visited++==HX_ROWS)return HX_BAD_ROW;
  id=node-1;a=r+id;
  if(!a->pointer||home(a->pointer)!=start)return HX_BAD_ROW;
  dist=(id-start)&HX_MASK;
  if(a->pointer==p&&a->heap==heap&&(!custom||(((a->policy>>26)&7)==4&&a->block==desc))&&dist<edge&&
   (best==HX_NONE||dist<((best-start)&HX_MASK)))best=id;
  node=x->next[id];
 }
 if(best!=HX_NONE)return best+1;
 if(!create)return 0;
 dead=bit_next(&x->dead,start);if(dead==HX_BAD_ROW)return HX_BAD_ROW;
 if(dead!=HX_NONE&&(empty==HX_NONE||((dead-start)&HX_MASK)<=edge))return dead+1;
 return empty==HX_NONE?0:empty+1;
}
HxU _gz_hx_row_changed(HxIndex*x,const HxRow*r,HxU id,HxU old_pointer){
 HxU p,node,previous=0,seen=0,bucket;
 if(!header(x)||id>=HX_ROWS)return 0;if(x->generation==0xffffffffu){x->ready=0;return 0;}p=r[id].pointer;
 if(old_pointer!=p){
  if(old_pointer){
   bucket=home(old_pointer);node=x->buckets[bucket];
   while(node&&node!=id+1){
    if(node>HX_ROWS||seen++==HX_ROWS||!r[node-1].pointer||home(r[node-1].pointer)!=bucket)return 0;
    previous=node;node=x->next[node-1];
   }
   if(node!=id+1||!x->occupied)return 0;
   if(previous)x->next[previous-1]=x->next[id];else x->buckets[bucket]=x->next[id];
   x->occupied--;x->next[id]=0;
  }else if(x->next[id])return 0;
  if(p){bucket=home(p);x->next[id]=x->buckets[bucket];x->buckets[bucket]=id+1;x->occupied++;}
 }
 bit_assign(&x->empty,id,!p);bit_assign(&x->dead,id,state(r+id)==3);x->generation++;return 1;
}

HxU _gz_hx_cursor_begin(const HxIndex*x,HxU p,HxCursor*c){HxU empty,start;
 if(!header(x)||!p)return HX_WALK_BAD;
 start=home(p);empty=bit_next(&x->empty,start);if(empty==HX_BAD_ROW)return HX_WALK_BAD;
 c->pointer=p;c->start=start;c->edge=empty==HX_NONE?HX_ROWS:(empty-start)&HX_MASK;
 c->node=x->buckets[start];c->visited=0;c->generation=x->generation;c->reserved[0]=c->reserved[1]=0;
 return c->node>HX_ROWS?HX_WALK_BAD:HX_WALK_OK;
}
HxU _gz_hx_cursor_next(const HxIndex*x,const HxRow*r,HxCursor*c,HxU*out){HxU id,node;const HxRow*a;
 if(!header(x))return HX_WALK_BAD;
 if(c->generation!=x->generation)return HX_WALK_CHANGED;
 if(!c->pointer||c->start!=home(c->pointer)||c->edge>HX_ROWS||c->visited>HX_ROWS||c->reserved[0]||c->reserved[1])return HX_WALK_BAD;
 while(c->node){
  node=c->node;if(node>HX_ROWS||c->visited++==HX_ROWS)return HX_WALK_BAD;
  id=node-1;a=r+id;
  if(!a->pointer||home(a->pointer)!=c->start)return HX_WALK_BAD;
  c->node=x->next[id];if(c->node>HX_ROWS)return HX_WALK_BAD;
  if(a->pointer==c->pointer&&((id-c->start)&HX_MASK)<c->edge){*out=node;return HX_WALK_OK;}
 }
 return HX_WALK_DONE;
}
