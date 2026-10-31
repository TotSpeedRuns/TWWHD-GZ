/* Native evidence: vtables1016ef64/94/c4/f4 -> destructor common027fbde8.
 * It cleans two28-byte members at+10/+2c through027bf4ac. Member flag bit1
 * frees +8; bit0 frees *+0 then +0. No root/child target is dereferenced before
 * an observed allocation extent is resolved. Other families remain explicit. */
#include "resource_dependencies.h"
RdReport _gz_rd_report={.magic=0x52445031u,.bytes=sizeof(RdReport),.version=1,.missing=31};
typedef char RdReportSize[sizeof(RdReport)==1352?1:-1];
static u32 rd_type(u32 vt){return vt==0x1016ef64u||vt==0x1016ef94u||vt==0x1016efc4u||vt==0x1016eff4u;}
static u32 rd_equal(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n;i++)if(((const unsigned char*)a)[i]!=((const unsigned char*)b)[i])return 0;return 1;}
static u32 rd_output_private(void){u32 i;HlHeap*h;
 if(!hr_private_base((u32)&_gz_rd_report,sizeof(_gz_rd_report),0))return 0;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&
   !hr_private_base((u32)&_gz_rd_report,sizeof(_gz_rd_report),h))return 0;
 }return 1;
}
static void rd_identity(HlObservation*out,const HlStoredAllocation*a,const HlHeap*h){
 zero(out,sizeof(*out));hl_export(&out->allocation,a);copy(&out->heap,h,sizeof(*h));
 out->flags=_gz_hl_control.flags;out->session=_gz_hl_control.session;out->valid=1;
}
static u32 rd_member(HlHeap*h,u32 object){u32 head=h->heap+0x40,node=R32(head,4),previous=head,count=R32(head,8),i,found=0;
 if(count>16384||R32(head,12)!=4)return 0;
 for(i=0;i<count;i++){
  if(!span(node,8)||node==head||R32(node,0)!=previous)return 0;
  if(node==object+4)found++;previous=node;node=R32(node,4);
 }return found==1&&node==head&&R32(head,0)==previous;
}
static u32 rd_edge(u32 slot,u32 role,u32 field,u32 value){HlStoredAllocation*a;HlHeap*h;RdEdge*e;u32 z,i;
 if(!span(value,4)||!hr_apart(value,4,(u32)&_gz_rd_report,sizeof(_gz_rd_report)))return RD_FIELD;
 z=owner_index_locked(value,4,&a,&h);if(z)return z==3?RD_OBSERVER:RD_UNTRACKED;
 /* The native delete calls consume allocation starts, never guessed interiors. */
 if(a->pointer!=value||a->missing!=31)return RD_UNTRACKED;
 if(a->heap==_gz_rd_report.root.allocation.heap&&a->pointer==_gz_rd_report.root.allocation.pointer)return RD_FIELD;
 for(i=0;i<_gz_rd_report.edge_count;i++)if(a->heap==_gz_rd_report.edges[i].identity.allocation.heap&&
  a->pointer==_gz_rd_report.edges[i].identity.allocation.pointer)return RD_FIELD;
 if(_gz_rd_report.edge_count>=6)return RD_FIELD;
 e=_gz_rd_report.edges+_gz_rd_report.edge_count++;e->slot=slot;e->role=role;e->field=field;e->value=value;rd_identity(&e->identity,a,h);return 0;
}
static u32 rd_inspect_locked(u32 object,u32 owner_heap){u32 z=RD_ARGUMENT,i,base,flags,table,data,serial,releases,offset;HlStoredAllocation*a;HlHeap*h,*owner;
 unsigned char check[72];RdReport*r=&_gz_rd_report;
 if(!span(object,72)||!hr_apart(object,72,(u32)r,sizeof(*r)))return RD_ARGUMENT;
 zero(r,sizeof(*r));r->magic=0x52445031u;r->bytes=sizeof(*r);r->version=1;r->object=object;r->owner=owner_heap;r->missing=31;
 if(!hr_healthy()){z=RD_OBSERVER;goto done;}
 serial=_gz_hl_control.serial;releases=_gz_hl_control.release_serial;r->serial=serial;r->release_serial=releases;
 z=owner_index_locked(object,72,&a,&h);if(z){z=z==3?RD_OBSERVER:RD_UNTRACKED;goto done;}
 rd_identity(&r->root,a,h);copy(r->object_bytes,(const void*)object,72);r->vtable=R32(r->object_bytes,12);
 if(!rd_type(r->vtable)){z=RD_TYPE;goto done;}
 owner=heap_find(owner_heap,0);
 if(!owner||owner->state!=HL_LIVE||!_gz_hm_scope_live(owner)||R32(r->object_bytes,0)!=owner_heap||!rd_member(owner,object)){z=RD_REGISTRY;goto done;}
 for(i=0;i<2;i++){
  offset=16+i*28;base=object+offset;flags=r->object_bytes[offset+20];
  if(flags&2){data=R32(r->object_bytes,offset+8);if(data){z=rd_edge(i,RD_OWNED_ARRAY,base+8,data);if(z)goto done;}}
  if(flags&1){
   table=R32(r->object_bytes,offset);if(!table){z=RD_FIELD;goto done;}
   z=rd_edge(i,RD_OWNED_TABLE,base,table);if(z)goto done;
   data=R32(table,0);r->table_value[i]=data;
   if(data){z=rd_edge(i,RD_OWNED_TABLE_DATA,table,data);if(z)goto done;}
  }
 }
 copy(check,(const void*)object,72);if(!rd_equal(check,r->object_bytes,72)){z=RD_CHANGED;goto done;}
 for(i=0;i<r->edge_count;i++)if(r->edges[i].role==RD_OWNED_TABLE&&R32(r->edges[i].value,0)!=r->table_value[r->edges[i].slot]){z=RD_CHANGED;goto done;}
 if(_gz_hl_control.serial!=serial||_gz_hl_control.release_serial!=releases||!hr_healthy()){z=RD_CHANGED;goto done;}
 z=0;__sync_synchronize();r->valid=1;
done:r->code=z;return z;
}
u32 _gz_rd_inspect(u32 object,u32 owner_heap){u32 z;
 if(!span(object,72)||!hr_apart(object,72,(u32)&_gz_rd_report,sizeof(_gz_rd_report))||!_gz_sc_private(object,72))return RD_ARGUMENT;
 if(!lock())return RD_BUSY;
 if(!rd_output_private()){unlock();return RD_ARGUMENT;}
 z=rd_inspect_locked(object,owner_heap);unlock();return z;
}
