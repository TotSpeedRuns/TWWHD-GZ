/* Same-allocation, coupled CPU bridge participant. Include after the BRG3
 * capture module and genuine WGP definitions in the CURRENT bomb TU.
 * No route reload, actor recreation, Execute/Draw call or GPU backing copy.
 * ROOT must qualify emitted code and the ordered ordinary next Draw before
 * this source proposal becomes an ordinary tested restore participant. */
enum {
    BRR_CONTEXT=1840, BRR_SOURCE=1841, BRR_COHORT=1842,
    BRR_CHANGED=1843, BRR_CAPACITY=1844, BRR_BYTES=1845,
    BRR_PREPARED=1, BRR_FORWARD=2, BRR_APPLIED=3, BRR_UNDO=4, BRR_ROLLED=5,
    BRR_ROWS=3, BRR_SPANS=482, BRR_SCALARS=6477, BRR_POD=60871, BRR_READS=1024
};

typedef struct {
    VrActor source;
    BrgWitness binding;
    u32 scalar_offset,scalar_bytes,geometry_offset,geometry_bytes;
    u32 span_first,span_count,source_graph,source_omitted,read_first,read_count;
} BrrRow;

typedef struct {
    u32 magic,bytes,valid,token,frame,source_frame,phase,source_hash,identity,generation;
    u32 rows,required,covered,spans,scalars,pod,reads,attempted,applied,undone,target_frame,seal;
    u32 cohort[8];
    BrrRow row[BRR_ROWS];
    BrgDescriptor span[BRR_SPANS];
    BrgReadRange read[BRR_READS];
    u8 scalar_before[BRR_SCALARS],scalar_target[BRR_SCALARS];
    u8 before[BRR_POD],target[BRR_POD];
} BrrPlan;

static BrrPlan brr_plan __attribute__((section(".bss.gz_brr")));
static u32 brr_entered,brr_mode,brr_entry_seal;
volatile u32 _gz_brr_report[24] __attribute__((section(".data.gz_brr")))={0x42525233u,96,3};
extern u32 _gz_jpg_acquire(u32,u32),_gz_jpg_check(u32,u32);
extern u32 _gz_jpg_gpu_restore(u32,u32),_gz_jpg_gpu_restore_ready(u32,u32);

u32 _gz_brr_idle(void) { return !brr_entered && !brr_plan.magic; }

/* BMH has no bridge writer yet. Reject a nonempty new bridge cohort there
 * before its world services; ordinary WCT below owns the complete bridge path.
 * This guard grants neither source ownership nor a fallback room load. */
u32 _gz_brr_factory_source_allowed(u32 source,u32 used)
{
    u32 offset,length;
    if(!brg_archive_format(source,used))return 0;
    offset=brg3_word(source,144);length=brg3_word(source,148);
    if(length>=64 && brg3_word(source+offset,0)==BRG3_MAGIC)
        return brg3_word(source+offset,16)==0;
    return 1;
}

u32 _gz_brr_private_apart(u32 p,u32 n)
{
    return n && brg_apart(p,n,(u32)&brr_plan,sizeof(brr_plan)) &&
        brg_apart(p,n,(u32)&brr_entered,4) && brg_apart(p,n,(u32)&brr_mode,4) &&
        brg_apart(p,n,(u32)&brr_entry_seal,4) &&
        brg_apart(p,n,(u32)_gz_brr_report,sizeof(_gz_brr_report));
}

u32 _gz_brr_native_context(void)
{
    return brr_entered==1 && brr_plan.token && brr_plan.frame==_gz_pbac_frame &&
        brr_mode>=1 && brr_mode<=4 &&
        _gz_wgp_contact_scope(brr_plan.token,brr_plan.frame,brr_mode) &&
        (!brr_plan.rows || (_gz_wct_heap_held(brr_plan.token,brr_plan.frame) &&
         !_gz_jpg_check(brr_plan.token,brr_plan.frame) &&
         !_gz_jpg_gpu_restore_ready(brr_plan.token,brr_plan.frame)));
}

u32 _gz_brr_source_frame(void)
{
    return _gz_brr_native_context()?brr_plan.source_frame:0;
}

static u32 brr_seal_value(void)
{
    u32 h=2166136261u,i,end=__builtin_offsetof(BrrPlan,scalar_before);
    if(brr_plan.rows>BRR_ROWS || brr_plan.spans>BRR_SPANS ||
       brr_plan.scalars>BRR_SCALARS || brr_plan.pod>BRR_POD || brr_plan.reads>BRR_READS)return 0;
    /* All fixed metadata is sealed, including unused zero rows/spans. POD
     * tails are never read or written; only the exact used shadows are hashed. */
    for(i=0;i<end;i++) {
        u32 c=i>=__builtin_offsetof(BrrPlan,seal)&&i<__builtin_offsetof(BrrPlan,seal)+4?
              0:brg_f_byte((u32)&brr_plan,i);
        h=(h^c)*16777619u;
    }
    h=brg_hash_word(h,brg3_hash((u32)brr_plan.scalar_before,brr_plan.scalars));
    h=brg_hash_word(h,brg3_hash((u32)brr_plan.scalar_target,brr_plan.scalars));
    h=brg_hash_word(h,brg3_hash((u32)brr_plan.before,brr_plan.pod));
    return brg_hash_word(h,brg3_hash((u32)brr_plan.target,brr_plan.pod));
}

static void brr_seal(void)
{
    brr_plan.seal=brr_seal_value();
    if(brr_entered && brr_plan.valid)brr_entry_seal=brr_plan.seal;
    __asm__ volatile("sync" ::: "memory");
}

u32 _gz_brr_sealed(u32 token)
{
    return token && brr_plan.magic==0x42525233u && brr_plan.bytes==sizeof(brr_plan) &&
        brr_plan.valid==1 && brr_plan.token==token && brr_plan.frame==_gz_pbac_frame &&
        brr_plan.phase>=BRR_PREPARED && brr_plan.phase<=BRR_ROLLED &&
        brr_plan.required==((1u<<brr_plan.rows)-1) && brr_plan.covered==brr_plan.required &&
        brr_plan.seal && (brr_entered && brr_entry_seal?
            brr_plan.seal==brr_entry_seal:brr_plan.seal==brr_seal_value());
}

static u32 brr_enter(u32 token,u32 frame,u32 mode)
{
    if(brr_entered || !token || token!=brr_plan.token || frame!=brr_plan.frame ||
       frame!=_gz_pbac_frame || mode<1 || mode>4)return BRR_CONTEXT;
    /* Validate the full private plan once at entry. Recursive WGP/WCT owner
     * queries during this nonyielding entry may reuse this exact seal. Every
     * native destination/read extent excludes this plan, and only the checked
     * local phase transitions below can update its metadata/seal. This avoids
     * hashing ~160KiB for each of hundreds of fresh OB queries. It grants no
     * native scope: real WGP/HG/JPG checks still run on every body access. */
    if(brr_plan.valid && !_gz_brr_sealed(token))return BRR_CHANGED;
    brr_entered=1;brr_mode=mode;brr_entry_seal=brr_plan.valid?brr_plan.seal:0;
    if(!_gz_brr_native_context()) { brr_entered=brr_mode=brr_entry_seal=0;return BRR_CONTEXT; }
    return 0;
}

static u32 brr_leave(u32 result)
{
    if(!result && !_gz_brr_native_context())result=BRR_CHANGED;
    _gz_brr_report[3]=brr_plan.token;_gz_brr_report[4]=_gz_pbac_frame;
    _gz_brr_report[5]=result;_gz_brr_report[6]=brr_plan.phase;
    _gz_brr_report[7]=brr_plan.rows;_gz_brr_report[8]=brr_plan.required;
    _gz_brr_report[9]=brr_plan.covered;_gz_brr_report[10]=brr_plan.spans;
    _gz_brr_report[11]=brr_plan.scalars;_gz_brr_report[12]=brr_plan.pod;
    _gz_brr_report[13]=brr_plan.attempted;_gz_brr_report[14]=brr_plan.applied;
    _gz_brr_report[15]=brr_plan.undone;_gz_brr_report[16]=brr_plan.target_frame;
    brr_entered=brr_mode=brr_entry_seal=0;return result;
}

/* Fresh PO cohort before any bridge body follow; the observer lock is shared
 * with PO/HL writers. Empty source is certified against current metadata too.
 * These metadata stamps do not issue the independent WGP/HG/JPG scope. */
static u32 brr_cohort(u32 capture)
{
    u32 i,j,count=0,found=0,stamp[8],z=BRR_COHORT;
    if(!_gz_brr_native_context() || !_gz_hl_meta_try_lock())return z;
    if(_gz_po_header.magic!=PO_MAGIC || _gz_po_header.bytes!=64 || _gz_po_header.version!=1 ||
       _gz_po_header.capacity!=PO_CAP || _gz_po_header.flags || _gz_po_header.busy)goto done;
    stamp[0]=_gz_po_header.serial;stamp[1]=_gz_po_header.active;
    stamp[2]=_gz_po_header.created;stamp[3]=_gz_po_header.ending;
    stamp[4]=_gz_po_header.ended;stamp[5]=_gz_po_header.rows;
    stamp[6]=_gz_po_header.session;stamp[7]=brr_plan.required;
    /* The initial scan certifies the cohort. Subsequent checks reuse it only
     * while EVERY real observer topology stamp is unchanged under this lock;
     * each row still performs its own fresh PO/OB/HL/BG/resource bind. */
    if(!capture) {
        if(brg_equal(brr_plan.cohort,stamp,sizeof(stamp)))z=0;
        goto done;
    }
    for(i=0;i<PO_CAP;i++) {
        const PoEntry *p=_gz_po_rows+i;
        if(p->profile!=89 || (p->state!=PO_LIVE && p->state!=PO_ENDING))continue;
        if(p->state!=PO_LIVE || p->flags || p->end_frame || !p->generation || p->process_name!=89)goto done;
        for(j=0;j<brr_plan.rows;j++)if(p->object==brr_plan.row[j].source.actor && p->pid==brr_plan.row[j].source.pid)break;
        if(j==brr_plan.rows || (found&(1u<<j)))goto done;
        found|=1u<<j;count++;
    }
    if(count!=brr_plan.rows || found!=brr_plan.required)goto done;
    brg_copy(brr_plan.cohort,stamp,sizeof(stamp));
    z=0;
done:
    _gz_hl_meta_unlock();return z;
}

/* Source bytes are consumed only in prepare/gpu admission. Undo never calls
 * this or follows an archive/source pointer. */
static u32 brr_source(u32 token,u32 frame,u32 *source,u32 *used,u32 *bank)
{
    const VrWorld *world;
    u32 s=wsp_source.source,n=wsp_source.used;
    if(!token || frame!=_gz_pbac_frame || !_gz_wct_group_context(token,frame,1) ||
       !wsp_source.valid || wsp_source.token!=token ||
       wsp_source.frame!=frame || _gz_osa_pin_id()!=wsp_source.pin || _gz_osa_validate(wsp_source.pin) ||
       _gz_osa_source(wsp_source.pin)!=s || _gz_osa_bytes(wsp_source.pin)!=n ||
       !brg_archive_format(s,n))return BRR_SOURCE;
    world=(const VrWorld*)(s+brg3_word(s,168));
    if(_gz_vws_validate(world,sizeof(*world)))return BRR_SOURCE;
    *source=s;*used=n;*bank=s+brg3_word(s,144);
    /* Empty legacy components remain compatible. Old bridge rows have no
     * geometry and require a new capture; they never borrow new coverage. */
    if(brg3_word(*bank,0)!=BRG3_MAGIC &&
       (brg3_word(s,148)!=4 || brg3_word(*bank,0)))return BRR_SOURCE;
    return 0;
}

static u32 brr_source_count(u32 bank)
{
    return brg3_word(bank,0)==BRG3_MAGIC?brg3_word(bank,16):0;
}

/* Called at genuine CLEAN/service1 BEFORE WHL closes allocator/jobs. No GPU
 * wait is introduced inside a closed native allocator. The gate remains
 * owned until the real ROH owner is idle; JPG's normal poll retires it. */
u32 _gz_brr_gpu_prepare(u32 token,u32 frame)
{
    u32 s,n,bank,z=brr_source(token,frame,&s,&n,&bank);
    if(z)return z;
    if(!brr_source_count(bank))return 0;
    z=_gz_jpg_acquire(token,frame);if(z)return z;
    z=_gz_jpg_gpu_restore(token,frame);if(z)return z;
    return _gz_jpg_gpu_restore_ready(token,frame);
}

u32 _gz_brr_needs_heap(u32 token,u32 frame)
{
    u32 s,n,bank;
    return !brr_source(token,frame,&s,&n,&bank) && brr_source_count(bank)!=0;
}

static u32 brr_row_current(u32 index,u32 matching,u32 prepare)
{
    BrrRow *r=brr_plan.row+index;
    BrgWitness now;
    u32 i,j,cursor=0,z=brg_actor(&r->source,r->binding.world_index,&now);
    if(z)return z;
    if(prepare)now.world_index=r->binding.world_index;
    else if(!brg_equal(&now.process,&r->binding.process,sizeof(now.process)) ||
            !brg_equal(&now.actor,&r->binding.actor,sizeof(now.actor)))return BRR_CHANGED;
    z=brg_geometry(&now,0,3);if(z)return z;
    if(now.graph_hash!=r->source_graph || now.omitted_hash!=r->source_omitted ||
       now.type!=brg3_byte((u32)brr_plan.scalar_target+r->scalar_offset,6) ||
       now.count!=brg3_byte((u32)brr_plan.scalar_target+r->scalar_offset,5) ||
       now.geometry_bytes!=r->geometry_bytes || now.span_count!=r->span_count)return BRR_CHANGED;
    if(prepare)r->binding=now;
    else if(!brg_equal(&now.heap,&r->binding.heap,sizeof(now.heap)) ||
            !brg_equal(&now.background,&r->binding.background,sizeof(now.background)))return BRR_CHANGED;
    if(prepare) {
        if(brg_read_count>BRR_READS-brr_plan.reads)return BRR_CAPACITY;
        r->read_first=brr_plan.reads;r->read_count=brg_read_count;
        brg_copy(brr_plan.read+r->read_first,brg_reads,brg_read_count*sizeof(BrgReadRange));
        brr_plan.reads+=brg_read_count;
    } else if(r->read_count!=brg_read_count ||
              !brg_equal(brr_plan.read+r->read_first,brg_reads,brg_read_count*sizeof(BrgReadRange)))return BRR_CHANGED;
    for(i=0;i<r->span_count;i++) {
        BrgDescriptor current=brg_descriptors[i],*saved=brr_plan.span+r->span_first+i;
        current.packed+=r->geometry_offset;
        if(prepare)*saved=current;
        else if(!brg_equal(&current,saved,sizeof(current)))return BRR_CHANGED;
        for(j=0;j<current.count;j++) {
            const u8 *expected=matching==1?brr_plan.target+current.packed+j*current.width:
                                          brr_plan.before+r->geometry_offset+cursor;
            u32 address=current.address+j*current.stride;
            if(prepare)brg_copy(brr_plan.before+r->geometry_offset+cursor,(const void*)address,current.width);
            else if(matching && !brg_equal((const void*)address,expected,current.width))return BRR_BYTES;
            cursor+=current.width;
        }
    }
    if(cursor!=r->geometry_bytes)return BRR_BYTES;
    for(i=0;i<r->scalar_bytes;i++) {
        u32 offset=brg3_scalar_offset(i),value;
        if(offset!=_gz_world_bridge_offsets[i] || offset>=BRG_ACTOR_BYTES)return BRR_SOURCE;
        value=brg_f_byte(now.process.object,offset);
        if(prepare)brr_plan.scalar_before[r->scalar_offset+i]=value;
        else if(matching && value!=(matching==1?brr_plan.scalar_target:brr_plan.scalar_before)[r->scalar_offset+i])return BRR_BYTES;
    }
    if(prepare) {
        if(!brg_equal(brr_plan.scalar_before+r->scalar_offset,brr_plan.scalar_target+r->scalar_offset,7) ||
           !brg3_scalar((u32)brr_plan.scalar_before+r->scalar_offset,r->scalar_bytes,now.type,now.count) ||
           !brg3_geometry((u32)brr_plan.before+r->geometry_offset,r->geometry_bytes,now.type,now.count))return BRR_BYTES;
    }
    return 0;
}

static u32 brr_current(u32 matching)
{
    u32 i,z;
    if(!_gz_brr_native_context())return BRR_CONTEXT;
    z=brr_cohort(0);if(z)return z;
    for(i=0;i<brr_plan.rows;i++) { z=brr_row_current(i,matching,0);if(z)return z; }
    return _gz_brr_native_context()?0:BRR_CHANGED;
}

u32 _gz_brr_prepare(u32 token,u32 frame)
{
    const VrWorld *world;
    u32 source,used,bank,count,i,j,z;
    if(brr_plan.magic || brr_entered)return BRR_CONTEXT;
    z=brr_source(token,frame,&source,&used,&bank);if(z)return z;
    count=brr_source_count(bank);if(count>BRR_ROWS)return BRR_CAPACITY;
    brg_zero(&brr_plan,sizeof(brr_plan));
    world=(const VrWorld*)(source+brg3_word(source,168));
    brr_plan.magic=0x42525233u;brr_plan.bytes=sizeof(brr_plan);
    brr_plan.token=token;brr_plan.frame=frame;brr_plan.source_frame=world->frame;
    brr_plan.rows=count;brr_plan.required=(1u<<count)-1;brr_plan.covered=brr_plan.required;
    brr_plan.source_hash=wsp_source.source_hash;brr_plan.identity=wsp_source.identity;brr_plan.generation=wsp_source.generation;
    for(i=0;i<count;i++) {
        BrrRow *r=brr_plan.row+i;
        u32 row=bank+64+i*64,index=brg3_word(row,0),scalars=brg3_word(row,20),pod=brg3_word(row,28);
        u32 spans=brg3_descriptor_count(brg3_word(row,8),brg3_word(row,12));
        if(index>=world->count || scalars>BRR_SCALARS-brr_plan.scalars ||
           pod>BRR_POD-brr_plan.pod || spans>BRR_SPANS-brr_plan.spans) { brg_zero(&brr_plan,sizeof(brr_plan));return BRR_CAPACITY; }
        r->source=world->actors[index];r->binding.world_index=index;
        r->scalar_offset=brr_plan.scalars;r->scalar_bytes=scalars;
        r->geometry_offset=brr_plan.pod;r->geometry_bytes=pod;
        r->span_first=brr_plan.spans;r->span_count=spans;
        r->source_graph=brg3_word(row,40);r->source_omitted=brg3_word(row,44);
        brg_copy(brr_plan.scalar_target+r->scalar_offset,(const void*)(bank+brg3_word(row,16)),scalars);
        brg_copy(brr_plan.target+r->geometry_offset,(const void*)(bank+brg3_word(row,24)),pod);
        brr_plan.scalars+=scalars;brr_plan.pod+=pod;brr_plan.spans+=spans;
    }
    z=brr_enter(token,frame,1);if(z) { brg_zero(&brr_plan,sizeof(brr_plan));return z; }
    z=brr_cohort(1);
    for(i=0;!z&&i<count;i++)z=brr_row_current(i,0,1);
    /* All replay spans must be mutually disjoint across the whole cohort.
     * No later row can silently overwrite another bridge's before/target. */
    for(i=0;!z&&i<brr_plan.spans;i++)for(j=0;j<i;j++) {
        const BrgDescriptor *a=brr_plan.span+i,*b=brr_plan.span+j;
        if(!brg_apart(a->address,(a->count-1)*a->stride+a->width,
                      b->address,(b->count-1)*b->stride+b->width)) { z=BRR_BYTES;break; }
    }
    for(i=0;!z&&i<brr_plan.spans;i++)for(j=0;j<brr_plan.reads;j++) {
        const BrgDescriptor *d=brr_plan.span+i;
        const BrgReadRange *r=brr_plan.read+j;
        if(!brg_apart(d->address,(d->count-1)*d->stride+d->width,r->address,r->bytes)) { z=BRR_BYTES;break; }
    }
    for(i=0;!z&&i<brr_plan.reads;i++)for(j=0;j<i;j++) {
        const BrgReadRange *a=brr_plan.read+i,*b=brr_plan.read+j;
        if(a->strict && b->strict && !brg_apart(a->address,a->bytes,b->address,b->bytes)) { z=BRR_BYTES;break; }
    }
    if(!z && (_gz_osa_validate(wsp_source.pin) || wsp_source.source_hash!=brr_plan.source_hash ||
       wsp_source.identity!=brr_plan.identity || wsp_source.generation!=brr_plan.generation))z=BRR_SOURCE;
    if(!z) { brr_plan.phase=BRR_PREPARED;brr_plan.valid=1;brr_seal();z=brr_current(2); }
    z=brr_leave(z);
    if(z)brg_zero(&brr_plan,sizeof(brr_plan));
    return z;
}

u32 _gz_brr_prepared_before(u32 token,u32 frame)
{
    u32 z;
    if(!_gz_brr_sealed(token) || brr_plan.phase!=BRR_PREPARED)return BRR_CONTEXT;
    z=brr_enter(token,frame,1);if(z)return z;return brr_leave(brr_current(2));
}

/* Private integer-only tails. Every destination and complete before image is
 * qualified under the same real hold BEFORE the first store. No fallible API,
 * source read, allocator, native method or yield occurs inside these tails. */
static void brr_copy_tail(u32 target)
{
    u32 i,j,k;
    const u8 *scalars=target?brr_plan.scalar_target:brr_plan.scalar_before;
    const u8 *geometry=target?brr_plan.target:brr_plan.before;
    for(i=0;i<brr_plan.rows;i++) {
        const BrrRow *r=brr_plan.row+i;
        for(j=7;j<r->scalar_bytes;j++)
            *(volatile u8*)(r->binding.process.object+brg3_scalar_offset(j))=scalars[r->scalar_offset+j];
    }
    for(i=0;i<brr_plan.spans;i++) {
        const BrgDescriptor *d=brr_plan.span+i;
        for(j=0;j<d->count;j++)for(k=0;k<d->width;k++)
            *(volatile u8*)(d->address+j*d->stride+k)=geometry[d->packed+j*d->width+k];
    }
    __asm__ volatile("sync" ::: "memory");
}

u32 _gz_brr_forward(u32 token,u32 frame)
{
    u32 z;
    if(!_gz_brr_sealed(token) || brr_plan.phase!=BRR_PREPARED)return BRR_CONTEXT;
    z=brr_enter(token,frame,2);if(z)return z;
    z=brr_current(2);if(z)return brr_leave(z);
    brr_plan.phase=BRR_FORWARD;brr_plan.attempted=1;brr_seal();
    brr_copy_tail(1);
    brr_plan.applied=1;brr_plan.phase=BRR_APPLIED;brr_seal();
    return brr_leave(brr_current(1));
}

u32 _gz_brr_target(u32 token,u32 frame)
{
    u32 z;
    if(!_gz_brr_sealed(token) || brr_plan.phase!=BRR_APPLIED || !brr_plan.applied)return BRR_CONTEXT;
    z=brr_enter(token,frame,4);if(z)return z;
    z=brr_current(1);
    if(!z) { brr_plan.target_frame=frame;brr_seal(); }
    return brr_leave(z);
}

u32 _gz_brr_undo(u32 token,u32 frame)
{
    u32 z;
    if(!_gz_brr_sealed(token) || (brr_plan.phase!=BRR_PREPARED &&
       brr_plan.phase!=BRR_FORWARD && brr_plan.phase!=BRR_APPLIED))return BRR_CONTEXT;
    z=brr_enter(token,frame,3);if(z)return z;
    z=brr_current(brr_plan.attempted?0:2);if(z)return brr_leave(z);
    brr_plan.phase=BRR_UNDO;brr_seal();
    if(brr_plan.attempted)brr_copy_tail(0);
    brr_plan.undone=1;brr_plan.phase=BRR_ROLLED;brr_seal();
    return brr_leave(brr_current(2));
}

u32 _gz_brr_before(u32 token,u32 frame)
{
    u32 z;
    if(!_gz_brr_sealed(token) || brr_plan.phase!=BRR_ROLLED || !brr_plan.undone)return BRR_CONTEXT;
    z=brr_enter(token,frame,3);if(z)return z;return brr_leave(brr_current(2));
}

u32 _gz_brr_commit_scope(u32 token,u32 frame)
{
    return !brr_entered && _gz_brr_sealed(token) && brr_plan.phase==BRR_APPLIED &&
        brr_plan.applied==1 && brr_plan.target_frame==frame && frame==_gz_pbac_frame;
}

u32 _gz_brr_forget_clean(u32 token)
{
    if(!brr_plan.magic)return 0;
    if(brr_entered || brr_plan.token!=token || brr_plan.attempted ||
       (brr_plan.valid && brr_plan.phase!=BRR_PREPARED))return BRR_CONTEXT;
    brg_zero(&brr_plan,sizeof(brr_plan));return 0;
}

void _gz_brr_finish_leaf(u32 token)
{
    if(brr_entered || brr_plan.token!=token || !brr_plan.valid ||
       (brr_plan.phase!=BRR_APPLIED && brr_plan.phase!=BRR_ROLLED))return;
    brg_zero(&brr_plan,sizeof(brr_plan));
}
