#ifndef GZ_GENERIC_CPU_CAPTURE_DIAGNOSTIC_H
#define GZ_GENERIC_CPU_CAPTURE_DIAGNOSTIC_H

typedef unsigned int GacU;
enum {
    GAC_OK = 0, GAC_CONTEXT = 2100, GAC_STATE = 2101,
    GAC_PROCESS = 2102, GAC_ALLOCATION = 2103, GAC_HEAP = 2104,
    GAC_CAPACITY = 2105, GAC_CHANGED = 2106, GAC_FORMAT = 2107,
    GAC_STORAGE = 2108, GAC_TARGET = 2109,
    GAC_CAPACITY_BYTES = 0x10000, GAC_TARGETS = 2,
    GAC_DIAGNOSTIC_ONLY = 1,
    GAC_MISSING_WORKER_WRITERS = 1, GAC_MISSING_INBOUND = 2,
    GAC_MISSING_SHARED_RESOURCES = 4, GAC_MISSING_AUDIO = 8,
    GAC_MISSING_LIFETIME_INVERSE = 16, GAC_MISSING_ATOMICITY = 32,
    GAC_MISSING_ALL = 63
};

/* No application/restore API exists. The image is raw diagnostic evidence,
 * with unresolved closure bits retained even if two passes compare equal. */
GacU _gz_gac_validate(const void *image, GacU bytes);
GacU _gz_gac_capture(GacU destination, GacU capacity, GacU *written);
GacU _gz_gac_read_under_root(GacU frame);
GacU _gz_gac_kernel_scope(GacU frame);
GacU _gz_gac_kernel_identity(GacU frame); /* scalar foreign-writer identity only */
GacU _gz_gac_storage_scope(void);
GacU _gz_gac_cleanup_safe(void);
void _gz_gac_tick(void);

/* The new explicit root/JPG discriminator must provide these. They cannot
 * be supplied by old KRC flags, caller domain bits, tests or archive fields. */
GacU _gz_gac_root_access(void);
GacU _gz_gac_root_read_scope(GacU frame);
GacU _gz_gac_particle_entry(GacU frame);
GacU _gz_gac_particle_identity(GacU frame, GacU cookie, GacU thread);

extern volatile GacU _gz_gac_request, _gz_gac_ack;
extern volatile GacU _gz_gac_report[32];
#endif
