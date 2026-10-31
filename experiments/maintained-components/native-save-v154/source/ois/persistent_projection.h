#ifndef GZ_PERSISTENT_PROJECTION_H
#define GZ_PERSISTENT_PROJECTION_H

typedef unsigned int PpU32;
typedef unsigned char PpU8;

#define PP_ROW_TUPLES 3825u
#define PP_ROW_BYTES 45924u
#define PP_SAVE_BYTES 3787u
#define PP_OUTPUT_BYTES 3794u
#define PP_STAGE_BYTES 34u
#define PP_BANKS 16u
#define PP_NUMERIC_LOW 0x10000000u
#define PP_NUMERIC_HIGH 0xB0000000u

enum PpStatus {
    PP_OK = 0,
    PP_SOURCE = 70,
    PP_SIZE = 71,
    PP_OUTPUT = 72,
    PP_ALIAS = 73,
    PP_HEADER = 74,
    PP_SAVE = 75,
    PP_EQUIP = 76,
    PP_CLOCK = 77,
    PP_STAGE = 78,
    PP_COMMIT = 79
};

typedef struct {
    PpU32 offsets[PP_SAVE_BYTES];
    PpU32 bank_output_index[PP_BANKS];
} PpFixedTable;

extern const PpFixedTable _gz_pp_fixed;

/* Pure current-format schema18/component1 parser. source must be4-aligned,
 * readable and immutable for the complete call. Numeric checks do not establish
 * mapping or ownership. Table storage must retain its linked immutable bytes.
 * Header pose is ignored; count3825, angle<=65535 and usesPosition1 are required.
 * Clock and xyz must be finite IEEE754 binary32 representations; all finite
 * bits (including signed zero/subnormals) are retained without FP instructions.
 * There is no invented clock gameplay-range test. Pose is never projected.
 */
PpU32 _gz_pp_validate(PpU32 source, PpU32 bytes);

/* Before the first output store, validates the entire row, source/output/table
 * spans and aliases. output must be caller-owned mapped writable storage with
 * exactly3794 bytes; byte alignment is sufficient. It must not alias source or
 * the complete fixed table. Output save bytes follow offsets[] order, then
 *3equips and4big-endian clock bytes. Final34 stage writes override their earlier
 * save bytes. Caller must exclude concurrent source/table/output mutation.
 * No pose, native destinations, item setters, acquisitions ORs or authority.
 * Every refusal leaves output untouched under those prerequisites.
 */
PpU32 _gz_pp_project(PpU32 source, PpU32 bytes,
                     PpU32 output, PpU32 output_bytes);

/* Always28. Pure projection never admits native or world restoration. */
PpU32 _gz_pp_native_admit(void);

#endif
