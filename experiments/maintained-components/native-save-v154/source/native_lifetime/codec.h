#ifndef SLOT_COMPRESSION_CODEC_R298_H
#define SLOT_COMPRESSION_CODEC_R298_H
#include <stdint.h>
#define sc_measure _gz_och_sc_measure
#define sc_encode _gz_och_sc_encode
#define sc_decode _gz_och_sc_decode
#define sc_hash _gz_och_sc_hash
#define sc_validate _gz_och_sc_validate
#define sc_read_range _gz_och_sc_read_range
#define sc_matches _gz_och_sc_matches

enum {SC_OK=1,SC_KEEP_RAW=2,SC_BAD_ARGUMENT=3,SC_SPACE=4,SC_CORRUPT=5,SC_MAX=0x200000};
#ifdef _WIN32
#define SC_API __declspec(dllexport)
#else
#define SC_API
#endif
/* Source must be immutable for the call. SC_KEEP_RAW publishes no encoded
 * bytes: retain the original raw source unchanged. No automatic raw sniffing.
 * Caller accepts output only on SC_OK and retains its old source otherwise. */
/* Exact encoded length before allocation. Immutable input; no output buffer.
 * KEEP_RAW returns zero; result must be aligned and disjoint from input.
 * Callers compare rounded capacities and retain RAW on no physical saving. */
SC_API uint32_t sc_measure(const void*,uint32_t,uint32_t*);
SC_API uint32_t sc_encode(const void*,uint32_t,void*,uint32_t,uint32_t*);
/* Expected length/hash come from caller-owned metadata; stream fields alone
 * confer no source authority. FNV is corruption detection, not cryptography.
 * Decode into separate unpublished scratch; validity is set1 only on SC_OK.
 * Invalid/overlapping result pointers are not dereferenced. Always gate on
 * the returned status; a stale caller result word alone is never admission. */
SC_API uint32_t sc_decode(const void*,uint32_t,void*,uint32_t,uint32_t,uint32_t,uint32_t*);
SC_API uint32_t sc_hash(const void*,uint32_t);
SC_API uint32_t sc_validate(const void*,uint32_t,uint32_t,uint32_t);
SC_API uint32_t sc_read_range(const void*,uint32_t,uint32_t,uint32_t,uint32_t,void*,uint32_t);
SC_API uint32_t sc_matches(const void*,uint32_t,uint32_t,uint32_t,const void*);
#endif
