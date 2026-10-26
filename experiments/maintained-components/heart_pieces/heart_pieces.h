#ifndef GZ_HEART_PIECES_H
#define GZ_HEART_PIECES_H
typedef unsigned int HpcU;
typedef unsigned char HpcByte;
enum { HPC_REQUEST=948, HPC_INVENTORY_PAGE=8, HPC_CONTAINER_ROW=25 };
/* Called only by the existing guarded inventory-request controller. */
HpcU _gz_hpc_apply(volatile HpcByte *save);
#endif
