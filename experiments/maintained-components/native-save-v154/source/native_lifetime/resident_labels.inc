/* Include only in the future fixed genuine AFTER_RETURN consumer. Invoke
 * outside HG/job/HL holds. These are the ONLY native reads added by it.
 * Values remain opaque integer labels; they are never dereferenced here.
 * Taking each alias address must receive the actual native RPL DATA fixup.
 */
extern volatile unsigned int _gz_ghd_game0_slot;
extern volatile unsigned int _gz_ghd_game1_slot;
extern volatile unsigned int _gz_ghd_zelda_slot;
static GhdLabels ghd_resident_labels(void) {
 GhdLabels labels;
 labels.game0=_gz_ghd_game0_slot;
 labels.game1=_gz_ghd_game1_slot;
 labels.zelda=_gz_ghd_zelda_slot;
 labels.reserved=0;
 return labels;
}
static unsigned int ghd_same_labels(const GhdLabels*a,const GhdLabels*b) {
 return !a->reserved&&!b->reserved&&a->game0==b->game0&&a->game1==b->game1&&a->zelda==b->zelda;
}
