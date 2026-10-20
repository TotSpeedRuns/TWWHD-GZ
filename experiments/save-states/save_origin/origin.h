#ifndef GZ_PARENT_ORIGIN_H
#define GZ_PARENT_ORIGIN_H
#include "parent_backing.h"
typedef SpbU32 SpoU32;
enum { SPO_EMPTY=0,SPO_PENDING=1,SPO_BORN=2,SPO_RETURNED=3,SPO_CONSTRUCTED=4,SPO_INITIALIZING=5,SPO_READY=6,SPO_RETIRED=7 };
enum { SPO_OUTER=1,SPO_PRE=2,SPO_FACTORY=3,SPO_RETURN=4,SPO_CTOR=5,SPO_INITPRE=6,SPO_INITPOST=7,SPO_DTOR=8 };
/* Fixed-entry query of values only. A successful raw SRO/SRW/SPB value API
 * cannot mint this separate private completion receipt. Serialized reviewed
 * fixed-entry origin/current-stack mapping remain premises, never consequences
 * of integers, frame equality or this0 return. No native lifetime permission.
 * out must actually be mapped caller-owned128-byte writable memory. */
SpoU32 _gz_spo_query(SpoU32 root,SpoU32 generation,SpbValue*out);
SpoU32 _gz_spo_native_admit(void);
#endif
