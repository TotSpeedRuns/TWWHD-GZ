#ifndef GZ_SOURCE_PIN_DIGEST_H
#define GZ_SOURCE_PIN_DIGEST_H
/* Upper32: FULL0 / actual issued digest1 / refusal2. Lower32: real digest.
 * Hash zero remains a valid digest. This is private saved SOURCE metadata,
 * never a native object/read/world or live CURRENT comparison capability. */
typedef unsigned long long GzSourceDigest;
enum { GZ_DIGEST_FULL=0,GZ_DIGEST_ISSUED=1,GZ_DIGEST_REFUSE=2 };
GzSourceDigest _gz_osa_verified_source_digest(unsigned int pin,unsigned int source,
    unsigned int used,unsigned int identity,unsigned int generation,unsigned int revision);
GzSourceDigest _gz_som_verified_source_digest(unsigned int source,unsigned int used);
unsigned int _gz_wct_source_digest_scope(void);
unsigned int _gz_osa_verify_source_final(unsigned int pin,unsigned int source,
    unsigned int used,unsigned int identity,unsigned int generation,unsigned int revision);
#endif
