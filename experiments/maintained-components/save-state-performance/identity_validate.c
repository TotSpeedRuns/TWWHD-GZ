/* Pure IDG1 format validation. No native state, cache, allocation or lock.
 * The exported assembly shim retains the original r6..r31 contract. */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

extern u32 _gz_identity_edges_safe(const void *graph);

#define IDG_MAGIC 0x49444731u
#define IDG_BAD 28u
#define IDG_MAX 512u

static u32 row_header_invalid(const u8 *row)
{
    return *(const u16 *)(row + 4) > 483u ||
           *(const u32 *)row == 0xffffffffu;
}

static u32 row_floats_invalid(const u8 *row)
{
    const u32 *words = (const u32 *)row;
    return (words[4] & 0x7f800000u) == 0x7f800000u ||
           (words[5] & 0x7f800000u) == 0x7f800000u ||
           (words[6] & 0x7f800000u) == 0x7f800000u;
}

/* Keep an exact sorted set of earlier PIDs. Binary search detects a duplicate
 * as soon as its row is visited; descending insertion has at most n(n-1)/2
 * private word moves. The 512-row cap bounds both search and movement. There
 * is no hash function, collision bucket, native read, or persistent cache. */
static __attribute__((noinline)) u32 large_rows(const u8 *rows, u32 count)
{
    u32 ids[IDG_MAX];
    u32 i;
    for (i = 0; i < count; ++i) {
        const u8 *row = rows + i * 40u;
        u32 value, low = 0, high = i, j;
        if (row_header_invalid(row))
            return IDG_BAD;
        value = *(const u32 *)row;
        while (low < high) {
            u32 middle = low + (high - low) / 2u;
            if (ids[middle] == value)
                return IDG_BAD;
            if (ids[middle] < value)
                low = middle + 1u;
            else
                high = middle;
        }
        /* Match the original row order: header/PID, duplicate, then floats. */
        if (row_floats_invalid(row))
            return IDG_BAD;
        for (j = i; j > low; --j)
            ids[j] = ids[j - 1u];
        ids[low] = value;
    }
    return 0;
}

u32 _gz_identity_record_validate_c(const void *graph)
{
    const u32 *header = (const u32 *)graph;
    const u8 *rows = (const u8 *)graph + 32;
    u32 count, version, i, j;
    if (header[0] == 0)
        return 0;
    if (header[0] != IDG_MAGIC)
        return IDG_BAD;
    count = header[1];
    version = header[2];
    if (count > IDG_MAX || (version != 1u && version != 2u) ||
        (version == 1u ? header[3] != 0 : header[3] > 208u) ||
        header[6] > 255u || header[7] > 255u)
        return IDG_BAD;
    if (count <= 8u) {
        for (i = 0; i < count; ++i) {
            const u8 *row = rows + i * 40u;
            if (row_header_invalid(row))
                return IDG_BAD;
            for (j = 0; j < i; ++j)
                if (*(const u32 *)row == *(const u32 *)(rows + j * 40u))
                    return IDG_BAD;
            if (row_floats_invalid(row))
                return IDG_BAD;
        }
    } else if (large_rows(rows, count)) {
        return IDG_BAD;
    }
    /* The untouched original edge validator remains authoritative. */
    return _gz_identity_edges_safe(graph) ? IDG_BAD : 0;
}
