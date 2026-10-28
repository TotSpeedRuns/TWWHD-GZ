#ifndef GZ_SOURCE_WORK_DIAGNOSTIC_H
#define GZ_SOURCE_WORK_DIAGNOSTIC_H
/* Report-only callsite counters. Extent is the original call argument, not a
 * promise of bytes actually read. No counter result grants any permission. */
enum {
    SVD_ROH_ARCHIVE_FNV = 0,
    SVD_WSP_START_FNV = 1,
    SVD_WSP_READ_FNV = 2,
    SVD_WCT_FORWARD_FNV = 3,
    SVD_WCT_TARGET_FNV = 4,
    SVD_SO_ACQUIRE_FNV = 5,
    SVD_SAMPLE_PF_FORMAT = 6,
    SVD_SAMPLE_BC_FORMAT = 7,
    SVD_SAMPLE_LG_FORMAT = 8,
    SVD_SAMPLE_B294_FORMAT = 9,
    SVD_SRD_SELECTED_FORMAT = 10,
    SVD_ROWS = 11
};
void _gz_svd_note(unsigned int category, unsigned int extent);
#endif
