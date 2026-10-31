#ifndef GZ_METADATA_HASH_PLAN_H
#define GZ_METADATA_HASH_PLAN_H

typedef unsigned int HhtU;
enum {
    HHT_OK=0,HHT_ARGUMENT=1,HHT_SPACE=2,HHT_FORMAT=3,
    HHT_PROTECTED=4,HHT_DUPLICATE=5,HHT_READER=6,
    HHT_EMPTY=0,HHT_LIVE_OUTSIDE=1,HHT_RETIRED=2,HHT_EXTENDED=4,
    HHT_PRIMARY_KEY=1,HHT_ALLOCATION_KEY=2
};

typedef struct {HhtU primary,secondary,descriptor,flags;} HhtCell;
typedef struct {HhtU slots,shift,sources,mode;} HhtSpec;
/* Readers project private metadata only. They have no native pointer,
 * allocator, actor or resource permission. The native binder must supply
 * fixed compiled readers under its genuine metadata locks and current hold.
 */
typedef HhtU (*HhtReader)(const void *context,HhtU index,HhtCell *output);
typedef struct {HhtReader current,source;const void *current_context,*source_context;} HhtInput;

/* Mirrors the actual HL probing rule: match before empty, reuse the first
 * tombstone only after the existing-key search reaches empty/end. Source
 * indices are not destinations. Protected live outside entries never move.
 * Shadow is temporary workspace and may change on failure. Destinations
 * remain unchanged until the entire source set is placed successfully.
 */
HhtU _gz_hht_plan(const HhtSpec *spec,const HhtInput *input,
                  HhtU *shadow,HhtU *destinations,HhtU *written);

#endif
