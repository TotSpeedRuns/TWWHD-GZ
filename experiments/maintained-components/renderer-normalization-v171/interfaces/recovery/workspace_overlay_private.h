#ifndef GZ_RECOVERY_WORKSPACE_OVERLAY_PRIVATE_H
#define GZ_RECOVERY_WORKSPACE_OVERLAY_PRIVATE_H
/* Exact private layouts from the hash-pinned v154 lifetime/OIS translation
 * units. These declarations name existing loader-relocated objects. They
 * allocate no replacement state and contain no runtime preferred addresses.
 */
#include <stddef.h>
#include <stdint.h>
#include "owned_images.h"
#include "heap_domain_cut.h"
#include "owned_heap_native.h"
#include "owned_heap_process.h"
#include "owned_heap_common_domain.h"

typedef struct { uint32_t heap, pointer, capacity, identity; } WsRegion;
typedef struct {
    WsRegion region;
    uint32_t used, type, generation, readers;
    AfDomain domain;
} WsSlot;
typedef struct { uint32_t active, slot; OisLease lease; } WsPin;
typedef struct {
    uint32_t phase, token, code, slot, thread;
    OisLease source;
    WsRegion undo, map;
    AjRestoreRequest request;
    AjRestoreResult result;
} WsTx;
typedef struct {
    uint32_t magic, version, bytes, serial, inverse, entered, thread, writer;
    WsSlot slots[OIS_SLOTS];
    WsPin pins[OIS_PINS];
    WsRegion staging;
    WsTx tx;
} WsState;
typedef struct {
    uint32_t phase, request, frame, thread, used, result, purpose;
    uint32_t role, target, image_version;
} WsHeap;
typedef struct {
    uint32_t phase, request, frame, thread, slot, used, generation, token;
    OisLease saved;
    WsRegion current, work;
    OisLease reader;
} WsCut;
typedef struct {
    uint32_t active, request, frame, raw, bytes, capacity;
    GhdLabels labels;
    HlHeap root;
    HmScope scope;
    uint32_t purpose, process_capacity, allocation_capacity;
} WsProposal;

_Static_assert(sizeof(WsRegion) == 16 && sizeof(WsSlot) == 80 && sizeof(WsPin) == 40,
    "published OIS region/slot/pin ABI");
_Static_assert(sizeof(WsTx) == 292 && sizeof(WsState) == 2420, "published OIS State ABI");
_Static_assert(sizeof(WsHeap) == 40 && sizeof(WsCut) == 128, "published OIS heap/cut ABI");
_Static_assert(sizeof(WsProposal) == 148, "published private proposal ABI");
_Static_assert(offsetof(WsState, slots) == 32 && offsetof(WsState, pins) == 832 &&
    offsetof(WsState, staging) == 2112 && offsetof(WsState, tx) == 2128,
    "published State offsets");
_Static_assert(offsetof(WsHeap, purpose) == 24 && offsetof(WsHeap, role) == 28 &&
    offsetof(WsHeap, target) == 32 && offsetof(WsHeap, image_version) == 36,
    "published heap offsets");
_Static_assert(offsetof(WsCut, saved) == 32 && offsetof(WsCut, current) == 64 &&
    offsetof(WsCut, work) == 80 && offsetof(WsCut, reader) == 96,
    "published cut offsets");
_Static_assert(offsetof(WsProposal, labels) == 24 && offsetof(WsProposal, root) == 40 &&
    offsetof(WsProposal, scope) == 104 && offsetof(WsProposal, purpose) == 136 &&
    offsetof(WsProposal, process_capacity) == 140 && offsetof(WsProposal, allocation_capacity) == 144,
    "published proposal offsets");

extern const volatile WsState _gz_ws_state;
extern volatile WsHeap _gz_ws_heap;
extern const volatile WsCut _gz_ws_cut;
extern const volatile WsProposal _gz_ws_proposal;
extern uint32_t _gz_ws_ois_header(void);
extern uint32_t _gz_ws_ghs_scope(uint32_t request, uint32_t frame, uint32_t phase);
/* Existing native preservation wrapper around the genuine coreinit query.
 * Bind only after verifying the exact baseline wrapper and its import record.
 * A guessed numeric thread identity or host response is not a substitute.
 */
extern void *_gz_ws_thread(void);
extern uint32_t _gz_ws_native_capture(uint32_t request, uint32_t frame,
    uint32_t pointer, uint32_t capacity, uint32_t *written);
uint32_t _gz_ws_overlay_capacity(uint32_t request, uint32_t frame,
    uint32_t original, uint32_t largest);
uint32_t _gz_ws_overlay_capture(uint32_t request, uint32_t frame,
    uint32_t pointer, uint32_t capacity, uint32_t *written);
#endif
