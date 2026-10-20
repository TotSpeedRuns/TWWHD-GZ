#ifndef GZ_DEATH_OBSERVER_H
#define GZ_DEATH_OBSERVER_H
#include "process.h"
#include "native_lifetime/lifetime.h"
enum {DTO_CAP=256,DTO_MAGIC=0x44544831u,DTO_CONTEXT=620,DTO_PROCESS=621,
 DTO_REQUEST=622,DTO_HEAP=623,DTO_CHANGED=624,DTO_LAYOUT=625,DTO_MISSING=626,DTO_TIMEOUT=627};
enum {DTO_NONE=0,DTO_BEFORE_REQUEST=1,DTO_REQUESTED=2,DTO_CONSTRUCTING=3,
 DTO_READY=4,DTO_FADING=5,DTO_WINDOW=6,DTO_FINISHING=7};
typedef struct {
 u32 pointer,append,layer,phase;
 HlAllocation allocation,append_allocation;
 HlHeap heap,append_heap;
 u8 request_bytes[96],append_bytes[28];
} DtoRequest;
typedef struct {
 u32 magic,bytes,valid,sequence,frame,kind,request_nonce,epoch,stage0,stage1,room,layer;
 u32 process_serial,create_head,delete_head,link_audit;
 PoEntry link;
 /* proc,sentinel,callback,health,fade,GameOverID,demo,event,mode,status,
  * GameOver status,graphics root audit, followed by zero reserved words. */
 u32 values[24];
 PoEntry gameover;
 /* init,timer,delay,state,animeStart, followed by zero reserved words. */
 u32 process_values[8];
 DtoRequest request;
 u32 checksum;
} DtoRecord;
typedef char DtoRequestSize[sizeof(DtoRequest)==428?1:-1];
typedef char DtoRecordSize[sizeof(DtoRecord)==752?1:-1];
extern volatile u32 _gz_dto_request,_gz_dto_ack,_gz_dto_stop;
extern u32 _gz_dto_report[32];
extern DtoRecord _gz_dto_records[DTO_CAP];
u32 _gz_dto_kernel_scope(u32 frame);
u32 _gz_dto_read_under_root(u32 frame);
void _gz_dto_step(void);
#endif
