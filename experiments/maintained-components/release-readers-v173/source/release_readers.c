#include "hash_index.h"
typedef HxU U;
typedef struct {
    U magic,bytes,version,busy,flags,serial,release_serial,active_heaps;
    U active_allocations,births,allocations,release_begin,release_end;
    U heap_end,dropped,session;
} HlControl;
extern HlControl _gz_hl_control;
extern HxIndex _gz_hx_index;
extern HxRow _gz_hl_allocations[HX_ROWS];
extern U _gz_hx_existing_ready(void);
_Static_assert(sizeof(HlControl)==64,"retained Native HL control");

static HxRow *bad(void)
{
    /* Same fail-closed cache policy as the existing runtime. The genuine
     * Native completion still supplies its stale flag and unlock path.
     */
    _gz_hx_index.ready=0;
    __sync_synchronize();
    __sync_fetch_and_or(&_gz_hl_control.flags,
        _gz_hx_index.generation==0xffffffffu ? 1024u : 1u);
    return 0;
}

static U acyclic(U node)
{
    U checkpoint=node,power=1,span=0,visited=0;
    /* Validate only existing index links, before the yielding cursor. This
     * also catches cycles containing no requested-pointer candidate; those
     * otherwise stay inside one cursor_next call until its full row budget.
     */
    while (node) {
        if (node>HX_ROWS || visited++==HX_ROWS) return 0;
        node=_gz_hx_index.next[node-1];
        if (node && node==checkpoint) return 0;
        if (++span==power) {
            checkpoint=node;
            if (power<HX_ROWS) power<<=1;
            span=0;
        }
    }
    return 1;
}

/* Exact release_find selection under its genuine caller-held HL lock.
 * No allocation/heap/native payload is dereferenced. Rows, index and controls
 * are existing module metadata, not a physical lease on the returned pointer.
 */
HxRow *_gz_hx_release_select(U pointer,U heap,U token)
{
    HxCursor cursor;
    U id,code,best=HX_NONE,start;
    if (!pointer) return 0; /* Native release loop tests EMPTY before a match. */
    /* Exact static ready() used by the existing selector: common-lock, header
     * and cold initialization admission. The cursor validates the entire
     * relevant bucket before any selected row is published to Native code.
     */
    if (!_gz_hx_existing_ready()) return 0;
    if (_gz_hx_cursor_begin(&_gz_hx_index,pointer,&cursor)!=HX_WALK_OK)
        return bad();
    if (!acyclic(cursor.node)) return bad();
    start=cursor.start;
    while ((code=_gz_hx_cursor_next(&_gz_hx_index,_gz_hl_allocations,
                                   &cursor,&id))==HX_WALK_OK) {
        HxRow *row=_gz_hl_allocations+id-1;
        U at=id-1;
        if (row->heap==heap && row->policy>>29==2 &&
            row->release_token==token &&
            (best==HX_NONE || ((at-start)&HX_MASK)<((best-start)&HX_MASK)))
            best=at;
    }
    if (code!=HX_WALK_DONE) return bad();
    return best==HX_NONE ? 0 : _gz_hl_allocations+best;
}
