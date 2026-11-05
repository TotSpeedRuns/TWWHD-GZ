#ifndef GZ_STABLE_LIFETIME_HASH_INDEX_H
#define GZ_STABLE_LIFETIME_HASH_INDEX_H
typedef unsigned int HxU;
enum {HX_ROWS=262144,HX_MASK=262143,HX_NONE=262144,HX_BAD_ROW=0xffffffffu,
 HX_MAGIC=0x48584831u,HX_VERSION=2};
/* Exact packed native storage. No fields or row addresses are changed. */
typedef struct {HxU pointer,generation,heap,heap_generation,requested,alignment;
 HxU block,capacity,born_frame,end_frame,birth_site,end_site,release_token,heap_epoch,policy;} HxRow;
typedef struct {HxU leaf[8192],middle[256],upper[8],root;} HxBits;
typedef struct {HxU magic,bytes,version,ready;
 HxU occupied,generation,reserved[2];
 HxU buckets[HX_ROWS],next[HX_ROWS];HxBits empty,dead;} HxIndex;
_Static_assert(sizeof(HxU)==4&&sizeof(HxRow)==60,"native row ABI");
_Static_assert(sizeof(HxBits)==33828,"bitmap hierarchy ABI");
_Static_assert(sizeof(HxIndex)==2164840,"stable index storage ABI");
/* Private metadata only, under the existing common lifetime lock.
 * Rebuild enumerates every primary row. It is not an owner/lifetime shortcut.
 * Never read or modify native allocations through these functions.
 */
HxU _gz_hx_rebuild(HxIndex*,const HxRow*);
/* Exact old selector result as row+1, zero absent, BAD_ROW for malformed index.
 * custom==0 matches(pointer,heap); custom!=0 additionally kind4/descriptor.
 * Duplicate keys retain the original circular-probe first-match semantics.
 */
HxU _gz_hx_find(const HxIndex*,const HxRow*,HxU pointer,HxU heap,
 HxU descriptor,HxU custom,HxU create);
/* Called under the same lock immediately after one genuine primary-row write;
 * old_pointer must be captured before that write. Every pointer/state writer,
 * including journal forward/inverse, must synchronize before queries resume.
 * Returns0 on a malformed preexisting chain; production must flag/refuse it.
 */
HxU _gz_hx_row_changed(HxIndex*,const HxRow*,HxU row,HxU old_pointer);
HxU _gz_hx_next_empty(const HxIndex*,HxU start);
HxU _gz_hx_next_dead(const HxIndex*,HxU start);
/* Bounded all-candidate pointer walk. Caller owns both outputs and holds the
 * same common HL lock as every index writer. A cursor is not a native lease. */
typedef struct {HxU pointer,start,edge,node,visited,generation,reserved[2];} HxCursor;
enum {HX_WALK_OK=0,HX_WALK_DONE=1,HX_WALK_BAD=2,HX_WALK_CHANGED=3};
_Static_assert(sizeof(HxCursor)==32,"cursor ABI");
HxU _gz_hx_cursor_begin(const HxIndex*,HxU,HxCursor*);
HxU _gz_hx_cursor_next(const HxIndex*,const HxRow*,HxCursor*,HxU*);
#endif
