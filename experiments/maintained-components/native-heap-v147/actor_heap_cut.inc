/* A NEW fixed metadata consumer inside the actual HG/job hold. The parent
 * empty-hold contract does not cover this diagnostic. No callback pointer,
 * native owner/body read or allocator is used. Report publication waits for
 * the matching actual cleanup path; a retained token can never yield usable1.
 */
#include "heap_domain_cut.h"
#define JGD_FN static __attribute__((noinline, noclone, section(".text.gz_jgd")))
extern uint32_t _gz_jol_context_frame(uint32_t cookie);
extern uint32_t _gz_jol_context_parent(uint32_t cookie);

typedef struct {
    uint32_t pending, called, status, pre_hg, pre_job, post_hg, post_job, source_mask, clean;
} JgdAttempt;
static uint32_t jgd_last_frame __attribute__((section(".bss.gz_jgd")));
static uint32_t jgd_halted __attribute__((section(".bss.gz_jgd")));

/* JGD1/256/v1. A new ABI, not JBM64. Fixed DATA input+256/cut+272.
 * Sequence is word3; raw MAX means no call. See CONTRACT.md. */
typedef struct {
    volatile uint32_t report[64];
    GhdLabels input;
    GhdCut cut;
} JgdStorage;
typedef char JgdStorage3600[sizeof(JgdStorage)==3600?1:-1];
JgdStorage _gz_jgd_storage __attribute__((section(".data.gz_jgd"))) = {
    .report = {
        [0]=0x4a474431u,[1]=256,[2]=1,
        [16]=UINT32_MAX,[17]=UINT32_MAX,[18]=UINT32_MAX,[19]=UINT32_MAX,
        [20]=UINT32_MAX,[21]=UINT32_MAX,[22]=UINT32_MAX,
        [26]=300,[28]=15,[30]=31,[31]=3328
    }
};
#define _gz_jgd_report (_gz_jgd_storage.report)
extern volatile uint32_t _gz_ghd_game0_slot;
extern volatile uint32_t _gz_ghd_game1_slot;
extern volatile uint32_t _gz_ghd_zelda_slot;
JGD_FN void jgd_load_labels(GhdLabels*out) {
    out->game0=_gz_ghd_game0_slot;
    out->game1=_gz_ghd_game1_slot;
    out->zelda=_gz_ghd_zelda_slot;
    out->reserved=0;
}
JGD_FN __attribute__((noipa)) uint32_t jgd_same_labels(const GhdLabels*a,const GhdLabels*b) {
    return !a->reserved&&!b->reserved&&a->game0==b->game0&&a->game1==b->game1&&a->zelda==b->zelda;
}

JGD_FN __attribute__((noipa)) uint32_t jgd_due(uint32_t frame, uint32_t previous) {
    uint32_t delta = frame - previous;
    return frame && (!previous || (delta >= 300 && delta < 0x80000000u));
}

JGD_FN __attribute__((noipa)) uint32_t jgd_sequence(uint32_t sequence, uint32_t halted) {
    if (halted || (sequence & 1) || sequence > UINT32_MAX - 3u) return 0;
    return sequence == UINT32_MAX - 3u ? 2u : 1u;
}

JGD_FN __attribute__((noipa)) uint32_t jgd_cut_valid(const GhdCut*c,const GhdLabels*labels) {
    uint32_t i; const uint32_t*values=(const uint32_t*)labels;
    if(c->magic!=0x47484431u||c->bytes!=3328||c->version!=1||!c->nodes||c->nodes>GHD_NODES||
       c->global_heaps>HL_HEAPS||c->global_allocations>HL_ALLOCS||c->selected_heaps>c->global_heaps||
       c->selected_allocations>c->global_allocations||c->unknown_chain_heaps>c->global_heaps||
       c->unknown_chain_indexed>c->global_allocations||(c->flags&~7u)||c->missing!=31||c->session||labels->reserved)return 0;
    for(i=0;i<GHD_ROOTS;i++) {
        const GhdRoot*r=&c->root[i];
        if(!values[i]||r->label!=values[i]||!r->node||r->node>c->nodes||!r->heaps||r->heaps>c->selected_heaps||
           r->live>c->selected_allocations||r->ending>c->selected_allocations-r->live||
           r->reserved[0]||r->reserved[1]||r->world||r->read)return 0;
    }
    return 1; /* Format/source join only, no physical permission. */
}

JGD_FN void jgd_fault(uint32_t reason) {
    if (!_gz_jgd_report[11]) _gz_jgd_report[11] = reason;
    jgd_halted = 1;
    _gz_jgd_report[4] = 2;
}

JGD_FN void jgd_count(uint32_t index) {
    if (_gz_jgd_report[index] == UINT32_MAX) jgd_fault(2);
    else ++_gz_jgd_report[index];
}

/* Called before HG acquisition, from the already authenticated fixed JLC
 * producer. Getters authenticate private context; cookie alone is no label.
 */
JGD_FN void jgd_plan(uint32_t cookie, JgdAttempt *attempt) {
    uint32_t frame, parent, thread, sequence, decision, i;
    if (jgd_halted) return;
    frame = _gz_jol_context_frame(cookie);
    parent = _gz_jol_context_parent(cookie);
    thread = _gz_hg_platform_thread();
    if (!frame || !parent || !thread || !jgd_due(frame, jgd_last_frame)) return;
    sequence = _gz_jgd_report[3];
    decision = jgd_sequence(sequence, jgd_halted);
    if (!decision) {
        jgd_halted = 1;
            return; /* Frozen/terminal report remains immutable. */
    }
    ++_gz_jgd_report[3];
    jol_barrier();
    if (decision == 2) {
        jgd_fault(1);
        jol_barrier();
        ++_gz_jgd_report[3];
        return; /* Reserved FC->FD->FE terminal pair; no gates/API. */
    }
    attempt->pending = 1;
    attempt->called = attempt->source_mask = attempt->clean = 0;
    attempt->status = attempt->pre_hg = attempt->pre_job = UINT32_MAX;
    attempt->post_hg = attempt->post_job = UINT32_MAX;
    jgd_last_frame = frame; /* Failed/BUSY attempts also consume the interval. */
    jgd_count(5);
    _gz_jgd_report[4] = jgd_halted ? 2 : 1;
    _gz_jgd_report[12] = cookie; _gz_jgd_report[13] = parent;
    _gz_jgd_report[14] = thread; _gz_jgd_report[15] = frame;
    _gz_jgd_report[25] = 0;
    for(i=32;i<56;i++) _gz_jgd_report[i]=0;
    /* Exactly three scalar native reads, before any HG/job acquisition. */
    jgd_load_labels(&_gz_jgd_storage.input);
    _gz_jgd_report[32]=_gz_jgd_storage.input.game0;
    _gz_jgd_report[33]=_gz_jgd_storage.input.game1;
    _gz_jgd_report[34]=_gz_jgd_storage.input.zelda;
    _gz_jgd_report[35]=_gz_jgd_storage.input.reserved;
    _gz_jgd_report[53]=1;
}

JGD_FN uint32_t jgd_source_owned(uint32_t cookie) {
    return cookie && _gz_jgd_report[13] && _gz_jgd_report[14] && _gz_jgd_report[15] &&
        cookie == _gz_jgd_report[12] &&
        _gz_jol_context_frame(cookie) == _gz_jgd_report[15] &&
        _gz_jol_context_parent(cookie) == _gz_jgd_report[13] &&
        _gz_hg_platform_thread() == _gz_jgd_report[14];
}

/* Fixed direct consumer: actual HG seal and SAME static job Gate checks
 * bracket the real try-only metadata API. Only local stack diagnostics and
 * its fixed private3328B unpublished output are modified while both holds live.
 */
JGD_FN void jgd_consume(uint32_t cookie, JgdAttempt *attempt,
                       uint32_t pre_hg, uint32_t pre_job,
                       uint32_t actual_hg_token, GzJcaExclusive *actual_job_token) {
    if (!attempt->pending) return;
    attempt->pre_hg = pre_hg; attempt->pre_job = pre_job;
    if (jgd_source_owned(cookie)) attempt->source_mask |= 1;
    if (pre_hg == 0 && pre_job == GZ_JCA_OK && attempt->source_mask == 1) {
        attempt->called = 1;
        attempt->status = _gz_ob_try_actor_heap_cut(&_gz_jgd_storage.input, &_gz_jgd_storage.cut);
    }
    attempt->post_hg = _gz_hg_capture_seal(actual_hg_token);
    attempt->post_job = _gz_jwa_lifetime_check(actual_job_token, cookie);
    if (jgd_source_owned(cookie)) attempt->source_mask |= 2;
    attempt->clean = pre_hg == 0 && pre_job == GZ_JCA_OK &&
        attempt->post_hg == 0 && attempt->post_job == GZ_JCA_OK && attempt->source_mask == 3;
}

/* Parent has now executed its exact matching cleanup path. Raw failures and
 * retained actual tokens remain visible, never overwritten with fake zero.
 * usable1 additionally requires BOTH real cleanup calls succeeded and all
 * retained tokens disappeared. This report never grants WORLD/READ access.
 */
JGD_FN void jgd_publish(JgdAttempt *attempt) {
    uint32_t cleanup = 0, outstanding, healthy, labels_match=0;
    GhdLabels after;
    if (!attempt->pending) return;
    outstanding = _gz_jol_coupled_outstanding();
    if ((_gz_jol_report[31] & JOL_CALLED_JOB_LEAVE) && _gz_jol_report[23] == GZ_JCA_OK &&
        !(outstanding & 2u)) cleanup |= 1;
    if ((_gz_jol_report[31] & JOL_CALLED_HG_END) && _gz_jol_report[20] == 0 && !(outstanding & 1u)) cleanup |= 2;
    healthy=cleanup==3&&!outstanding&&!_gz_jol_coupled_halted();
    if(healthy) {
        /* Exactly three scalar reads AFTER both genuine cleanup calls. */
        jgd_load_labels(&after);
        _gz_jgd_report[36]=after.game0; _gz_jgd_report[37]=after.game1;
        _gz_jgd_report[38]=after.zelda; _gz_jgd_report[39]=after.reserved;
        _gz_jgd_report[54]=1;
        labels_match=jgd_same_labels(&_gz_jgd_storage.input,&after);
        if(jgd_source_owned(_gz_jgd_report[12]))attempt->source_mask|=4;
    }
    _gz_jgd_report[40]=labels_match;
    _gz_jgd_report[52]=(attempt->source_mask&4)?1:0;
    _gz_jgd_report[16] = attempt->status;
    _gz_jgd_report[17] = attempt->pre_hg; _gz_jgd_report[18] = attempt->pre_job;
    _gz_jgd_report[19] = attempt->post_hg; _gz_jgd_report[20] = attempt->post_job;
    _gz_jgd_report[21] = _gz_jol_report[23]; _gz_jgd_report[22] = _gz_jol_report[20];
    _gz_jgd_report[23] = attempt->source_mask; _gz_jgd_report[24] = cleanup;
    if (attempt->called) {
        jgd_count(6);
        if (attempt->status == OB_OK) jgd_count(7);
        else if (attempt->status == OB_BUSY) jgd_count(8);
        else if (attempt->status == OB_BUDGET) jgd_count(9);
        else jgd_count(10);
        if (attempt->status != OB_OK && attempt->status != OB_BUSY &&
            attempt->status != OB_BUDGET && attempt->status != OB_OBSERVER && attempt->status != OB_ABSENT) jgd_fault(3);
    } else jgd_count(10);
    if (attempt->called && attempt->status == OB_OK) {
        const GhdCut*c=&_gz_jgd_storage.cut;
        _gz_jgd_report[41]=c->flags; _gz_jgd_report[42]=c->nodes;
        _gz_jgd_report[43]=c->selected_heaps; _gz_jgd_report[44]=c->selected_allocations;
        _gz_jgd_report[45]=c->global_heaps; _gz_jgd_report[46]=c->global_allocations;
        _gz_jgd_report[47]=c->unknown_chain_heaps; _gz_jgd_report[48]=c->unknown_chain_indexed;
        _gz_jgd_report[49]=c->serial; _gz_jgd_report[50]=c->release_serial; _gz_jgd_report[51]=c->frame;
        if(!attempt->clean||!healthy||!jgd_cut_valid(c,&_gz_jgd_storage.input))jgd_fault(4);
        else if(!jgd_halted&&attempt->source_mask==7&&labels_match)_gz_jgd_report[25]=1;
        /* Post-cleanup label/context change is an unusable observation,
         * never an invented API status or a physical ownership claim. */
    }
    if (outstanding || _gz_jol_coupled_halted()) {
        jgd_halted = 1;
        _gz_jgd_report[4] = 2;
    } else if (!jgd_halted) _gz_jgd_report[4] = 0;
    jol_barrier();
    ++_gz_jgd_report[3];
    jol_barrier();
}
#undef _gz_jgd_report
#undef JGD_FN
