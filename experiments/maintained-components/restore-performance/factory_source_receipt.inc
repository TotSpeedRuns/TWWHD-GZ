/* Saved archive validation only, inside the genuine BMH five-view READ hold.
 * Every original sample/current/source-bridge/PO/CO/native predicate runs.
 * This receipt is independent of the existing WGP PREPARE receipt. */
#include "factory_read_scope.h"

enum {FSR_MINT = 1, FSR_ACTIVE = 2, FSR_VERIFY = 3, FSR_POISON = 4};

typedef struct {
    GzSourceReceiptKey source;
    GzFactoryReadScope purpose;
} GzFactorySourceKey;

static struct {
    u32 state;
    u32 entered;
    u32 poison;
    u32 valid;
    GzFactorySourceKey key;
    GzSourceArchive saved;
    u32 seal;
} factory_source;

/* Diagnostics only; never consumed by admission or recovery. */
u32 _gz_factory_source_report[16] = {0x46535231u, 64, 1};

static void fsr_count(u32 index)
{
    if (_gz_factory_source_report[index] != 0xffffffffu) {
        _gz_factory_source_report[index]++;
    }
}

static void fsr_clear(void)
{
    u32 i;

    for (i = 0; i < sizeof(factory_source) / 4; i++) {
        ((volatile u32 *)&factory_source)[i] = 0;
    }
}

static u32 fsr_seal(void)
{
    return hash((u32)&factory_source.key,
                sizeof(factory_source.key) + sizeof(factory_source.saved));
}

static u32 fsr_key_equal(const GzFactorySourceKey *a, const GzFactorySourceKey *b)
{
    const u32 *x = (const u32 *)a;
    const u32 *y = (const u32 *)b;
    u32 i;

    for (i = 0; i < sizeof(*a) / 4; i++) {
        if (x[i] != y[i]) {
            return 0;
        }
    }
    return 1;
}

/* Copy only the immutable saved-data result. The caller has already proved
 * the complete current key; this helper itself grants no read permission. */
static __attribute__((noinline, noclone)) u32
fsr_project(RohSample *sample, const GzSourceArchive *saved, u32 pot_work)
{
    if (!span(pot_work, saved->pot_bytes)) {
        return 0;
    }
    sample->archive = saved->archive;
    sample->archive_bytes = saved->archive_bytes;
    sample->archive_version = saved->archive_version;
    sample->components = saved->components;
    sample->directory_hash = saved->directory_hash;
    sample->archive_hash = saved->archive_hash;
    sample->pot_source = saved->pot_source;
    sample->pot_bytes = saved->pot_bytes;
    sample->pot_hash = saved->pot_hash;
    sample->pot_work = pot_work;
    return 1;
}

static u32 fsr_key(u32 token, u32 frame, GzFactorySourceKey *key)
{
    const RohOwner *owner = &_gz_roh_owner;
    const RohSample *sample = &owner->sample;
    const GzFactoryReadScope *purpose;
    GzSourceReceiptKey *source = &key->source;
    u32 pin;
    u32 thread;
    u32 i;

    if (!owned(token) || failed() || owner->phase != ROH_MUTATING ||
        owner->mutated != 1 || owner->error || !owner->frame_open ||
        owner->inflight < ROH_LIGHT || owner->inflight > ROH_POT ||
        owner->last_frame != frame || frame != _gz_roh_live_frame || !frame ||
        sample->epoch != _gz_roh_live_epoch || sample->slot > 9 ||
        sample->slot != _gz_slots_selected ||
        !choice_current(_gz_roh_pinned_choice) || !sample->registry_revision ||
        sample->registry_revision != _gz_roh_registry_revision) {
        return 0;
    }
    purpose = _gz_bmh_source_context(token, frame);
    if (!purpose) {
        return 0;
    }
    thread = _gz_hg_platform_thread();
    if (!thread || thread != purpose->thread) {
        return 0;
    }
    for (i = 0; i < sizeof(*purpose) / 4; i++) {
        ((u32 *)&key->purpose)[i] = ((const u32 *)purpose)[i];
    }
    pin = _gz_osa_pin_id();
    if (!pin || pin != key->purpose.pin || _gz_osa_validate(pin)) {
        return 0;
    }
    source->token = token;
    source->serial = _gz_roh_serial;
    source->frame = frame;
    source->epoch = sample->epoch;
    source->thread = thread;
    source->request = key->purpose.request;
    source->ack = key->purpose.ack;
    source->choice = _gz_roh_pinned_choice;
    source->slot = sample->slot;
    source->selected = _gz_slots_selected;
    source->pin = pin;
    source->source = _gz_osa_source(pin);
    source->used = _gz_osa_bytes(pin);
    source->identity = _gz_osa_pin_identity(pin);
    source->generation = _gz_osa_pin_generation(pin);
    source->capacity = _gz_osa_capacity_for(source->source);
    source->raw_hash = _gz_osa_pin_hash(pin);
    source->revision = _gz_roh_registry_revision;
    source->service = owner->inflight;
    source->service_calls = owner->service_calls;
    source->start_frame = owner->start_frame;
    source->last_frame = owner->last_frame;
    source->limit = owner->limit;
    source->hold = key->purpose.hold;
    if (!source->identity || !source->generation || source->serial != token ||
        source->source != sample->archive || source->used != sample->archive_bytes ||
        source->source != key->purpose.source || source->used != key->purpose.used ||
        source->used > source->capacity || !span(source->source, source->used) ||
        source->service != key->purpose.service ||
        source->service_calls != key->purpose.service_calls) {
        return 0;
    }
    purpose = _gz_bmh_source_context(token, frame);
    if (!purpose || !equal_bytes((u32)purpose, (u32)&key->purpose, sizeof(*purpose))) {
        return 0;
    }
    thread = _gz_hg_platform_thread();
    purpose = _gz_bmh_source_context(token, frame);
    return purpose && equal_bytes((u32)purpose, (u32)&key->purpose, sizeof(*purpose)) &&
        owned(token) && !failed() && owner->phase == ROH_MUTATING &&
        owner->mutated == 1 && !owner->error && owner->frame_open &&
        owner->inflight == source->service && owner->service_calls == source->service_calls &&
        frame == _gz_roh_live_frame && owner->last_frame == frame &&
        source->revision == _gz_roh_registry_revision &&
        source->epoch == _gz_roh_live_epoch && source->selected == _gz_slots_selected &&
        choice_current(source->choice) && pin == _gz_osa_pin_id() &&
        !_gz_osa_validate(pin) && thread == source->thread;
}

/* 0: unchanged full path; 1: saved projection; 2: active key lost/refused. */
static __attribute__((noinline, noclone)) u32
factory_archive_receipt(u32 archive, RohSample *sample)
{
    GzFactorySourceKey now;
    u32 ok;

    if (!factory_source.state && !factory_source.entered &&
        !factory_source.valid && !factory_source.poison) {
        return 0;
    }
    if ((factory_source.state == FSR_MINT || factory_source.state == FSR_VERIFY) &&
        factory_source.entered == 1 && !factory_source.valid) {
        return 0;
    }
    if (factory_source.entered) {
        factory_source.poison = 1;
        factory_source.valid = 0;
        return 2;
    }
    factory_source.entered = 1;
    ok = factory_source.state == FSR_ACTIVE && factory_source.valid == 1 &&
        !factory_source.poison && factory_source.seal == fsr_seal() &&
        archive == factory_source.key.source.source && owner_sample_call.open == 2 &&
        fsr_key(factory_source.key.source.token, factory_source.key.source.frame, &now) &&
        fsr_key_equal(&factory_source.key, &now);
    if (!ok || factory_source.poison || factory_source.state != FSR_ACTIVE ||
        factory_source.valid != 1) {
        factory_source.valid = 0;
        factory_source.poison = 1;
        factory_source.state = FSR_POISON;
        factory_source.entered = 0;
        fsr_count(7);
        return 2;
    }
    ok = fsr_project(sample, &factory_source.saved, (u32)_gz_roh_live_pot);
    if (!ok) {
        factory_source.valid = 0;
        factory_source.poison = 1;
        factory_source.state = FSR_POISON;
        fsr_count(7);
    } else {
        fsr_count(4);
    }
    factory_source.entered = 0;
    return ok ? 1 : 2;
}

u32 _gz_roh_factory_source_begin(u32 token, u32 frame)
{
    GzFactorySourceKey before;
    GzFactorySourceKey after;
    const RohSample *sample = &_gz_roh_owner.sample;
    u32 i;
    u32 result = ROH_ARCHIVE;

    fsr_count(3);
    if (factory_source.state || factory_source.entered ||
        factory_source.valid || factory_source.poison) {
        factory_source.poison = 1;
        factory_source.valid = 0;
        return ROH_REENTRY;
    }
    factory_source.state = FSR_MINT;
    factory_source.entered = 1;
    if (_gz_source_receipt.state || _gz_source_receipt.entered ||
        _gz_source_receipt.valid || _gz_source_receipt.poison ||
        !fsr_key(token, frame, &before)) {
        goto done;
    }
    result = _gz_roh_replay_common(token, frame, 1);
    if (result) {
        goto done;
    }
    result = ROH_ARCHIVE;
    if (!fsr_key(token, frame, &after) || !fsr_key_equal(&before, &after) ||
        factory_source.poison || factory_source.state != FSR_MINT) {
        goto done;
    }
    for (i = 0; i < sizeof(after) / 4; i++) {
        ((volatile u32 *)&factory_source.key)[i] = ((const u32 *)&after)[i];
    }
    factory_source.saved.archive = sample->archive;
    factory_source.saved.archive_bytes = sample->archive_bytes;
    factory_source.saved.archive_version = sample->archive_version;
    factory_source.saved.components = sample->components;
    factory_source.saved.directory_hash = sample->directory_hash;
    factory_source.saved.archive_hash = sample->archive_hash;
    factory_source.saved.pot_source = sample->pot_source;
    factory_source.saved.pot_bytes = sample->pot_bytes;
    factory_source.saved.pot_hash = sample->pot_hash;
    factory_source.seal = fsr_seal();
    factory_source.state = FSR_ACTIVE;
    factory_source.entered = 0;
    __asm__ volatile("sync" ::: "memory");
    factory_source.valid = 1;
    _gz_factory_source_report[8] = token;
    _gz_factory_source_report[9] = frame;
    _gz_factory_source_report[10] = after.source.service;
    _gz_factory_source_report[11] = after.purpose.serial;
    _gz_factory_source_report[12] = after.source.used;
    return 0;
done:
    fsr_count(7);
    fsr_clear();
    return result;
}

u32 _gz_roh_factory_source_end(u32 token, u32 frame, u32 result)
{
    GzFactorySourceKey before;
    GzFactorySourceKey after;
    u32 verified = ROH_ARCHIVE;
    u32 refused;

    fsr_count(5);
    if (factory_source.entered) {
        factory_source.poison = 1;
        factory_source.valid = 0;
        return result ? result : ROH_REENTRY;
    }
    factory_source.entered = 1;
    refused = factory_source.state != FSR_ACTIVE || factory_source.valid != 1 ||
        factory_source.poison;
    if ((factory_source.state != FSR_ACTIVE && factory_source.state != FSR_POISON) ||
        factory_source.key.source.token != token ||
        factory_source.key.source.frame != frame || factory_source.seal != fsr_seal() ||
        !fsr_key(token, frame, &before) ||
        !fsr_key_equal(&factory_source.key, &before)) {
        goto done;
    }
    /* A prior refusal cannot skip the original full exit check while the
     * same issued physical/source key is still current. A revoked or damaged
     * key above is never followed. Poison remains a refusal after checking. */
    factory_source.valid = 0;
    __asm__ volatile("sync" ::: "memory");
    factory_source.state = FSR_VERIFY;
    verified = _gz_roh_replay_common(token, frame, 1);
    if (verified) {
        goto done;
    }
    verified = ROH_ARCHIVE;
    if (!fsr_key(token, frame, &after) || !fsr_key_equal(&before, &after) ||
        factory_source.state != FSR_VERIFY) {
        goto done;
    }
    fsr_count(6);
    verified = refused || factory_source.poison ? ROH_ARCHIVE : 0;
done:
    if (verified) {
        fsr_count(7);
    }
    _gz_factory_source_report[13] = verified;
    _gz_factory_source_report[14] = result;
    fsr_clear();
    return result ? result : verified;
}
