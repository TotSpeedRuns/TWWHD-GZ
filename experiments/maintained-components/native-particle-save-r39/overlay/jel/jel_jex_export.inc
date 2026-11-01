/* Included after the actual JEL/JEX bodies and unchanged quiet-reader API.
 * Only one native-owned fixed receiver is admitted before the original lock.
 * No native callback/API/allocation or foreign metadata lock runs while held.
 * All owner rows are exported: nonzero attribution is not made anonymous.
 */
static void jxs_copy(void *d,const void *s,u32 bytes){u32 i;
 volatile u32 *out=d;const u32 *in=s;
 for(i=0;i<bytes/4;i++)out[i]=in[i];
}
static u32 jxs_equal(const void *a,const void *b,u32 bytes){u32 i;
 const u32*x=a,*y=b;for(i=0;i<bytes/4;i++)if(x[i]!=y[i])return 0;return 1;
}
static void jxs_header(u32 *h,u32 request,u32 frame,u32 thread){
 h[0]=JEX_CAPTURE_MAGIC;h[1]=JEX_CAPTURE_BYTES;h[2]=JEX_CAPTURE_VERSION;h[3]=1;
 h[4]=request;h[5]=frame;h[6]=thread;h[7]=_gz_po_frame;
 h[8]=_gz_jel_state.serial;h[9]=_gz_jel_state.events;
 h[10]=JEL_ROWS;h[11]=sizeof(JexOwner);h[12]=JEX_DEPTH;h[13]=sizeof(JexScope);
 h[14]=JEX_CAP/32;h[15]=0;
}
/* The first export validates every actual retained event under JEL's
 * original lock. Subsequent held rechecks require exact source report,
 * owners and returned bits; native window mutations always advance serial
 * or change these receipts. No expensive full-window hash is repeated for
 * every payload page, and no event payload is inferred from archived keys. */
static u32 jxs_source_ok(u32 full_window) {
 u32 i;
 if(jexw_metadata_reason(_gz_jex_report,_gz_jex_returned)||
    (full_window&&jexw_window_reason(_gz_jex_report,(const u32*)_gz_jex_events,
                                    _gz_jex_returned))||
    _gz_jex_depth||_gz_jex_report[9]||_gz_jex_cookie!=_gz_jex_report[8]||
    _gz_jex_cookie!=_gz_jex_report[11])return 0;
 for(i=0;i<JEX_DEPTH;i++)if(_gz_jex_scopes[i].cookie)return 0;
 for(i=0;i<JEL_ROWS;i++){
  const JexOwner*owner=_gz_jex_owners+i;const JelRow*row=_gz_jel_state.row+i;
  /* Generation0 retains all earlier scalar history verbatim. */
  if(!owner->generation)continue;
  if(row->alive!=1||owner->manager!=row->manager||owner->emitter!=row->emitter||
     owner->generation!=row->serial||!owner->scope||owner->scope>_gz_jex_cookie||
     !owner->actor||!owner->pid||!owner->actor_generation||!owner->thread||
     !owner->frame||owner->generation>_gz_jel_state.serial)return 0;
 }
 return 1;
}
u32 _gz_jel_jpc_export(u32 request,u32 frame,u32 thread,JelJexSnapshot*out,u32 bytes){u32 h[16],ok;
 if(bytes!=sizeof(*out)||_gz_jpc_export_receiver(request,frame,thread,out,bytes,1)!=1||!enter(0))return 0;
 if(!jxs_source_ok(1)){leave();return 0;}
 jxs_header(h,request,frame,thread);h[3]=0;
 jxs_copy(&out->ledger,&_gz_jel_state,sizeof(out->ledger));
 /* Only our transient lock word is represented as the quiescent value0.
  * It is1 solely because this very exporter holds the original JEL lock. */
 out->ledger.busy=0;
 jxs_copy(out->pools,_gz_jel_pools,sizeof(out->pools));
 jxs_copy(out->aux_pools,_gz_jel_aux_pools,sizeof(out->aux_pools));
 jxs_copy(&out->jex,h,sizeof(h));
 jxs_copy(out->jex.report,_gz_jex_report,sizeof(out->jex.report));
 jxs_copy(out->jex.owner,_gz_jex_owners,sizeof(out->jex.owner));
 jxs_copy(out->jex.scope,_gz_jex_scopes,sizeof(out->jex.scope));
 out->jex.depth=_gz_jex_depth;out->jex.cookie=_gz_jex_cookie;
 jxs_copy(out->jex.returned,_gz_jex_returned,sizeof(out->jex.returned));
 /* Fault bits may be poisoned by a rejected concurrent event even while the
  * metadata lock is ours. Do not publish a known-faulted source snapshot. */
 if(*(volatile u32*)&_gz_jel_state.flags||!jxs_source_ok(0)){leave();return 0;}
 __sync_synchronize();out->jex.valid=1;leave();
 ok=_gz_jpc_export_receiver(request,frame,thread,out,bytes,2)==1;
 if(!ok)out->jex.valid=0;
 return ok;
}
u32 _gz_jel_jpc_recheck(u32 request,u32 frame,u32 thread,const JelJexSnapshot*in,u32 bytes){
 u32 h[16],i,ok=1;const u32*a,*b;
 if(bytes!=sizeof(*in)||_gz_jpc_export_receiver(request,frame,thread,in,bytes,2)!=1||!enter(0))return 0;
 if(!jxs_source_ok(0)){leave();return 0;}
 jxs_header(h,request,frame,thread);a=(const u32*)&in->ledger;b=(const u32*)&_gz_jel_state;
 for(i=0;i<sizeof(in->ledger)/4;i++)if(a[i]!=(i==4?0:b[i])){ok=0;break;}
 ok=ok&&_gz_jel_state.busy==1&&
  jxs_equal(in->pools,_gz_jel_pools,sizeof(in->pools))&&
  jxs_equal(in->aux_pools,_gz_jel_aux_pools,sizeof(in->aux_pools))&&
  jxs_equal(&in->jex,h,sizeof(h))&&jxs_equal(in->jex.report,_gz_jex_report,sizeof(in->jex.report))&&
  jxs_equal(in->jex.owner,_gz_jex_owners,sizeof(in->jex.owner))&&
  jxs_equal(in->jex.scope,_gz_jex_scopes,sizeof(in->jex.scope))&&
  in->jex.depth==_gz_jex_depth&&in->jex.cookie==_gz_jex_cookie&&
  jxs_equal(in->jex.returned,_gz_jex_returned,sizeof(in->jex.returned));
 leave();return ok&&_gz_jpc_export_receiver(request,frame,thread,in,bytes,2)==1;
}
