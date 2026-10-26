/* Favorites preference I/O is isolated from the game/render/input callbacks.
 * Only the game callback mutates the live list. The worker sees a sealed copy
 * and returns another copy; it never follows a route/actor/UI pointer. */
#include "favorites_r562.h"
#include <coreinit/filesystem.h>
#include <coreinit/thread.h>
#include <coreinit/messagequeue.h>
#include <nn/save.h>
typedef FavU U;
typedef int I;
extern I OSDynLoad_Acquire(const char*,U*),OSDynLoad_FindExport(U,I,const char*,void*);
extern U _gz_state[],_gz_pending,_gz_route_active,_gz_life_phase,_gz_game_counter;
extern volatile U _gz_roh_owner[];
static FSClient client __attribute__((aligned(64)));
static FSCmdBlock command __attribute__((aligned(64)));
static U disk[FAV_WORDS] __attribute__((aligned(64)));
static OSThread worker __attribute__((aligned(64)));
static unsigned char stack[16384] __attribute__((aligned(64)));
static OSMessageQueue queue;
static OSMessage messages[1];
typedef struct {U kind,serial,bank;I status,read_status[2];U record[FAV_WORDS] __attribute__((aligned(64)));} Job;
static Job job;
static volatile U job_state; /* 0 idle, 1 worker owns packet, 2 completed */
static U started,client_added,serial,bank=1,job_frame,timed_out,loaded_error;
U _gz_fav_storage_report[24]={0x46565332u,96,2};
#define FP(n) static __typeof__(&n) p_##n
FP(FSInit);FP(FSInitCmdBlock);FP(FSAddClient);FP(FSCloseFile);FP(FSReadFile);FP(FSWriteFile);
FP(SAVEInit);FP(SAVEInitSaveDir);FP(SAVEOpenFile);FP(SAVEFlushQuota);
FP(OSGetCurrentThread);FP(OSGetThreadPriority);FP(OSCreateThread);FP(OSResumeThread);FP(OSInitMessageQueue);FP(OSSendMessage);FP(OSReceiveMessage);
#undef FP
static const char*paths[2]={"gz-favorites-a.dat","gz-favorites-b.dat"};
static void barrier(void){__asm__ volatile("sync":::"memory");}
static U init_io(void){U m=0,s=0;
 if(client_added)return 1;
 _gz_fav_storage_report[4]=1;
 if(OSDynLoad_Acquire("coreinit.rpl",&m)||!m||OSDynLoad_Acquire("nn_save.rpl",&s)||!s)return 0;
#define GET(module,n) if(OSDynLoad_FindExport(module,0,#n,&p_##n)||!p_##n)return 0
 GET(m,FSInit);GET(m,FSAddClient);GET(m,FSInitCmdBlock);GET(m,FSCloseFile);GET(m,FSReadFile);GET(m,FSWriteFile);
 GET(s,SAVEInit);GET(s,SAVEInitSaveDir);GET(s,SAVEOpenFile);GET(s,SAVEFlushQuota);
#undef GET
 _gz_fav_storage_report[4]=2;p_FSInit();
 _gz_fav_storage_report[4]=3;if(p_SAVEInit())return 0;
 _gz_fav_storage_report[4]=4;if(p_SAVEInitSaveDir(255))return 0;
 _gz_fav_storage_report[4]=5;p_FSInitCmdBlock(&command);
 if(p_FSAddClient(&client,FS_ERROR_FLAG_NONE))return 0;
 client_added=1;return 1;
}
static I read_bank(U which){FSFileHandle handle=0;I z,closed;
 _gz_fav_storage_report[4]=10+which*3;
 z=p_SAVEOpenFile(&client,&command,255,paths[which],"r",&handle,FS_ERROR_FLAG_ALL);if(z<0)return z;
 _gz_fav_storage_report[4]=11+which*3;
 z=p_FSReadFile(&client,&command,(unsigned char*)disk,1,sizeof(disk),handle,0,FS_ERROR_FLAG_ALL);
 _gz_fav_storage_report[4]=12+which*3;closed=p_FSCloseFile(&client,&command,handle,FS_ERROR_FLAG_ALL);
 return z==(I)sizeof(disk)&&closed>=0&&_gz_fav_record_valid(disk,sizeof(disk))?0:-100;
}
static void load_banks(void){U b,i,found=0;
 job.serial=0;job.bank=1;
 for(b=0;b<2;b++){
  job.read_status[b]=read_bank(b);
  if(!job.read_status[b]&&(!found||(I)(disk[3]-job.serial)>0)){
   job.serial=disk[3];job.bank=b;for(i=0;i<FAV_WORDS;i++)job.record[i]=disk[i];found=1;
  }
 }
 if(!found)for(i=0;i<FAV_WORDS;i++)job.record[i]=0;
 job.status=!found&&(job.read_status[0]!=-6||job.read_status[1]!=-6)?-100:0;
}
static I write_bank(void){U next=job.bank^1;FSFileHandle handle=0;I z,closed;
 _gz_fav_storage_report[4]=20;
 z=p_SAVEOpenFile(&client,&command,255,paths[next],"w",&handle,FS_ERROR_FLAG_ALL);if(z<0)return z;
 _gz_fav_storage_report[4]=21;
 z=p_FSWriteFile(&client,&command,(unsigned char*)job.record,1,sizeof(job.record),handle,0,FS_ERROR_FLAG_ALL);
 _gz_fav_storage_report[4]=22;closed=p_FSCloseFile(&client,&command,handle,FS_ERROR_FLAG_ALL);
 if(z!=(I)sizeof(job.record)||closed<0)return z<0?z:closed<0?closed:-101;
 _gz_fav_storage_report[4]=23;z=p_SAVEFlushQuota(&client,&command,255,FS_ERROR_FLAG_ALL);if(z<0)return z;
 job.serial=job.record[3];job.bank=next;return 0;
}
static int entry(int argc,const char**argv){OSMessage message;(void)argc;(void)argv;
 for(;;){
  if(!p_OSReceiveMessage(&queue,&message,OS_MESSAGE_FLAGS_BLOCKING))continue;
  barrier();if(job_state!=1)continue;
  if(!init_io())job.status=-102;
  else if(job.kind==1)load_banks();
  else if(job.kind==2)job.status=write_bank();
  else job.status=-103;
  _gz_fav_storage_report[4]=30;_gz_fav_storage_report[5]++;barrier();job_state=2;
 }
 return 0;
}
static U start(void){U m=0;OSThread*parent;I priority;
 if(started)return started==1;
 started=2;
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
 if(job_state||timed_out)return 0;
 job.kind=kind;job.serial=serial;job.bank=bank;job.status=0;job.read_status[0]=job.read_status[1]=0;
 if(kind==2){
  if(!seq)seq=1;
  for(i=0;i<FAV_WORDS;i++)job.record[i]=0;
  job.record[0]=0x46415631u;job.record[1]=sizeof(job.record);job.record[2]=1;job.record[3]=seq;job.record[4]=_gz_fav_count;job.record[5]=FAV_CAP;
  for(i=0;i<_gz_fav_count;i++){job.record[8+i*2]=_gz_fav_keys[i][0];job.record[9+i*2]=_gz_fav_keys[i][1];}
  job.record[7]=_gz_fav_checksum(job.record);
 }
 job_frame=_gz_game_counter;_gz_fav_storage_report[6]++;_gz_fav_storage_report[7]=kind;
 barrier();job_state=1;message.message=&job;
 if(!p_OSSendMessage(&queue,&message,OS_MESSAGE_FLAGS_NONE)){job_state=0;return 0;}
 return 1;
}
static void unavailable(void){loaded_error=1;_gz_fav_report[8]=1;if(!_gz_fav_loaded){_gz_fav_loaded=1;_gz_fav_rebuild();}}
void _gz_fav_io_tick(void){U request,i;
 /* No filesystem call, wait, join or blocking message operation on this path.
  * Even a stuck device leaves the preference worker asleep, not the game. */
 if(!job_state&&!_gz_fav_request&&!_gz_fav_toggle_request)return;
 _gz_fav_storage_report[3]++;_gz_fav_storage_report[8]=job_state;
 if(job_state==1){
  _gz_fav_storage_report[9]=_gz_game_counter-job_frame;
  if(!timed_out&&_gz_game_counter-job_frame>300){timed_out=1;_gz_fav_storage_report[10]=1;unavailable();}
  if(!timed_out)return;
 }
 if(job_state==2){
  barrier();_gz_fav_storage_report[11]=(U)job.status;
  if(!timed_out){
   if(job.kind==1){
    serial=job.serial;bank=job.bank;_gz_fav_report[10]=(U)job.read_status[0];_gz_fav_report[11]=(U)job.read_status[1];
    _gz_fav_count=job.record[0]?job.record[4]:0;
    for(i=0;i<FAV_CAP;i++){_gz_fav_keys[i][0]=job.record[8+i*2];_gz_fav_keys[i][1]=job.record[9+i*2];}
    _gz_fav_loaded=1;_gz_fav_rebuild();if(job.status)unavailable();
   }else if(job.kind==2){
    _gz_fav_report[12]=(U)job.status;
    if(job.status)unavailable();else{serial=job.serial;bank=job.bank;_gz_fav_dirty=0;_gz_fav_report[9]=serial;}
   }
  }
  barrier();job_state=0;
 }
 if(!_gz_state[0]||_gz_pending||_gz_route_active||_gz_life_phase||_gz_roh_owner[3]||_gz_roh_owner[4]||_gz_roh_owner[12])return;
 if(!_gz_fav_loaded){
  if(!start()||!submit(1))unavailable();
  return;
 }
 request=_gz_fav_toggle_request;_gz_fav_toggle_request=0;
 if(request)_gz_fav_toggle(request-1);
 _gz_fav_request=0;_gz_fav_report[7]=1;
 if(_gz_fav_dirty&&!loaded_error&&!timed_out&&(!start()||!submit(2)))unavailable();
}
