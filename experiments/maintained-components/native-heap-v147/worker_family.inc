/* Same TU as the actual static jwa_gate. No second Gate, body mutex or
 * application lock is added. Cold accounting does not call any OS API. */
#include "worker_family.h"
#include "worker_atomic.h"
#define JWF_FN __attribute__((noinline,noclone,section(".text.gz_jwf")))
static JwaInvocation jwf_invocation[JWF_RECORDS] __attribute__((section(".bss.gz_jwf")));
static uint32_t jwf_serial __attribute__((section(".bss.gz_jwf")));
/* JWF1/256/v1. Independent atomic words, NOT a coherent multiwriter seqlock:
 * 3sticky flags;4families;5records;6serialmax;7WORLD0;
 * 8starts;9returns;10cold;11shared calls;12sharedOK;13sharedfail;
 * 14leaveOK;15leavefail;16..18newWHOLE active;19old9 bootstrap active;20..22family starts;
 * 23..25returns;26..28cold;29..31sharedOK;40serial label;
 * 41zero checks;42BUSY;43zeroOK;44broken;45last real shared;46last leave;
 * 35old9cold starts;36old9cold returns;37cold gaps;38OPEN-races;
 * 47first failure kind+1;48..53first retained token;54publish state;
 * 55last private zero result;56WORLD0;57READ0;58missing15;59physical31;
 * 60session0;61sameGate1;62old9bootstrap kinds;63total12 kinds.
 * Other words reserved0. Raw MAX means no API call. */
volatile uint32_t _gz_jwf_report[64] __attribute__((section(".data.gz_jwf")))={
    [0]=0x4a574631u,[1]=256,[2]=1,[4]=JWF_FAMILIES,[5]=JWF_RECORDS,
    [6]=JWA_SERIAL_MAX,[45]=UINT32_MAX,[46]=UINT32_MAX,[55]=UINT32_MAX,
    [58]=15,[59]=31,[61]=1,[62]=9,[63]=12
};
enum {JWF_E_KIND=1,JWF_E_COUNT=2,JWF_E_POOL=4,JWF_E_SERIAL=8,
      JWF_E_CONTEXT=16,JWF_E_ENTER=32,JWF_E_RETURN=64,JWF_E_LEAVE=128};
static JWF_FN void jwf_barrier(void) { __asm__ volatile("sync":::"memory"); }
static JWF_FN void jwf_poison(uint32_t reason)
{
    _gz_jwf_atomic_or(&_gz_jwf_report[3],reason);
    _gz_jwf_atomic_or(&_gz_jwa_report[4],JWA_F_GATE);
    jwa_initialization=POISONED;_gz_jwa_report[3]=POISONED;
    jwf_barrier();
}
static JWF_FN void jwf_count(uint32_t index)
{
    if (index>=64 || !_gz_jwf_atomic_inc(&_gz_jwf_report[index])) jwf_poison(JWF_E_COUNT);
}
static JWF_FN uint32_t jwf_ordinary_context(void)
{
    uint32_t previous=(uint32_t)OSDisableInterrupts();
    uint32_t restore=(uint32_t)OSRestoreInterrupts((BOOL)previous);
    return previous!=0 && restore==0;
}
static JWF_FN void jwf_retain(const JwaInvocation *record)
{
    const uint32_t *token=(const uint32_t*)&record->token;
    uint32_t i;
    if (!_gz_jwf_atomic_cas(&_gz_jwf_report[54],0,1)) return;
    _gz_jwf_report[47]=record->kind+1;
    for (i=0;i<6;i++) _gz_jwf_report[48+i]=token[i];
    jwf_barrier();_gz_jwf_report[54]=2;
}
JWF_FN uint32_t _gz_jwf_enter(uint32_t family,uint32_t incoming_sp,uint32_t caller_lr)
{
    JwaInvocation *record=NULL;
    uint32_t i,serial,state,ordinary=0,admitted=UINT32_MAX;
    if (family>=JWF_FAMILIES) { jwf_poison(JWF_E_KIND);return 0; }
    /* This MUST precede every initialization observation, including COLD.
     * Saturation never wraps to zero. Untracked failure leaves it nonzero. */
    if (!_gz_jwf_atomic_inc(&_gz_jwf_report[16+family])) {
        jwf_poison(JWF_E_COUNT);return 0;
    }
    jwf_barrier();
    state=_gz_jwf_atomic_load(&jwa_initialization);
    jwf_count(8);jwf_count(20+family);
    if (!incoming_sp || (incoming_sp&7) || !caller_lr || (caller_lr&3)) {
        jwf_poison(JWF_E_CONTEXT);return 0;
    }
    if (state==OPEN && !_gz_jwf_atomic_load(&_gz_jwf_report[3])) {
        ordinary=jwf_ordinary_context();
        if (!ordinary) jwf_poison(JWF_E_CONTEXT);
    } else { jwf_count(10);jwf_count(26+family); }
    for (i=0;i<JWF_RECORDS;i++) {
        JwaIdentity identity;
        uint32_t live;
        if (!record && _gz_jwf_atomic_cas(&jwf_invocation[i].control,JWA_FREE,JWA_RESERVED))
            record=&jwf_invocation[i];
        live=_gz_jwa_identity_private(&jwf_invocation[i],&identity);
        if (live && identity.incoming_sp==incoming_sp) {
            /* A lost/unwound older return must not make a later call's LR
             * ambiguous. Forward this new body without rewriting its LR. */
            if (record) { jwf_barrier();record->control=JWA_FREE; }
            jwf_poison(JWF_E_RETURN);return 0;
        }
    }
    if (!record) { jwf_poison(JWF_E_POOL);return 0; }
    serial=_gz_jwf_atomic_issue(&jwf_serial,JWA_SERIAL_MAX);
    if (!serial) {
        jwf_poison(JWF_E_SERIAL);record->control=JWA_QUARANTINED;return 0;
    }
    record->serial=serial;record->incoming_sp=incoming_sp;record->caller_lr=caller_lr;
    record->kind=family;record->thread=record->depth=0;
    record->admission=UINT32_MAX;record->token.active=0;record->token.gate=NULL;
    if (ordinary && _gz_jwf_atomic_load(&jwa_initialization)==OPEN &&
        !_gz_jwf_atomic_load(&_gz_jwf_report[3])) {
        jwf_count(11);
        /* Shared bodies remain concurrent. Only the existing private common
         * metadata mutex/closed-reader condition can delay this admission. */
        admitted=_gz_jca_enter_shared(&jwa_gate,&record->token);
        record->admission=admitted;_gz_jwf_report[45]=admitted;
        if (admitted==GZ_JCA_OK && record->token.active==1 &&
            record->token.gate==&jwa_gate && record->token.thread &&
            record->token.slot<GZ_JCA_SLOTS && record->token.generation && record->token.depth) {
            record->thread=(uint32_t)(uintptr_t)record->token.thread;
            record->depth=record->token.depth;jwf_count(12);jwf_count(29+family);
        } else { jwf_count(13);jwf_poison(JWF_E_ENTER); }
    }
    _gz_jwf_report[40]=serial;
    jwf_barrier();record->control=(serial<<3)|JWA_LIVE;
    return 1; /* Only a real immutable LR record authorizes return rewriting. */
}
JWF_FN void _gz_jwf_after(uint32_t incoming_sp,uint32_t output[2])
{
    JwaInvocation *record=NULL;
    JwaIdentity selected={0};
    uint32_t i,matches=0,control=0,left;
    output[0]=output[1]=0;
    /* Recover exact LR from immutable nonreused identity BEFORE any OS call. */
    for (i=0;i<JWF_RECORDS;i++) {
        JwaIdentity identity;
        uint32_t found=_gz_jwa_identity_private(&jwf_invocation[i],&identity);
        if (found && identity.incoming_sp==incoming_sp) {
            record=&jwf_invocation[i];selected=identity;control=found;++matches;
        }
    }
    if (matches!=1 || !record || !selected.caller_lr || selected.kind>=JWF_FAMILIES ||
        !_gz_jwf_atomic_cas(&record->control,control,(selected.serial<<3)|JWA_RETIRING)) {
        jwf_poison(JWF_E_RETURN);return;
    }
    output[0]=selected.caller_lr;output[1]=1; /* All three native epilogues restore LR through r0. */
    if (record->token.active || record->token.gate) {
        if (!jwf_ordinary_context()) {
            record->control=(selected.serial<<3)|JWA_QUARANTINED;
            jwf_retain(record);jwf_poison(JWF_E_CONTEXT|JWF_E_LEAVE);return;
        }
        left=_gz_jca_leave_shared(&jwa_gate,&record->token);
        _gz_jwf_report[46]=left;
        if (left!=GZ_JCA_OK || record->token.active || record->token.gate) {
            record->control=(selected.serial<<3)|JWA_QUARANTINED;
            jwf_count(15);jwf_retain(record);jwf_poison(JWF_E_LEAVE);return;
        }
        jwf_count(14);
    }
    if (!_gz_jwf_atomic_dec(&_gz_jwf_report[16+selected.kind])) {
        record->control=(selected.serial<<3)|JWA_QUARANTINED;
        jwf_poison(JWF_E_COUNT);return;
    }
    jwf_count(9);jwf_count(23+selected.kind);
    jwf_barrier();record->control=JWA_FREE;
}
/* Called only when the old nine-entry OPEN fast check did not see OPEN.
 * Increment before the DECISIVE atomic observation: a racing transition to
 * OPEN must use the original shared admission, never a late cold body. */
JWF_FN uint32_t _gz_jwf_cold_jwa_enter(uint32_t kind,uint32_t incoming_sp,uint32_t caller_lr)
{
    JwaInvocation *record=NULL;
    uint32_t i,serial,state;
    if (!_gz_jwf_atomic_inc(&_gz_jwf_report[19])) { jwf_poison(JWF_E_COUNT);return 0; }
    jwf_barrier();state=_gz_jwf_atomic_load(&jwa_initialization);
    if (state==OPEN) {
        if (!_gz_jwf_atomic_dec(&_gz_jwf_report[19])) jwf_poison(JWF_E_COUNT);
        jwf_count(38);return JWF_OPEN_RACED;
    }
    if (state!=COLD && state!=INITIALIZING) {
        jwf_count(37);jwf_poison(JWF_E_CONTEXT);return 0;
    }
    jwf_count(35);
    if (kind>=JWA_KINDS || !incoming_sp || (incoming_sp&7) || !caller_lr || (caller_lr&3)) {
        jwf_count(37);jwf_poison(JWF_E_CONTEXT);return 0;
    }
    for (i=0;i<JWF_RECORDS;i++) {
        JwaIdentity identity;
        uint32_t live;
        if (!record && _gz_jwf_atomic_cas(&jwf_invocation[i].control,JWA_FREE,JWA_RESERVED))
            record=&jwf_invocation[i];
        live=_gz_jwa_identity_private(&jwf_invocation[i],&identity);
        if (live && identity.incoming_sp==incoming_sp) {
            if (record) { jwf_barrier();record->control=JWA_FREE; }
            jwf_count(37);jwf_poison(JWF_E_RETURN);return 0;
        }
    }
    if (!record) { jwf_count(37);jwf_poison(JWF_E_POOL);return 0; }
    serial=_gz_jwf_atomic_issue(&jwf_serial,JWA_SERIAL_MAX);
    if (!serial) {
        record->control=JWA_QUARANTINED;jwf_count(37);jwf_poison(JWF_E_SERIAL);return 0;
    }
    record->serial=serial;record->incoming_sp=incoming_sp;record->caller_lr=caller_lr;
    record->kind=JWF_FAMILIES+kind;record->thread=record->depth=0;
    record->admission=UINT32_MAX;record->token.active=0;record->token.gate=NULL;
    jwf_barrier();record->control=(serial<<3)|JWA_LIVE;
    return 1; /* Original nine-entry shim rewrites LR only for this real record. */
}
/* An old cold record has the SAME return shim as later original OPEN
 * records. Reject same-SP reuse before that later LR can be rewritten, even
 * after a nonlocal unwind left an immutable older cold LIVE identity. */
JWF_FN uint32_t _gz_jwf_cold_jwa_conflict(uint32_t incoming_sp)
{
    uint32_t i;
    if (!_gz_jwf_atomic_load(&_gz_jwf_report[19])) return 0;
    for (i=0;i<JWF_RECORDS;i++) {
        JwaIdentity identity;
        uint32_t found=_gz_jwa_identity_private(&jwf_invocation[i],&identity);
        if (found && gz_jwf_cold_identity_matches(identity.kind,identity.incoming_sp,incoming_sp)) {
            jwf_count(37);jwf_poison(JWF_E_RETURN);return 1;
        }
    }
    return 0;
}
JWF_FN uint32_t _gz_jwf_cold_jwa_after(uint32_t incoming_sp,uint32_t output[2])
{
    JwaInvocation *record=NULL;
    JwaIdentity selected={0};uint32_t i,matches=0,control=0;
    /* Full-time OPEN returns have only this fast atomic zero check. */
    if (!_gz_jwf_atomic_load(&_gz_jwf_report[19])) return 0;
    for (i=0;i<JWF_RECORDS;i++) {
        JwaIdentity identity;
        uint32_t found=_gz_jwa_identity_private(&jwf_invocation[i],&identity);
        if (found && identity.kind>=JWF_FAMILIES && identity.incoming_sp==incoming_sp) {
            record=&jwf_invocation[i];selected=identity;control=found;++matches;
        }
    }
    if (!matches) return 0; /* The original OPEN directory still owns this LR. */
    if (matches!=1 || !record || selected.kind>=JWF_TRACKED_KINDS || !selected.caller_lr ||
        !_gz_jwf_atomic_cas(&record->control,control,(selected.serial<<3)|JWA_RETIRING)) {
        jwf_poison(JWF_E_RETURN);return 1;
    }
    output[0]=selected.caller_lr;
    output[1]=(selected.kind-JWF_FAMILIES)!=2 && (selected.kind-JWF_FAMILIES)!=6;
    if (record->token.active || record->token.gate || record->admission!=UINT32_MAX ||
        !_gz_jwf_atomic_dec(&_gz_jwf_report[19])) {
        record->control=(selected.serial<<3)|JWA_QUARANTINED;
        jwf_poison(JWF_E_RETURN);return 1;
    }
    jwf_count(36);jwf_barrier();record->control=JWA_FREE;
    return 1;
}
JWF_FN uint32_t _gz_jwa_workers_zero(const GzJcaExclusive *token,uint32_t cookie)
{
    uint32_t status=JWF_ZERO_BROKEN,a,b,c,d;
    jwf_count(41);
    if (_gz_jwa_lifetime_check(token,cookie)!=GZ_JCA_OK) goto done;
    if (_gz_jwf_atomic_load(&_gz_jwf_report[3])) goto done;
    a=_gz_jwf_atomic_load(&_gz_jwf_report[16]);
    b=_gz_jwf_atomic_load(&_gz_jwf_report[17]);
    c=_gz_jwf_atomic_load(&_gz_jwf_report[18]);
    d=_gz_jwf_atomic_load(&_gz_jwf_report[19]);
    jwf_barrier();
    if (_gz_jwf_atomic_load(&_gz_jwf_report[3])) goto done;
    status=(a||b||c||d)?JWF_ZERO_BUSY:JWF_ZERO_OK;
done:
    _gz_jwf_report[55]=status;
    jwf_count(status==JWF_ZERO_OK?43:status==JWF_ZERO_BUSY?42:44);
    return status;
}
#undef JWF_FN
