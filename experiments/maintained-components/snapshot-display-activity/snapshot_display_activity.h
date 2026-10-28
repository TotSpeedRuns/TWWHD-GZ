#ifndef GZ_SNAPSHOT_DISPLAY_ACTIVITY_H
#define GZ_SNAPSHOT_DISPLAY_ACTIVITY_H
/* Activity classification only. These bits do not issue an owner, read lease,
 * renderer token, quiet receipt or native success. The ordinary chooser is a
 * read-only intent producer. Its worker forces menu1 before any replay, and
 * every existing current-context, owner, renderer and participant check stays.
 * Only fixed legacy_allow values 0 / GZ_SNAPSHOT_LEGACY_UPCHARGE are used.
 * Never clear user flags or pass this result as the actual flags to a report.
 */
enum {
 GZ_SNAPSHOT_DISPLAY_DEBUG=1u,
 GZ_SNAPSHOT_DISPLAY_INPUT=2u,
 GZ_SNAPSHOT_DISPLAY_FLAGS=GZ_SNAPSHOT_DISPLAY_DEBUG|GZ_SNAPSHOT_DISPLAY_INPUT,
 GZ_SNAPSHOT_LEGACY_UPCHARGE=8u
};
static inline unsigned int gz_snapshot_activity_conflict(unsigned int actual_flags,
                                                        unsigned int legacy_allow){
 return actual_flags&~(GZ_SNAPSHOT_DISPLAY_FLAGS|legacy_allow);
}
#endif
