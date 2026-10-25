#include "favorites_r562.h"
typedef FavU U;
typedef int I;
extern I OSDynLoad_Acquire(const char*,U*),OSDynLoad_FindExport(U,I,const char*,void*);
extern U _gz_state[],_gz_pending,_gz_route_active,_gz_life_phase;
extern volatile U _gz_roh_owner[];
static unsigned char client[0x1700]__attribute__((aligned(64)));
static unsigned char command[0xa80]__attribute__((aligned(64)));
static U disk[FAV_WORDS]__attribute__((aligned(64)));
static U initialized,client_added,serial,bank=1,io_busy;
static void(*fs_init)(void),(*fs_command)(void*);
static I(*fs_add)(void*,U),(*fs_close)(void*,void*,U,U);
static I(*fs_read)(void*,void*,void*,U,U,U,U,U),(*fs_write)(void*,void*,void*,U,U,U,U,U);
static I(*save_open)(void*,void*,unsigned char,const char*,const char*,U*,U);
static I(*save_flush)(void*,void*,unsigned char,U);
static I(*save_directory)(unsigned char);
static const char*paths[2]={"gz-favorites-a.dat","gz-favorites-b.dat"};
static U init(void){U m=0,s=0;
 if(initialized)return client_added;
 initialized=1;
 if(OSDynLoad_Acquire("coreinit.rpl",&m)||!m||OSDynLoad_Acquire("nn_save.rpl",&s)||!s)return 0;
#define GET(module,name,target) if(OSDynLoad_FindExport(module,0,name,&target)||!target)return 0
 GET(m,"FSInit",fs_init);GET(m,"FSAddClient",fs_add);GET(m,"FSInitCmdBlock",fs_command);
 GET(m,"FSCloseFile",fs_close);GET(m,"FSReadFile",fs_read);GET(m,"FSWriteFile",fs_write);
 GET(s,"SAVEOpenFile",save_open);GET(s,"SAVEFlushQuota",save_flush);
 GET(s,"SAVEInitSaveDir",save_directory);
#undef GET
 /* Slot255 is the title's common preference directory, not an account
  * slot. It may not exist in an otherwise valid practice installation. */
 fs_init();if(save_directory(255))return 0;fs_command(command);
 if(fs_add(client,0))return 0;
 client_added=1;return 1;}
static I read_bank(U which){U handle=0;I z,close;
 z=save_open(client,command,255,paths[which],"r",&handle,0);
 if(z<0)return z;
 z=fs_read(client,command,disk,1,sizeof(disk),handle,0,0);
 close=fs_close(client,command,handle,0);
 if(z!=(I)sizeof(disk)||close<0||!_gz_fav_record_valid(disk,sizeof(disk)))return -100;
 return 0;}
static void load(void){U b,i,found=0;I z;
 for(b=0;b<2;b++){
  z=read_bank(b);_gz_fav_report[10+b]=(U)z;
  if(!z&&(!found||(I)(disk[3]-serial)>0)){
   serial=disk[3];bank=b;_gz_fav_count=disk[4];
   for(i=0;i<FAV_CAP;i++){_gz_fav_keys[i][0]=disk[8+i*2];_gz_fav_keys[i][1]=disk[9+i*2];}
   found=1;
  }
 }
 if(!found&&((I)_gz_fav_report[10]!=-6||(I)_gz_fav_report[11]!=-6))_gz_fav_report[8]=1;
 _gz_fav_loaded=1;_gz_fav_rebuild();
}
static I write_bank(void){U i,next=bank^1,handle=0,seq=serial+1;I z,closed;
 if(!seq)seq=1;
 for(i=0;i<FAV_WORDS;i++)disk[i]=0;
 disk[0]=0x46415631u;disk[1]=sizeof(disk);disk[2]=1;disk[3]=seq;disk[4]=_gz_fav_count;disk[5]=FAV_CAP;
 for(i=0;i<_gz_fav_count;i++){disk[8+i*2]=_gz_fav_keys[i][0];disk[9+i*2]=_gz_fav_keys[i][1];}
 disk[7]=_gz_fav_checksum(disk);
 z=save_open(client,command,255,paths[next],"w",&handle,0);if(z<0)return z;
 z=fs_write(client,command,disk,1,sizeof(disk),handle,0,0);
 closed=fs_close(client,command,handle,0);
 if(z!=(I)sizeof(disk)||closed<0)return z<0?z:closed<0?closed:-101;
 z=save_flush(client,command,255,0);if(z<0)return z;
 serial=seq;bank=next;_gz_fav_dirty=0;_gz_fav_report[9]=serial;return 0;}
void _gz_fav_io_tick(void){U request;I z;
 /* I/O runs once per user request in the normal game callback, never in
  * VPAD, drawing, a restore transaction, a room load or ordinary play. */
 if(!_gz_fav_request||!_gz_state[0]||_gz_pending||_gz_route_active||_gz_life_phase||
    _gz_roh_owner[3]||_gz_roh_owner[4]||_gz_roh_owner[12]||io_busy)return;
 io_busy=1;_gz_fav_request=0;
 if(!init()){
  _gz_fav_report[8]=1;
  if(!_gz_fav_loaded){_gz_fav_loaded=1;_gz_fav_rebuild();}
 }else if(!_gz_fav_loaded)load();
 request=_gz_fav_toggle_request;_gz_fav_toggle_request=0;
 if(request)_gz_fav_toggle(request-1);
 if(client_added&&_gz_fav_dirty&&!_gz_fav_report[8]){
  z=write_bank();_gz_fav_report[12]=(U)z;
  if(z)_gz_fav_report[8]=1;
 }
 _gz_fav_report[7]=_gz_fav_loaded;io_busy=0;
}
