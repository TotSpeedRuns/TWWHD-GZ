#ifndef GZ_ARCHIVE_LAYOUT_H
#define GZ_ARCHIVE_LAYOUT_H
/* Exact directories only. Legacy25 has no recorded rope state; it must not be
 * interpreted as a new empty-rope capture. Actor permission is unrelated. */
typedef unsigned int u32;typedef unsigned char u8;
static u32 gz_arc_shape(u32 p,u32 n){u32 c,h,i,end,o,b;
 if((p&3)||p<0x10000000u||p>=0xbffe0000u||n<216||n>0x200000u||n>0xbffe0000u-p||(n&3))return 0;
 if(*(volatile const u32*)p!=0x475a5331u||*(volatile const u32*)(p+4)!=n)return 0;
 c=*(volatile const u32*)(p+8);if((c!=25&&c!=26&&c!=27)||*(volatile const u32*)(p+12)!=c)return 0;
 h=16+c*8;if(n<h)return 0;end=h;
 for(i=0;i<c;i++){o=*(volatile const u32*)(p+16+8*i);b=*(volatile const u32*)(p+20+8*i);if(o!=end||(b&3)||b>n-end)return 0;end+=b;}
 return end==n?c:0;
}
#endif
