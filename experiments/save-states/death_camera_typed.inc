/* Included only after the sealed DCK plan/current camera binder. The source
 * actor address is an audit key: current queue membership plus a fresh Po
 * generation produces every native pointer. No source pointer is followed.
 * Ordinary camera fields/work use the previously audited independent maps.
 * The whole raw before-image is comparison/undo evidence, never saved replay.
 */
extern u32 _gz_roh_direct_camera_common(u32,u32,u32);
u8 _gz_dct_native_witness[2868];
u32 _gz_dct_witness_seal;
static u32 dct_queue(u32 value,u32 pointer,PoEntry*out){u32 q=_gz_pba_native_actor_queue,left=4096,a,found=0;
 eks_zero(out,sizeof(*out));if((pointer&&!value)||(!pointer&&value==BAD))return 0;
 while(q){
  if(!left--||!span(q,16))return DCK_BINDING;a=R32(q,12);
  if(a){
   if(!span(a,0x300))return DCK_BINDING;
   if(pointer?(a==value):(R32(a,4)==value)){if(found)return DCK_BINDING;found=a;}
  }
  q=R32(q,8);
 }
 if(!found||_gz_po_lookup(found)||!_gz_po_observation.valid||_gz_po_observation.flags)return DCK_BINDING;
 btr_copy(out,&_gz_po_observation.entry,sizeof(*out));
 if(out->object!=found||out->state!=PO_LIVE||!out->generation||out->flags||out->end_frame||
  out->pid>=0xfffffffeu||out->profile>=484||out->allocation_bytes<0x300||out->allocation_bytes>0x400000||
  out->base_bytes+out->other_bytes<out->base_bytes||out->base_bytes+out->other_bytes!=out->allocation_bytes||
  R32(found,4)!=out->pid||R16(found,8)!=out->process_name||R8(found,12)!=2||
  R16(found,14)!=out->profile||R32(found,16)!=out->profile_entry)return DCK_BINDING;
 return 0;
}
static u32 dct_before_refs(void){const DckPlan*p=&_gz_dck_plan;PoEntry now;u32 i,z;
 for(i=0;i<6;i++){
  z=dct_queue(i<5?R32(p->before,0x248+cct_link_fields[i]):R32(p->before,0x248+0x130),i<5,&now);
  if(z||!btr_equal(&now,&p->before_refs[i],sizeof(now)))return DCK_BINDING;
 }
 return p->before_refs[0].object==p->link&&p->before_refs[0].pid==p->link_pid?0:DCK_BINDING;
}
static u32 dct_saved_refs(void){const DckPlan*p=&_gz_dck_plan;PoEntry now;u32 i,z;
 for(i=0;i<6;i++){
  z=dct_queue(i<5?R32(p->saved,32+i*4):R32(p->saved,412),0,&now);
  if(z||!btr_equal(&now,&p->saved_refs[i],sizeof(now)))return DCK_BINDING;
 }
 return p->saved_refs[0].object==p->link&&p->saved_refs[0].pid==p->link_pid?0:DCK_BINDING;
}
static u32 dct_capture_refs(void){DckPlan*p=&_gz_dck_plan;u32 i,j,z,off,n,b,bytes,count,pid,match,source_entry;
 PoEntry current;
 /* DCK has already proved the selected source's complete deep owner binding.
  * Every subsequent archive read remains under its actual pin/extent. */
 off=R32(p->source,192);bytes=R32(p->source,196);n=p->source_bytes;
 if(off<216||(off&3)||off>n||bytes>n-off||bytes<96)return DCK_SOURCE;
 b=p->source+off;count=R32(b,16);
 if(R32(b,0)!=0x42434231u||R32(b,8)!=1||R32(b,12)!=1||count>208||bytes!=96+count*192)return DCK_SOURCE;
 for(i=0;i<6;i++){
  z=dct_queue(i<5?R32(p->before,0x248+cct_link_fields[i]):R32(p->before,0x248+0x130),i<5,&p->before_refs[i]);if(z)return z;
  pid=i<5?R32(p->saved,32+i*4):R32(p->saved,412);
  z=dct_queue(pid,0,&current);if(z)return z;
  if(pid!=BAD){
   match=source_entry=0;
   for(j=0;j<count;j++)if(R32(b,96+j*192+16+24)==pid){
    if(match++)return DCK_SOURCE;source_entry=b+96+j*192;
   }
   if(match!=1||R32(source_entry,4)!=1||R32(source_entry,12)!=current.generation||
    !btr_equal((const void*)(source_entry+16),&current,sizeof(current)))return DCK_BINDING;
  }
  btr_copy(&p->saved_refs[i],&current,sizeof(current));
 }
 if(p->before_refs[0].object!=p->link||p->saved_refs[0].object!=p->link||
  p->before_refs[0].pid!=p->link_pid||p->saved_refs[0].pid!=p->link_pid)return DCK_BINDING;
 return 0;
}
/* Destination is fixed by the caller to either this private projection or
 * the freshly bound current camera. All copied offsets are audited maps.
 * Native allocation headers, vtables, BG state, event data and UI are excluded.
 */
static void dct_saved_copy(u32 camera,u32 info){const DckPlan*p=&_gz_dck_plan;
 const CctField*w=cct_work(p->saved_algorithm);u32 k,pos=128,o,n,body=camera+0x248;
 for(k=0;cct_fields[k].bytes;k++){
  o=cct_fields[k].offset;n=cct_fields[k].bytes;
  if(o==4)btr_copy((void*)(body+6),p->saved+pos+2,n-2);
  else btr_copy((void*)(body+o),p->saved+pos,n);
  pos+=n;
 }
 pos=1200;for(k=0;w[k].bytes;k++){
  btr_copy((void*)(body+0x37c+w[k].offset),p->saved+pos,w[k].bytes);pos+=w[k].bytes;
 }
 for(k=0;k<5;k++)*(volatile u32*)(body+cct_link_fields[k])=p->saved_refs[k].object;
 btr_copy((void*)(camera+0xcc),p->saved+1400,56);
 btr_copy((void*)(camera+0x234),p->saved+1460,6);
 btr_copy((void*)info,p->saved+1472,12);
}
static u32 dct_prepare(void){DckPlan*p=&_gz_dck_plan;u32 z,v;
 if(_gz_dck_mode<3)return 0;
 if(R32(p->saved,1536)!=0x55495331u||R32(p->saved,1540)>1||R32(p->saved,1544)>2)return DCK_SOURCE;
 v=R32(p->saved,1408);if(v<0x3f800000u||v>0x43340000u)return DCK_SOURCE;
 z=dct_capture_refs();if(z)return z;
 btr_copy(p->after,p->before,sizeof(p->after));btr_copy(p->after_info,p->info,12);
 *(u32*)(p->after+0x248+0x510)&=~0x20000000u;
 *(u32*)(p->after+0x248+0x8a8)=0x1004487cu+p->saved_style*132;
 *(u32*)(p->after+0x248+0x8ac)=p->saved_style;
 dct_saved_copy((u32)p->after,(u32)p->after_info);
 if(R32(p->after,0x248+0x510)&0x20000000u||R32(p->after,0x248+0x514)!=p->saved_style||
  R32(p->after,0x248+0x128)!=p->link||R8(p->after,0x248+4)!=R8(p->before,0x248+4)||
  R8(p->after,0x248+5)!=R8(p->before,0x248+5))return DCK_SOURCE;
 return 0;
}
static u32 dct_after_image(void){const DckPlan*p=&_gz_dck_plan;
 if(dct_before_refs()||dct_saved_refs()||!btr_equal((const void*)p->camera,p->after,sizeof(p->after))||
  !btr_equal((const void*)((u32)_gz_cct_native_info+p->index*52),p->after_info,12))return DCK_IMAGE;
 return 0;
}
static u32 dct_apply(u32 token,u32 frame){DckPlan*p=&_gz_dck_plan;u32 z;
 if(!dck_sealed()||p->groups!=3||!_gz_dck_gate||_gz_roh_direct_camera_common(token,frame,1)||
  _gz_hg_capture_seal(_gz_dck_gate)||dck_binding()||dct_before_refs()||dct_saved_refs())return DCK_CONTEXT;
 if(_gz_osa_validate(p->pin)||_gz_osa_pin_id()!=p->pin||_gz_osa_source(p->pin)!=p->source||
  _gz_osa_bytes(p->pin)!=p->source_bytes||R32(p->source,52)!=2048||
  !btr_equal((const void*)(p->source+R32(p->source,48)),p->saved,2048))return DCK_SOURCE;
 /* Exact complete before-image is sealed BEFORE the first typed write.
  * This bounded fixed-field copy has no fallible callback/native API. */
 p->phase=DCK_COPYING;p->groups=7;dck_seal();
 dct_saved_copy(p->camera,(u32)_gz_cct_native_info+p->index*52);
 z=dct_after_image();if(z)return z;
 if(_gz_hg_capture_seal(_gz_dck_gate)||_gz_roh_direct_camera_common(token,frame,1))return DCK_CONTEXT;
 btr_copy(_gz_dct_native_witness,(const void*)p->camera,0xb28);
 btr_copy(_gz_dct_native_witness+0xb28,(const void*)((u32)_gz_cct_native_info+p->index*52),12);
 _gz_dct_witness_seal=btr_hash(_gz_dct_native_witness,sizeof(_gz_dct_native_witness));
 _gz_dck_report[25]=1;_gz_dck_report[26]=_gz_dct_witness_seal;
 return 0;
}
static void dct_undo(void){const DckPlan*p=&_gz_dck_plan;const CctField*w=cct_work(p->saved_algorithm);u32 k,o,n;
 for(k=0;cct_fields[k].bytes;k++){
  o=cct_fields[k].offset;n=cct_fields[k].bytes;
  if(o==4)btr_copy((void*)(p->body+6),p->before+0x248+6,n-2);
  else btr_copy((void*)(p->body+o),p->before+0x248+o,n);
 }
 /* Restore ALL source-work write footprints, including inactive bytes outside
  * the current GameOver work36. The entire union is never copied wholesale. */
 for(k=0;w[k].bytes;k++)btr_copy((void*)(p->body+0x37c+w[k].offset),p->before+0x248+0x37c+w[k].offset,w[k].bytes);
 for(k=0;k<5;k++)*(volatile u32*)(p->body+cct_link_fields[k])=p->before_refs[k].object;
 btr_copy((void*)(p->camera+0xcc),p->before+0xcc,56);
 btr_copy((void*)(p->camera+0x234),p->before+0x234,6);
 btr_copy((void*)((u32)_gz_cct_native_info+p->index*52),p->info,12);
}
