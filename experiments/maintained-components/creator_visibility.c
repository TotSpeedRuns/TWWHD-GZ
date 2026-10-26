/* Session-local presentation setting. The capture beacon and memory transport
 * remain unchanged when the caption/number is hidden. */
extern volatile unsigned int _gz_ccv_visible;
extern void _gz_game_text(unsigned int,unsigned int,unsigned int,const char*);
void _gz_ccv_toggle(void){_gz_ccv_visible=!_gz_ccv_visible;}
void _gz_ccv_root_draw(void){
 _gz_game_text(0,4,12,_gz_ccv_visible?"Show Creator Code: On":"Show Creator Code: Off");
}
