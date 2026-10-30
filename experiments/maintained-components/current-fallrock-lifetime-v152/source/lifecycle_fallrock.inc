/* Finite CURRENT profile422 retirement shape. Caller already proved current
 * PO/OB/solid-HL and owns either the actual WHL inverse or fresh LRW writer.
 * No saved pointer is followed, no resource/native object is written, and
 * this check alone grants neither publication nor a lifecycle capability. */
static u32 lrr_rock_bound(u32 p,u32 bytes)
{
    return span(p,bytes) && !_gz_ob_try_capture(p,bytes,&lrr_binding) &&
        !_gz_ob_try_validate(&lrr_binding);
}

/* Same resident resource tree/headers used by the retained native lookup.
 * Fresh OB precedes each dynamic envelope. Only index32 (KROCK_00) from the
 * exact CURRENT Always entry is accepted; no saved or caller-selected name.
 * The genuine writer/WHL interval excludes concurrent native resource work. */
static u32 lrr_rock_resource(u32 expected)
{
    u32 control=_gz_brg_native_resource_control_slot,pool,storage,count,node;
    u32 low=0,high=0xffffffffu,seen[4]={0,0,0,0},steps=0;
    if(!control || control!=_gz_pr_resource_control ||
       !lrr_rock_bound(control,0x2130) ||
       R32(control,0)!=(u32)_gz_lpc_native_res_control_vtable)return 0;
    pool=control+20;storage=control+40;count=R32(pool,12);node=R32(pool,0);
    if(R32(pool,8)!=storage || R32(pool,16)!=128 || !count || count>128)return 0;
    while(node) {
        u32 i,key,info,table,n,container,header,bytes,resource;
        if(node<storage || node>=storage+128*32 || ((node-storage)&31) || ++steps>count)return 0;
        i=(node-storage)>>5;
        if(seen[i>>5]&(1u<<(i&31)))return 0;
        seen[i>>5]|=1u<<(i&31);
        if(R32(node,28)!=pool || R32(node,16)!=(u32)_gz_lpc_native_res_node_vtable ||
           R8(node,8)>1)return 0;
        key=R32(node,12);
        if(key!=R32(node,20) || key<low || key>high)return 0;
        if(key==0xa108392du) {
            info=R32(node,24);
            if(!lrr_rock_bound(info,0x8c) ||
               R32(info,12)!=(u32)_gz_lpc_native_res_info_vtable ||
               R32(info,16)!=info+28 || R32(info,20)!=(u32)_gz_lpc_native_res_string_vtable ||
               R32(info,24)!=64 || R32(info,28)!=0x416c7761u ||
               R16(info,32)!=0x7973 || R8(info,34))return 0;
            table=R32(info,96);n=R32(info,132);container=R32(info,92);
            if(n<=32 || n>4096 || !lrr_rock_bound(table,n*4))return 0;
            resource=R32(table,32*4);
            if(!resource || resource!=expected || !lrr_rock_bound(resource,0x20) ||
               !lrr_rock_bound(container,0x24))return 0;
            header=R32(container,20);
            if(!lrr_rock_bound(header,0x6c) || R32(header,0)!=0x46524553u ||
               R32(header,4)!=0x03040001u)return 0;
            bytes=R32(header,12);
            if(bytes<0x6c || bytes>0x4000000u || !lrr_rock_bound(header,bytes))return 0;
            /* Recheck every followed owning edge before returning an identity.
             * This is not a resource refcount edit or a destruction receipt. */
            if(!lrr_rock_bound(info,0x8c) || R32(info,96)!=table ||
               R32(info,132)!=n || R32(info,92)!=container ||
               !lrr_rock_bound(table,n*4) || R32(table,32*4)!=resource ||
               !lrr_rock_bound(container,0x24) || R32(container,20)!=header ||
               !lrr_rock_bound(control,0x2130) ||
               control!=_gz_brg_native_resource_control_slot || control!=_gz_pr_resource_control ||
               R32(pool,0)==0 || R32(pool,12)!=count || R32(node,24)!=info ||
               R32(node,12)!=0xa108392du || R32(node,20)!=0xa108392du)return 0;
            return 1;
        }
        if(key>0xa108392du) {if(!key)return 0;high=key-1;node=R32(node,0);}
        else {if(key==0xffffffffu)return 0;low=key+1;node=R32(node,4);}
    }
    return 0;
}

static u32 lrr_fallrock_shape(const LrrActor*x)
{
    u32 a=x->process.object,d=GZ_OB_DATA_BEGIN,t=GZ_OB_TEXT_BEGIN,model,resource;
    if(x->process.profile!=422 || x->process.process_name!=422 ||
       x->process.allocation_bytes!=0x734 || x->process.profile_entry!=d+0x1b4a60 ||
       x->process.init_state || x->process.flags || x->process.state!=PO_LIVE ||
       !x->process.generation || !x->process.pid || !span(a,0x734) ||
       R32(a,4)!=x->process.pid || R16(a,8)!=422 || R8(a,10)==1 ||
       R8(a,11)!=x->pause || (x->pause&3u) || R8(a,12)!=2 || R32(a,0x14) ||
       R32(a,0x2c)!=x->layer || !x->layer || x->layer_id>=0xfffffffdu ||
       _gz_bli_native_resolve(x->layer_id)!=x->layer || !span(x->layer,0x2c) ||
       R32(x->layer,12)!=x->layer_id || R16(x->layer,0x2a)>=0x7fffu ||
       R32(a,0xa8)!=d+0x1f3bdc || R32(a,0xbc)!=d+0x1f3088 ||
       R32(a,0xf0)!=d+0x1b4a1c || R32(a,0xb4)!=d+0xe7e4 ||
       R32(a,0xf4)!=x->solid.heap || R8(a,0x2dc) || R8(a,0x3aa)!=1 ||
       R32(a,0x3b4)!=2 ||
       R32(d+0x1b4a1c,4)!=t+(0x2131854u-0x02000000u) ||
       R32(d+0xe7e4,12)!=t+(0x2131b58u-0x02000000u))return 0;
    /* Native completed phase+4 is2; Delete ignores phase+0 on this path.
     * Never transplant profile255 ItemNo/follow offsets beyond this0x734 body. */
    if(_gz_ipk_native_is_node(IPK_NODE_TYPE,R32(a,0xb8)) || R8(a+0x68,16) ||
       !lrr_empty_root(d+0x1f39a4u) || !lrr_empty_root(d+0x1f3a1cu) ||
       !lrr_empty_root(d+0x48ab40u) ||
       R32(a+0x4c,0) || R32(a+0x4c,4) || R32(a+0x4c,8) ||
       R32(a+0x4c,12)!=a || R8(a+0x4c,16) ||
       R32(a+0xc8,4)!=d+0x1f3328u || R32(a+0xc8,12)!=a || R8(a+0xc8,16)!=1 ||
       !R32(d+0x1f3328u,8) || R32(d+0x1f3328u,8)>IPR_SOURCE_CAP)return 0;
    model=R32(a,0x3ac);
    if(_gz_gh_solid(x->solid.heap,model,0xf8) ||
       !_gz_gh_identity.valid || _gz_gh_identity.heap!=x->solid.heap ||
       _gz_gh_identity.model!=model || _gz_gh_identity.model_bytes!=0xf8 ||
       !lrr_rock_bound(model,0xf8) || lrr_binding.kind!=HL_SOLID ||
       lrr_binding.heap!=x->solid.heap || lrr_binding.heap_generation!=x->solid.generation)return 0;
    resource=R32(model,0xac);
    if(!lrr_rock_resource(resource) || !lrr_rock_bound(model,0xf8) ||
       lrr_binding.heap!=x->solid.heap || lrr_binding.heap_generation!=x->solid.generation ||
       R32(model,0xac)!=resource || _gz_ob_try_validate(&x->allocation) ||
       R32(a,0x3ac)!=model || R32(a,0x3b4)!=2 || R32(a,0xf4)!=x->solid.heap)return 0;
    return 1;
}
