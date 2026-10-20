#ifndef GZ_MORTH_SETUP_H
#define GZ_MORTH_SETUP_H
typedef unsigned int MhsU32;
/* Pure packet validation only; no native address or writer capability. */
MhsU32 _gz_mhs_validate(const void *record,MhsU32 bytes);
#endif
