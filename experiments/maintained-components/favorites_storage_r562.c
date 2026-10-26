/* Preference I/O stays on its real native worker. Only game callbacks mutate
 * the live list; one immutable job packet belongs to the worker until done. */
#include "favorites_r562.h"
#include "favorites_io_state.h"
#include <coreinit/filesystem.h>
#include <coreinit/thread.h>
#include <coreinit/messagequeue.h>
#include <nn/save.h>
typedef FavU U;typedef int I;
extern I OSDynLoad_Acquire(const char*,U*),OSDynLoad_FindExport(U,I,const char*,void*);
extern U _gz_pending,_gz_route_active,_gz_life_phase,_gz_game_counter;
extern volatile U _gz_roh_owner[];
static FSClient client __attribute__((aligned(64)));
static FSCmdBlock command __attribute__((aligned(64)));
static U disk[FAV_WORDS] __attribute__((aligned(64)));
static OSThread worker __attribute__((aligned(64)));
static unsigned char stack[16384] __attribute__((aligned(64)));
static OSMessageQueue queue;static OSMessage messages[1];
typedef struct {U kind,serial,bank,revision,migrated,slot,identity;I status,read_status[2],legacy_status[2];U record[FAV_WORDS] __attribute__((aligned(64)));} Job;
static Job job;static volatile U job_state;
static U started,client_added,serial,bank=1,job_frame,slow,retry_armed,retry_frame;
static uint8_t account_slot;static uint32_t account_identity;
U _gz_fav_storage_report[24]={0x46565332u,96,3};
#define FP(n) static __typeof__(&n) p_##n
FP(FSInit);FP(FSInitCmdBlock);FP(FSAddClient);FP(FSCloseFile);FP(FSReadFile);FP(FSWriteFile);
FP(SAVEInit);FP(SAVEInitSaveDir);FP(SAVEOpenFile);FP(SAVEFlushQuota);
FP(OSGetCurrentThread);FP(OSGetThreadPriority);FP(OSCreateThread);FP(OSResumeThread);FP(OSInitMessageQueue);FP(OSSendMessage);FP(OSReceiveMessage);
#undef FP
/* Exact WUT nn/act/client_cpp.h ABI: SlotNo uint8_t, PersistentId uint32_t,
 * IsSlotOccupied BOOL. Resolve real C++ exports, never guess an account. */
static uint8_t(*p_GetSlotNo)(void);
static uint32_t(*p_GetPersistentId)(void);
static BOOL(*p_IsSlotOccupied)(uint8_t);
static const char*paths[2]={"gz-favorites-a.dat","gz-favorites-b.dat"};
static void barrier(void){__asm__ volatile("sync":::"memory");}
static U current_account(void){uint8_t slot=p_GetSlotNo();uint32_t identity=p_GetPersistentId();
 return slot!=255&&identity&&p_IsSlotOccupied(slot)&&slot==account_slot&&identity==account_identity;
}
static U init_io(void){U m=0,s=0,a=0;I z;
 if(client_added)return current_account();
 _gz_fav_storage_report[4]=1;
 if(OSDynLoad_Acquire("coreinit.rpl",&m)||!m||OSDynLoad_Acquire("nn_save.rpl",&s)||!s||OSDynLoad_Acquire("nn_act.rpl",&a)||!a)return 0;
#define GET(module,n) if(OSDynLoad_FindExport(module,0,#n,&p_##n)||!p_##n)return 0
 GET(m,FSInit);GET(m,FSAddClient);GET(m,FSInitCmdBlock);GET(m,FSCloseFile);GET(m,FSReadFile);GET(m,FSWriteFile);
 GET(s,SAVEInit);GET(s,SAVEInitSaveDir);GET(s,SAVEOpenFile);GET(s,SAVEFlushQuota);
#undef GET
 if(OSDynLoad_FindExport(a,0,"GetSlotNo__Q2_2nn3actFv",&p_GetSlotNo)||!p_GetSlotNo||
  OSDynLoad_FindExport(a,0,"GetPersistentId__Q2_2nn3actFv",&p_GetPersistentId)||!p_GetPersistentId||
  OSDynLoad_FindExport(a,0,"IsSlotOccupied__Q2_2nn3actFUc",&p_IsSlotOccupied)||!p_IsSlotOccupied)return 0;
 account_slot=p_GetSlotNo();account_identity=p_GetPersistentId();if(!current_account())return 0;
 _gz_fav_storage_report[14]=account_slot;_gz_fav_storage_report[15]=account_identity;
 _gz_fav_storage_report[4]=2;p_FSInit();
 _gz_fav_storage_report[4]=3;z=p_SAVEInit();_gz_fav_storage_report[16]=(U)z;if(z)return 0;
 if(!current_account())return 0;
 /* Practice title allocates account quota and zero common quota. */
 _gz_fav_storage_report[4]=4;z=p_SAVEInitSaveDir(account_slot);_gz_fav_storage_report[16]=(U)z;if(z)return 0;
 _gz_fav_storage_report[4]=5;p_FSInitCmdBlock(&command);
 z=p_FSAddClient(&client,FS_ERROR_FLAG_NONE);_gz_fav_storage_report[16]=(U)z;if(z)return 0;
 client_added=1;return current_account();
}
static I read_bank(uint8_t slot,U which){FSFileHandle handle=0;I z,closed;
 if(!current_account())return -104;
 _gz_fav_storage_report[4]=(slot==255?40:10)+which*3;
 z=p_SAVEOpenFile(&client,&command,slot,paths[which],"r",&handle,FS_ERROR_FLAG_ALL);if(z<0)return z;
 _gz_fav_storage_report[4]=(slot==255?41:11)+which*3;
 z=p_FSReadFile(&client,&command,(unsigned char*)disk,1,sizeof(disk),handle,0,FS_ERROR_FLAG_ALL);
 closed=p_FSCloseFile(&client,&command,handle,FS_ERROR_FLAG_ALL);
 if(!current_account())return -104;
 if(z<0)return z;if(closed<0)return closed;
 return z==(I)sizeof(disk)&&_gz_fav_record_valid(disk,sizeof(disk))?0:-100;
}
static void accept_record(U which,U*found){U i;
 if(fav_io_newer(disk[3],job.serial,*found)){
  job.serial=disk[3];job.bank=which;for(i=0;i<FAV_WORDS;i++)job.record[i]=disk[i];*found=1;
 }
}
typedef char FavMissingMatchesSDK[FS_STATUS_NOT_FOUND==-6?1:-1];
static void load_banks(void){U b,i,found=0,choice;
 job.serial=0;job.bank=1;job.migrated=0;for(i=0;i<FAV_WORDS;i++)job.record[i]=0;
 for(b=0;b<2;b++){job.read_status[b]=read_bank(account_slot,b);if(!job.read_status[b])accept_record(b,&found);}
 /* Unreadable account storage is never an authoritative empty list. */
 if(fav_io_unknown_read(job.read_status[0])||fav_io_unknown_read(job.read_status[1])){job.status=-105;return;}
 if(found){job.status=0;return;}
 for(b=0;b<2;b++){
  /* Legacy common banks are read-only: no common init/write/flush. */
  job.legacy_status[b]=read_bank(255,b);if(!job.legacy_status[b])accept_record(b,&found);
 }
 choice=fav_io_load_policy(job.read_status[0],job.read_status[1],0,job.legacy_status[0],job.legacy_status[1],found);
 if(choice==FAV_LOAD_MIGRATE){job.bank=1;job.migrated=1;job.status=0;return;}
 job.status=choice==FAV_LOAD_EMPTY?0:-106;
}
static I write_bank(void){U next=job.bank^1,i;FSFileHandle handle=0;I z,closed;
 if(!current_account()||!_gz_fav_record_valid(job.record,sizeof(job.record)))return -104;
 _gz_fav_storage_report[4]=20;
 z=p_SAVEOpenFile(&client,&command,account_slot,paths[next],"w",&handle,FS_ERROR_FLAG_ALL);if(z<0)return z;
 if(!current_account()){p_FSCloseFile(&client,&command,handle,FS_ERROR_FLAG_ALL);return -104;}
 _gz_fav_storage_report[4]=21;
 z=p_FSWriteFile(&client,&command,(unsigned char*)job.record,1,sizeof(job.record),handle,0,FS_ERROR_FLAG_ALL);
 _gz_fav_storage_report[4]=22;closed=p_FSCloseFile(&client,&command,handle,FS_ERROR_FLAG_ALL);
 if(z!=(I)sizeof(job.record)||closed<0)return z<0?z:closed<0?closed:-101;
 if(!current_account())return -104;
 _gz_fav_storage_report[4]=23;z=p_SAVEFlushQuota(&client,&command,account_slot,FS_ERROR_FLAG_ALL);if(z<0)return z;
 z=read_bank(account_slot,next);if(z)return z;
 for(i=0;i<FAV_WORDS;i++)if(disk[i]!=job.record[i])return -107;
 if(!current_account())return -104;
 job.serial=job.record[3];job.bank=next;return 0;
}
static int entry(int argc,const char**argv){OSMessage message;(void)argc;(void)argv;
 for(;;){
  if(!p_OSReceiveMessage(&queue,&message,OS_MESSAGE_FLAGS_BLOCKING))continue;
  barrier();if(job_state!=1)continue;
  if(!init_io())job.status=-102;
  else{
   job.slot=account_slot;job.identity=account_identity;
   if(job.kind==1)load_banks();else if(job.kind==2)job.status=write_bank();else job.status=-103;
   if(!job.status&&(!current_account()||job.slot!=account_slot||job.identity!=account_identity))job.status=-104;
  }
  _gz_fav_storage_report[4]=30;_gz_fav_storage_report[5]++;barrier();job_state=2;
 }
 return 0;
}
static U start(void){U m=0;OSThread*parent;I priority;
 if(started)return 1;
 if(OSDynLoad_Acquire("coreinit.rpl",&m)||!m)return 0;
#define GET(n) if(OSDynLoad_FindExport(m,0,#n,&p_##n)||!p_##n)return 0
 GET(OSGetCurrentThread);GET(OSGetThreadPriority);GET(OSCreateThread);GET(OSResumeThread);GET(OSInitMessageQueue);GET(OSSendMessage);GET(OSReceiveMessage);
#undef GET
 parent=p_OSGetCurrentThread();if(!parent)return 0;priority=p_OSGetThreadPriority(parent);if(priority<0||priority>31)return 0;
 _gz_fav_storage_report[12]=(U)priority;priority=priority?priority-1:0;_gz_fav_storage_report[13]=(U)priority;
 p_OSInitMessageQueue(&queue,messages,1);
 if(!p_OSCreateThread(&worker,entry,0,0,stack+sizeof(stack),sizeof(stack),priority,OS_THREAD_ATTRIB_AFFINITY_ANY|OS_THREAD_ATTRIB_DETACHED))return 0;
 started=1;p_OSResumeThread(&worker);return 1;
}
static U submit(U kind){OSMessage message={0};U i,seq=serial+1;
 if(job_state)return 0;
 job.kind=kind;job.serial=serial;job.bank=bank;job.status=0;job.migrated=0;job.revision=_gz_fav_revision;
 job.slot=job.identity=0;
 job.read_status[0]=job.read_status[1]=job.legacy_status[0]=job.legacy_status[1]=0;
 if(kind==2){
  if(_gz_fav_count>FAV_CAP)return 0;if(!seq)seq=1;
  for(i=0;i<FAV_WORDS;i++)job.record[i]=0;
  job.record[0]=0x46415631u;job.record[1]=sizeof(job.record);job.record[2]=1;job.record[3]=seq;job.record[4]=_gz_fav_count;job.record[5]=FAV_CAP;
  for(i=0;i<_gz_fav_count;i++){job.record[8+i*2]=_gz_fav_keys[i][0];job.record[9+i*2]=_gz_fav_keys[i][1];}
  job.record[7]=_gz_fav_checksum(job.record);
 }
 job_frame=_gz_game_counter;slow=0;_gz_fav_report[13]=0;_gz_fav_storage_report[6]++;_gz_fav_storage_report[7]=kind;
 barrier();job_state=1;message.message=&job;
 if(!p_OSSendMessage(&queue,&message,OS_MESSAGE_FLAGS_NONE)){job_state=0;return 0;}
 return 1;
}
static void retry(I status){retry_armed=1;retry_frame=_gz_game_counter;_gz_fav_storage_report[22]++;
 _gz_fav_storage_report[11]=(U)status;_gz_fav_report[8]=1;_gz_fav_request=1;
}
static U quiet(void){return !_gz_pending&&!_gz_route_active&&!_gz_life_phase&&!_gz_roh_owner[3]&&!_gz_roh_owner[4]&&!_gz_roh_owner[12];}
void _gz_fav_io_tick(void){U request,i,decision;
 if(!job_state&&!_gz_fav_request&&!_gz_fav_toggle_request&&!_gz_fav_dirty)return;
 _gz_fav_storage_report[3]++;_gz_fav_storage_report[8]=job_state;
 if(job_state==1){
  _gz_fav_storage_report[9]=_gz_game_counter-job_frame;
  if(!slow&&_gz_game_counter-job_frame>300){slow=1;_gz_fav_storage_report[10]++;_gz_fav_report[13]=1;}
  /* Keep packet ownership; consume actual completion even after a warning. */
  return;
 }
 if(job_state==2){
  /* Fresh account metadata only after the existing quiet callback guards;
   * the cached job pin is consistency evidence, never a current API result. */
  if(!quiet())return;
  barrier();_gz_fav_storage_report[11]=(U)job.status;
  if(job.kind==1){_gz_fav_report[10]=(U)job.read_status[0];_gz_fav_report[11]=(U)job.read_status[1];
   _gz_fav_storage_report[17]=(U)job.legacy_status[0];_gz_fav_storage_report[18]=(U)job.legacy_status[1];}
  if(!job.status&&(!job.identity||job.slot==255||job.slot!=account_slot||job.identity!=account_identity||!current_account()))job.status=-104;
  decision=fav_io_completion(job.kind,job.status,_gz_fav_revision,job.revision,job.migrated);
  if(decision&FAV_IO_ADOPT){
   serial=job.serial;bank=job.bank;_gz_fav_report[10]=(U)job.read_status[0];_gz_fav_report[11]=(U)job.read_status[1];
   _gz_fav_storage_report[17]=(U)job.legacy_status[0];_gz_fav_storage_report[18]=(U)job.legacy_status[1];_gz_fav_storage_report[19]=job.migrated;
   _gz_fav_count=job.record[0]?job.record[4]:0;
   for(i=0;i<FAV_CAP;i++){_gz_fav_keys[i][0]=job.record[8+i*2];_gz_fav_keys[i][1]=job.record[9+i*2];}
   if(!++_gz_fav_revision)_gz_fav_revision=1;
   _gz_fav_loaded=1;_gz_fav_dirty=(decision&FAV_IO_MIGRATE)!=0;_gz_fav_rebuild();_gz_fav_report[9]=serial;
  }
  if(decision&FAV_IO_COMMIT){serial=job.serial;bank=job.bank;_gz_fav_report[12]=0;_gz_fav_report[9]=serial;
   if(decision&FAV_IO_CLEAR_DIRTY)_gz_fav_dirty=0;
   _gz_fav_storage_report[20]=serial;_gz_fav_storage_report[21]=job.revision;
  }
  if(decision&FAV_IO_RETRY)retry(job.status?job.status:-103);else{retry_armed=0;_gz_fav_report[8]=0;}
  slow=0;_gz_fav_report[13]=0;barrier();job_state=0;
 }
 /* No filesystem API here. Menu close must not strand a request. */
 if(!quiet())return;
 if(!_gz_fav_loaded){
  if(fav_io_retry_due(_gz_game_counter,retry_frame,retry_armed)&&(!start()||!submit(1)))retry(-108);
  return;
 }
 request=_gz_fav_toggle_request;_gz_fav_toggle_request=0;if(request)_gz_fav_toggle(request-1);
 _gz_fav_request=0;_gz_fav_report[7]=1;
 if(_gz_fav_dirty&&fav_io_retry_due(_gz_game_counter,retry_frame,retry_armed)&&(!start()||!submit(2)))retry(-108);
}
