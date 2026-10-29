#ifndef GZ_CURRENT_ITEM_LIFECYCLE_INTENT_H
#define GZ_CURRENT_ITEM_LIFECYCLE_INTENT_H
/* Fixed private participant. No caller chooses a native pointer/callback,
 * source tuple, nonce, result, or permission. */
unsigned int _gz_lrr_requested(void),_gz_lrr_native_step(void);
unsigned int _gz_lrr_blocked(void),_gz_lrr_private_apart(unsigned int,unsigned int);
static void lrr_arm(unsigned int,unsigned int);
#endif
