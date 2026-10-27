#include "favorites_r562.h"
typedef FavU U;
typedef struct {const char*name;U first,count,reserved;} Folder;
typedef struct {const char*name;U stage[2],entrance,data;} Route;
extern const Folder _gz_route_folders[7],_gz_area_records[];
extern const Route _gz_route_records[FAV_CAP];
extern U _gz_state[],_gz_page,_gz_list_offset,_gz_list_count;
extern U _gz_route_selection,_gz_area_selection,_gz_route_choice,_gz_pending;
extern U _gz_bind_masks[];
extern void _gz_game_text(U,U,U,const char*);
U _gz_fav_report[16]={0x46415631u,64,1};
U _gz_fav_keys[FAV_CAP][2],_gz_fav_count,_gz_fav_loaded;
U _gz_fav_dirty,_gz_fav_request,_gz_fav_toggle_request,_gz_fav_revision;
enum {FAV_AREA_CAP=256};
typedef struct {const char*name;U first,count;} FavGroup;
static U indices[FAV_CAP],visible,top;
static FavGroup groups[FAV_AREA_CAP];
static U group_count,active=FAV_AREA_CAP,root_row,root_top;
static U catalogue[FAV_CAP][2],catalogue_ready;
static char lines[10][64],context[2][96];
static void text(U x,U y,const char*s){_gz_game_text(0,x,y,s);}
static void copy(char*d,const char*s,U cap){U i=0;
 while(i+1<cap&&s[i]){d[i]=s[i];i++;}d[i]=0;
 if(s[i]&&cap>4){d[cap-4]='.';d[cap-3]='.';d[cap-2]='.';d[cap-1]=0;}}
static U location(U route,U*category,U*area){U c,a;
 if(route>=FAV_CAP)return 0;
 for(c=0;c<7;c++){
  const Folder*f=_gz_route_folders+c;
  if(f->first>256||f->count>64||f->first+f->count>256)return 0;
  for(a=0;a<f->count;a++){
   const Folder*g=_gz_area_records+f->first+a;
   if(g->first>FAV_CAP||g->count>FAV_CAP-g->first)return 0;
   if(route>=g->first&&route-g->first<g->count){*category=c;*area=a;return 1;}
  }
 }return 0;}
static void hash_byte(U*k,U b){k[0]=(k[0]^b)*16777619u;k[1]=(k[1]^b)*0x010001a7u;}
static void hash_string(U*k,const char*s){U n=0;while(n<160&&s[n])hash_byte(k,(unsigned char)s[n++]);hash_byte(k,255);}
static U key(U route,U*k){U c,a,i;const Route*r;
 if(!location(route,&c,&a))return 0;
 k[0]=2166136261u;k[1]=0x9e3779b9u;
 hash_string(k,_gz_route_folders[c].name);
 hash_string(k,_gz_area_records[_gz_route_folders[c].first+a].name);
 r=_gz_route_records+route;hash_string(k,r->name);
 for(i=0;i<8;i++)hash_byte(k,((const unsigned char*)r->stage)[i]);
 for(i=0;i<4;i++)hash_byte(k,(r->entrance>>(24-i*8))&255);
 return 1;}
static U find(const U*k){U i;for(i=0;i<_gz_fav_count;i++)if(_gz_fav_keys[i][0]==k[0]&&_gz_fav_keys[i][1]==k[1])return i;return FAV_CAP;}
U _gz_fav_checksum(const U*p){U i,j,h=2166136261u;
 for(i=0;i<FAV_WORDS;i++)if(i!=7)for(j=0;j<4;j++)h=(h^((p[i]>>(24-8*j))&255))*16777619u;
 return h;}
U _gz_fav_record_valid(const U*p,U bytes){U i,j;
 if(bytes!=FAV_WORDS*4||p[0]!=0x46415631u||p[1]!=FAV_WORDS*4||p[2]!=1||!p[3]||p[4]>FAV_CAP||p[6]||p[7]!=_gz_fav_checksum(p))return 0;
 for(i=0;i<p[4];i++)for(j=0;j<i;j++)if(p[8+i*2]==p[8+j*2]&&p[9+i*2]==p[9+j*2])return 0;
 for(i=8+p[4]*2;i<FAV_WORDS;i++)if(p[i])return 0;
 return 1;}
static U same_name(const char*a,const char*b){U i;
 for(i=0;i<160;i++){if(a[i]!=b[i])return 0;if(!a[i])return 1;}return 0;}
static U rows(void){return active<group_count?groups[active].count:group_count;}
static void clamp_page(void){U n=rows();
 if(_gz_state[2]>=n)_gz_state[2]=n?n-1:0;
 if(top>=n)top=0;
 if(_gz_state[2]<top)top=_gz_state[2];
 else if(_gz_state[2]>=top+10)top=_gz_state[2]-9;
}
void _gz_fav_rebuild(void){U i,j,k[2],c,a,g,pos,h;const char*prior=active<group_count?groups[active].name:0;
 visible=0;group_count=0;
 /* The existing category/area/name/stage/entrance keys remain byte-exact.
  * Folders are a derived view of actual route-area metadata, never file IDs.
  * Unknown saved keys remain stored and cannot redirect to a numeric route. */
 if(!catalogue_ready){for(j=0;j<FAV_CAP;j++){if(!key(j,k)){k[0]=k[1]=0;}catalogue[j][0]=k[0];catalogue[j][1]=k[1];}catalogue_ready=1;}
 for(i=0;i<_gz_fav_count&&i<FAV_CAP;i++)for(j=0;j<FAV_CAP;j++){
  const char*name;
  if(catalogue[j][0]!=_gz_fav_keys[i][0]||catalogue[j][1]!=_gz_fav_keys[i][1])continue;
  if(!location(j,&c,&a))continue;
  name=_gz_area_records[_gz_route_folders[c].first+a].name;
  for(g=0;g<group_count;g++)if(same_name(groups[g].name,name))break;
  if(g==group_count){if(group_count==FAV_AREA_CAP)break;groups[g].name=name;groups[g].first=visible;groups[g].count=0;group_count++;}
  /* Stable in-place buckets avoid another803-entry derived list. */
  pos=groups[g].first+groups[g].count;
  for(h=visible;h>pos;h--)indices[h]=indices[h-1];
  indices[pos]=j;visible++;groups[g].count++;
  for(h=g+1;h<group_count;h++)groups[h].first++;
  break;
 }
 active=FAV_AREA_CAP;
 if(prior)for(g=0;g<group_count;g++)if(same_name(prior,groups[g].name)){active=g;break;}
 if(_gz_page==FAV_PAGE){
  if(prior&&active==FAV_AREA_CAP){_gz_state[2]=root_row;top=root_top;}
  clamp_page();
 }
 _gz_fav_report[3]=_gz_fav_count;_gz_fav_report[4]=visible;
 _gz_fav_report[5]=_gz_fav_count-visible;_gz_fav_report[14]=group_count;_gz_fav_report[15]=active<group_count;
}
U _gz_fav_toggle(U route){U i,k[2];
 if(!_gz_fav_loaded||!key(route,k)||_gz_fav_count>FAV_CAP)return 0;
 i=find(k);
 if(i<FAV_CAP){for(;i+1<_gz_fav_count;i++){_gz_fav_keys[i][0]=_gz_fav_keys[i+1][0];_gz_fav_keys[i][1]=_gz_fav_keys[i+1][1];}
  --_gz_fav_count;_gz_fav_keys[_gz_fav_count][0]=_gz_fav_keys[_gz_fav_count][1]=0;_gz_fav_report[6]=2;
 }else{
  if(_gz_fav_count==FAV_CAP)return 0;
  _gz_fav_keys[_gz_fav_count][0]=k[0];_gz_fav_keys[_gz_fav_count++][1]=k[1];_gz_fav_report[6]=1;
 }
 if(!++_gz_fav_revision)_gz_fav_revision=1;
 _gz_fav_dirty=1;_gz_fav_request=1;_gz_fav_rebuild();return 1;
}
static U match(U edges,U held,U mask){return mask&&(edges&mask)&&(held&mask)==mask;}
static void request_toggle(U index){
 if(!_gz_fav_toggle_request){_gz_fav_toggle_request=index+1;_gz_fav_request=1;}
}
void _gz_fav_open(void){_gz_page=FAV_PAGE;_gz_state[2]=0;top=0;active=FAV_AREA_CAP;root_row=root_top=0;_gz_fav_report[15]=0;_gz_fav_request=1;}
U _gz_fav_input(U edges,U held,U repeat){U s,c,a,idx,n;
 if(!_gz_state[0])return 0;
 if(_gz_page==5&&(edges&0x2000u)){
  idx=_gz_list_offset+_gz_state[2];
  if(_gz_state[2]<_gz_list_count&&idx<FAV_CAP)request_toggle(idx);
  return 1;
 }
 if(_gz_page!=FAV_PAGE)return 0;
 if(match(edges,held,_gz_bind_masks[6])){
  if(active<group_count){active=FAV_AREA_CAP;_gz_state[2]=root_row;top=root_top;_gz_fav_report[15]=0;clamp_page();}
  else{_gz_page=0;_gz_state[2]=FAV_ROOT_INDEX;}
  return 1;
 }
 n=rows();if(!_gz_fav_loaded||!n)return 1;
 s=_gz_state[2];if(s>=n)s=n-1;
 if(match(edges|repeat,held,_gz_bind_masks[3]))s=s?s-1:n-1;
 if(match(edges|repeat,held,_gz_bind_masks[4]))s=s+1<n?s+1:0;
 _gz_state[2]=s;clamp_page();
 if(active>=group_count){
  if(match(edges,held,_gz_bind_masks[5])){root_row=s;root_top=top;active=s;_gz_state[2]=top=0;_gz_fav_report[15]=1;}
  return 1;
 }
 idx=indices[groups[active].first+s];
 if(edges&0x2000u){request_toggle(idx);return 1;}
 if(!_gz_fav_toggle_request&&match(edges,held,_gz_bind_masks[5])&&location(idx,&c,&a)&&!_gz_pending){
  _gz_route_selection=c;_gz_area_selection=a;_gz_route_choice=idx;_gz_pending=4;
 }
 return 1;
}
void _gz_fav_root_draw(void){text(4,FAV_ROOT_Y,"Favorites");}
void _gz_fav_route_hint(void){U k[2],idx;
 if(_gz_page!=5)return;
 idx=_gz_list_offset+_gz_state[2];
 if(_gz_state[2]>=_gz_list_count||idx>=FAV_CAP)return;
 if(_gz_fav_loaded&&key(idx,k)&&find(k)<FAV_CAP){text(4,2,"Favorite");text(4,19,"X: Remove Favorite");}
 else text(4,19,"X: Add Favorite");
}
void _gz_fav_draw(void){U n,s,c,a,idx,total=rows();
 text(2,1,"Favorites");
 if(!_gz_fav_loaded){text(4,4,"Loading favorites...");if(_gz_fav_report[8])text(4,6,"Could not read favorites. Retrying...");text(4,19,"B: Back");return;}
 if(!visible){text(4,4,_gz_fav_count?"No available favorites":"No favorites yet");text(4,6,"Press X on a route memfile.");}
 else{
  s=_gz_state[2];if(s>=total)s=total-1;
  if(active<group_count)text(4,2,groups[active].name);
  for(n=0;n<10&&top+n<total;n++){
   const char*name=active<group_count?_gz_route_records[indices[groups[active].first+top+n]].name:groups[top+n].name;
   copy(lines[n],name,64);text(4,4+n,lines[n]);if(top+n==s)text(2,4+n,">");
  }
  if(active<group_count){
   idx=indices[groups[active].first+s];if(location(idx,&c,&a)){
    copy(context[0],_gz_route_folders[c].name,96);text(4,15,context[0]);
   }
  }
 }
 if(_gz_fav_report[8])text(4,17,"Favorites not saved yet. Retrying...");
 else if(_gz_fav_dirty)text(4,17,"Saving favorites...");
 else if(_gz_fav_report[9])text(4,17,"Favorites saved");
 if(_gz_fav_report[5])text(4,18,"Some favorites are unavailable in this build.");
 text(4,19,active<group_count?"A: Load   X: Remove   B: Areas":"A: Open Area   B: Back");
}

/* Current derived page size for authored analog cursor navigation only.
 * No key/format/storage mutation or activation is performed by this getter. */
U _gz_fav_navigation_count(void)
{
    return _gz_fav_loaded ? rows() : 0;
}
