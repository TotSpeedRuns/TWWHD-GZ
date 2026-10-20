#include "native_shared_data.h"
/* Common current primary-model identity for all sixteen native variants.
 * Independent of the older Type1 WAIT scalar gate. Does not admit replay. */
#include "model_resource.h"
typedef unsigned char u8;
#define R32(p,o) (*(volatile const u32*)((p)+(o)))
#define R16(p,o) (*(volatile const unsigned short*)((p)+(o)))
#define R8(p,o) (*(volatile const u8*)((p)+(o)))
#include "recipes.inc"

#define CONTROL_VT ((u32)_gz_lpc_native_res_control_vtable)
#define ARCHIVE_NODE_VT ((u32)_gz_lpc_native_res_node_vtable)
#define ARCHIVE_DISPOSER_VT ((u32)_gz_lpc_native_res_info_vtable)
#define NAME_VT ((u32)_gz_lpc_native_res_string_vtable)
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static u32 name_equal(u32 p,const PmrRecipe*r){u32 j;
    for(j=0;j<sizeof(r->archive);j++){
        u8 a=R8(p,j),b=(u8)r->archive[j];
        if(a!=b)return 0;
        if(!b)return 1;
    }
    return 0;
}
static u32 resource(u32 control,const PmrRecipe*r,u32 data){
    u32 pool,storage,count,node,steps=0,seen[4]={0,0,0,0},index,key,info,table;
    if(!span(control,0x2130)||R32(control,0)!=CONTROL_VT)return PMR_CONTROL;
    pool=control+0x14;storage=control+0x28;count=R32(pool,0xc);
    if(R32(pool,8)!=storage||R32(pool,0x10)!=128||!count||count>128)return PMR_POOL;
    node=R32(pool,0);
    while(node){
        if(node<storage||node>=storage+128*32||((node-storage)&31)||++steps>count)return PMR_NODE;
        index=(node-storage)>>5;
        if(seen[index>>5]&(1u<<(index&31)))return PMR_NODE;
        seen[index>>5]|=1u<<(index&31);
        if(R32(node,0x1c)!=pool||R32(node,0x10)!=ARCHIVE_NODE_VT||R8(node,8)>1)return PMR_NODE;
        key=R32(node,0xc);if(R32(node,0x14)!=key)return PMR_NODE;
        if(key==r->crc){
            info=R32(node,0x18);
            if(!span(info,0x8c)||R32(info,0xc)!=ARCHIVE_DISPOSER_VT||
               R32(info,0x10)!=info+0x1c||R32(info,0x14)!=NAME_VT||R32(info,0x18)!=64||
               !name_equal(info+0x1c,r)||!span(R32(info,0x5c),0x24))return PMR_ARCHIVE;
            table=R32(info,0x60);
            if(r->index>4095||R32(info,0x84)<=r->index||R32(info,0x84)>=0x80000000u||
               !span(table,(r->index+1)*4)||R32(table,r->index*4)!=data)return PMR_RESOURCE;
            return PMR_OK;
        }
        node=R32(node,key>r->crc?0:4);
    }
    return PMR_MISSING;
}
static u32 actor_ok(u32 actor,u32 pid){
    return span(actor,2364)&&pid!=0xffffffffu&&R32(actor,4)==pid&&R16(actor,8)==453&&
           R8(actor,12)==2&&R8(actor,0x3aa)==1;
}
u32 _gz_pmr_model(u32 actor,u32 pid,u32 epoch){
    u32 type,heap,model,data,control,e;
    if(_gz_pt_epoch!=epoch)return PMR_CHANGED;
    if(!actor_ok(actor,pid))return PMR_ACTOR;
    type=R32(actor,0x790);if(type>=16)return PMR_TYPE;
    heap=R32(actor,0xf4);model=R32(actor,0x3b4);
    if(!span(model,0xf8)||R32(actor,0x348)!=model+0xc8)return PMR_MODEL;
    /* Registration/full allocated extent/nested heap ownership precedes any
     * dereference of the current model body. No saved heap is followed. */
    if(_gz_gh_solid(heap,model,0xf8))return PMR_HEAP;
    data=R32(model,0xac);if(!span(data,0x14))return PMR_RESOURCE;
    control=_gz_pr_resource_control;e=resource(control,&_gz_pmr_recipes[type],data);if(e)return e;
    if(_gz_pt_epoch!=epoch||!actor_ok(actor,pid)||R32(actor,0x790)!=type||
       _gz_pr_resource_control!=control||R32(actor,0xf4)!=heap||R32(actor,0x3b4)!=model||
       R32(actor,0x348)!=model+0xc8||R32(model,0xac)!=data)return PMR_CHANGED;
    return PMR_OK;
}
