#ifndef GZ_FAVORITES_IO_STATE_H
#define GZ_FAVORITES_IO_STATE_H
#include "favorites_r562.h"
enum {FAV_IO_ADOPT=1,FAV_IO_COMMIT=2,FAV_IO_CLEAR_DIRTY=4,FAV_IO_RETRY=8,FAV_IO_MIGRATE=16};
enum {FAV_LOAD_RETRY=0,FAV_LOAD_ACCOUNT=1,FAV_LOAD_MIGRATE=2,FAV_LOAD_EMPTY=3};
/* -6 is exact WUT FS_STATUS_NOT_FOUND; -100 is a complete but invalid record.
 * Any other read failure leaves the bank's possibly newer data unknown. */
static inline FavU fav_io_unknown_read(int status){return status!=0&&status!=-6&&status!=-100;}
static inline FavU fav_io_load_policy(int a,int b,FavU account_found,int c,int d,FavU legacy_found){
 if(fav_io_unknown_read(a)||fav_io_unknown_read(b))return FAV_LOAD_RETRY;
 if(account_found)return FAV_LOAD_ACCOUNT;
 if(fav_io_unknown_read(c)||fav_io_unknown_read(d))return FAV_LOAD_RETRY;
 if(legacy_found)return FAV_LOAD_MIGRATE;
 return a==-6&&b==-6&&c==-6&&d==-6?FAV_LOAD_EMPTY:FAV_LOAD_RETRY;
}
/* Pure policy: a slow warning never invalidates eventual completion. */
static inline FavU fav_io_completion(FavU kind,int status,FavU live_revision,FavU snapshot_revision,FavU migrated){
 if(status)return FAV_IO_RETRY;
 if(kind==1)return FAV_IO_ADOPT|(migrated?FAV_IO_MIGRATE:0);
 if(kind==2)return FAV_IO_COMMIT|(live_revision==snapshot_revision?FAV_IO_CLEAR_DIRTY:0);
 return FAV_IO_RETRY;
}
static inline FavU fav_io_retry_due(FavU frame,FavU last,FavU armed){return !armed||frame-last>=180;}
static inline FavU fav_io_newer(FavU serial,FavU previous,FavU found){return !found||(int)(serial-previous)>0;}
#endif
