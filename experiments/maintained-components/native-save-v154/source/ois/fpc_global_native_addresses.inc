/* Native addresses are ELF relocation targets, never canonical numeric
 * wire keys. ROOT must preserve these real ABS aliases and every ADDR32
 * native DATA relocation through final payload collection/packing. */
typedef struct {FgcU canonical;const unsigned char*native;} FgcNativeAddress;
extern const unsigned char _native_game_literal_101f3908[];
extern const unsigned char _native_game_literal_101f390c[];
extern const unsigned char _native_game_literal_101f3934[];
extern const unsigned char _native_game_literal_101f39a4[];
extern const unsigned char _native_game_literal_101f3a1c[];
extern const unsigned char _native_game_literal_101f3ae8[];
extern const unsigned char _native_game_literal_101f3b18[];
extern const unsigned char _native_game_literal_101f3b24[];
extern const unsigned char _native_game_literal_101f3b28[];
extern const unsigned char _native_game_literal_101f3bd8[];
extern const unsigned char _native_game_literal_101f3c14[];
extern const unsigned char _native_game_literal_101f3d60[];
extern const unsigned char _native_game_literal_101f3d64[];
extern const unsigned char _native_game_literal_101f3da4[];
extern const unsigned char _native_game_literal_101f3dbc[];
extern const unsigned char _native_game_literal_1048a87c[];
extern const unsigned char _native_game_literal_1048a9bc[];
extern const unsigned char _native_game_literal_1048a9e8[];
extern const unsigned char _native_game_literal_1048ab40[];
extern const unsigned char _native_game_literal_101f835c[];
extern const unsigned char _native_game_literal_101f3aec[];
extern const unsigned char _native_game_literal_101f3b74[];
extern const unsigned char _native_game_literal_101f3e6c[];
static const FgcNativeAddress fgc_native_roots[FGC_COUNT]={
 {0x101f3908u,_native_game_literal_101f3908},
 {0x101f390cu,_native_game_literal_101f390c},
 {0x101f3934u,_native_game_literal_101f3934},
 {0x101f39a4u,_native_game_literal_101f39a4},
 {0x101f3a1cu,_native_game_literal_101f3a1c},
 {0x101f3ae8u,_native_game_literal_101f3ae8},
 {0x101f3b18u,_native_game_literal_101f3b18},
 {0x101f3b24u,_native_game_literal_101f3b24},
 {0x101f3b28u,_native_game_literal_101f3b28},
 {0x101f3bd8u,_native_game_literal_101f3bd8},
 {0x101f3c14u,_native_game_literal_101f3c14},
 {0x101f3d60u,_native_game_literal_101f3d60},
 {0x101f3d64u,_native_game_literal_101f3d64},
 {0x101f3da4u,_native_game_literal_101f3da4},
 {0x101f3dbcu,_native_game_literal_101f3dbc},
 {0x1048a87cu,_native_game_literal_1048a87c},
 {0x1048a9bcu,_native_game_literal_1048a9bc},
 {0x1048a9e8u,_native_game_literal_1048a9e8},
 {0x1048ab40u,_native_game_literal_1048ab40},
 {0x101f835cu,_native_game_literal_101f835c},
};
static const FgcNativeAddress fgc_native_templates[3]={
 {0x101f3aecu,_native_game_literal_101f3aec},
 {0x101f3b74u,_native_game_literal_101f3b74},
 {0x101f3e6cu,_native_game_literal_101f3e6c},
};
static FgcU fgc_native_root(FgcU index){
 if(index>=FGC_COUNT||_gz_fgc_layout[index].id!=index+1u||
  fgc_native_roots[index].canonical!=_gz_fgc_layout[index].address)return 0;
 return(FgcU)fgc_native_roots[index].native;}
static FgcU fgc_native_template(FgcU index){
 if(index>=3||fgc_native_templates[index].canonical!=fgc_static[index].address)return 0;
 return(FgcU)fgc_native_templates[index].native;}
