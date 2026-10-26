#ifndef GZ_FAVORITES_H
#define GZ_FAVORITES_H
typedef unsigned int FavU;
enum {FAV_CAP=803,FAV_WORDS=8+FAV_CAP*2,FAV_PAGE=52,FAV_ROOT_INDEX=5,FAV_ROOT_Y=8};
extern FavU _gz_fav_report[16];
extern FavU _gz_fav_keys[FAV_CAP][2],_gz_fav_count,_gz_fav_loaded;
extern FavU _gz_fav_dirty,_gz_fav_request,_gz_fav_toggle_request;
extern FavU _gz_fav_revision;
FavU _gz_fav_record_valid(const FavU *p,FavU bytes);
FavU _gz_fav_checksum(const FavU *p);
FavU _gz_fav_toggle(FavU route);
void _gz_fav_rebuild(void);
void _gz_fav_io_tick(void);
#endif
