static const FgcU fgc_template_0[11]={0x00000000u,0x00000000u,0x00000000u,0xffffffffu,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u};
static const FgcU fgc_template_1[7]={0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0xffffffffu};
static const FgcU fgc_template_2[2]={0xffffffffu,0xffffffffu};
typedef struct {FgcU address,bytes;const FgcU *words;} FgcStatic;
static const FgcStatic fgc_static[3]={
 {0x101f3aecu,44,fgc_template_0}, /* LAYER_TEMPLATE: never restore */
 {0x101f3b74u,28,fgc_template_1}, /* LAYER_TAG_TEMPLATE: never restore */
 {0x101f3e6cu,8,fgc_template_2}, /* PRIORITY_CLEAR: never restore */
};
