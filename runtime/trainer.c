/* Read-only trainer telemetry. Native input, animation and collision are never edited. */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int s32;
extern volatile u32 _gz_roll_state[28];
extern volatile u32 _gz_game_counter;
extern u8 _gz_roll_bg[];
typedef struct { u32 valid; s32 delta; u32 partial; u32 accepted; } Feedback;
volatile Feedback _gz_roll_feedback;
volatile u32 _gz_roll_result_tick;
typedef struct {
    u32 tick, tick_valid, count, start_valid;
    u32 start[3], last[3];
} Clip;
volatile Clip _gz_roll_clip_state;
extern void _gz_roll_line_set(void*,const float*,const float*,void*);
extern int _gz_roll_line_cross(void*,void*);
extern int _gz_roll_poly_safe(void*,void*);
extern float* _gz_roll_tri_plane(void*,u32,u32);
#define RS _gz_roll_state
#define FB _gz_roll_feedback
#define CS _gz_roll_clip_state
#define W(p,o) (*(volatile u32*)((u32)(p)+(o)))
#define H(p,o) (*(volatile u16*)((u32)(p)+(o)))
#define B(p,o) (*(volatile u8*)((u32)(p)+(o)))
#define F(p,o) (*(volatile float*)((u32)(p)+(o)))
#define NOW _gz_game_counter
static int finite_word(u32 x) { return (x&0x7F800000)!=0x7F800000; }
static float absf(float x) { return x<0?-x:x; }
static int address(u32 x) { return x>=0x10000000 && x<0xBFFF0000 && !(x&3); }
static void clear_clip(void) {
    CS.tick=CS.tick_valid=CS.count=CS.start_valid=0;
}
static void clear_feedback(void) {
    FB.valid=FB.partial=FB.accepted=0;FB.delta=0;
    _gz_roll_result_tick=0;
}
void _gz_roll_feedback_expire(void) {
    /* Native 30 Hz updates. Partial-only observations also expire. RS27
       prevents the same stuck climb from republishing the expired result. */
    if((FB.valid || FB.partial) && (u32)(NOW-_gz_roll_result_tick)>=90) {
        /* A detected setup is an active-attempt latch. Climbing can move the
           live collision back in bounds; that is not a failed roll clip.
           Timing messages still expire while the partial latch stays set. */
        if(FB.partial==2 && RS[0] && !RS[25] && RS[14]>=43 && RS[14]<=48) {
            FB.valid=FB.accepted=0;FB.delta=0;return;
        }
        u32 was=FB.partial;clear_feedback();RS[27]=was==2?2:1;
    }
}
static void publish_partial(u32 result) {
    if(RS[27] && result==1) return; /* An expired miss may still become a real clip. */
    if(result!=FB.partial) _gz_roll_result_tick=NOW;
    FB.partial=result;
}
void _gz_roll_cancel(void) {
    RS[0]=RS[1]=RS[3]=RS[5]=RS[9]=RS[14]=RS[25]=0;
    clear_clip(); /* Keep unexpired feedback visible after tracking ends. */
}
void _gz_roll_begin(void* link) {
    u32 held=RS[8];
    /* Hang -> climb is one setup. A fresh grab, changed Link, completed roll
       or animation rewind remains a new attempt and clears the old latch. */
    u32 keep_partial=RS[0] && RS[1]==(u32)link && !RS[25] && FB.partial==2 &&
        W(link,0x65F0)==48 && RS[14]>=43 && RS[14]<48;
    for(u32 i=0;i<28;i++) RS[i]=0;
    RS[0]=1;RS[1]=(u32)link;RS[2]=NOW;RS[8]=held;RS[14]=W(link,0x65F0);
    clear_clip();
    clear_feedback();
    if(keep_partial) { FB.partial=2;_gz_roll_result_tick=NOW; }
    for(u32 i=0;i<3;i++) CS.start[i]=W(link,0x314+4*i);
    CS.start_valid=1;
}
static void publish(s32 delta,u32 accepted) {
    FB.delta=delta;if(accepted)FB.partial=0;
    FB.accepted=accepted;_gz_roll_result_tick=NOW;FB.valid=1;
    RS[12]=(u32)delta;
}
void _gz_roll_gate_mark(void* link) {
    if(W(link,0x65F0)!=48) return;
    if(!RS[0] || RS[1]!=(u32)link || RS[14]!=48) _gz_roll_begin(link);
    if(RS[3]) return;
    RS[3]=1;RS[4]=NOW;
    if(!CS.start_valid) {
        for(u32 i=0;i<3;i++) CS.start[i]=W(link,0x314+4*i);
        CS.start_valid=1;
    }
    /* Only an edge in the final three animation frames is an early attempt.
       The initiating climb A and earlier climb taps are never candidates. */
    if(RS[5]==2 && NOW-RS[6]<=4) publish((s32)(RS[6]-NOW),0);
    else RS[5]=0;
}
void _gz_roll_press(void* link) {
    u32 proc=W(link,0x65F0),now=NOW;
    if(proc==48 && (!RS[0] || RS[1]!=(u32)link || RS[14]!=48)) _gz_roll_begin(link);
    if(!RS[0] || RS[1]!=(u32)link || RS[25]) return;
    if(!RS[3]) {
        /* Native procHangClimb first checks next mode after animation frame23.
           Comparing positive float words avoids using FPRs in the input hook. */
        u32 frame=W(link,0x589C),rate=W(link,0x5898);
        if(proc!=48 || now==RS[2] || frame<0x41A80000 || frame>0x41C00000 ||
           rate==0 || rate>=0x7F800000) return;
        RS[5]=2;RS[6]=now;return;
    }
    if(now-RS[4]>8 || (proc!=48 && proc!=4 && proc!=5 && proc!=30 && proc!=31)) return;
    RS[5]=1;RS[6]=now;
}
void _gz_roll_gate_finish(void* link) {
    if(!RS[0] || RS[1]!=(u32)link || !RS[3] || RS[25]) return;
    u32 proc=W(link,0x65F0),now=NOW;
    if((proc!=30 && proc!=31) || now-RS[4]>8) return;
    /* A native roll is authoritative; a previous early tap must not lock it out.
       The gate surrounds checkNextMode, so input callback order can differ. */
    RS[9]=1;RS[10]=proc==31;RS[24]=now;RS[25]=1;
    RS[5]=1;RS[6]=now;CS.count=0;
    publish((s32)(now-RS[4]),1);
}
/* Observe native wall collision circles against finite, Link-solid polygons.
 * All queries and callbacks use private scratch. No player/world writes. */
extern void* _gz_roll_sphere_construct(void*);
extern int _gz_roll_sphere_cross(void*,void*,void*);
volatile u32 _gz_roll_volume_trace[24];
typedef struct { float x,y,z,r; u32 hit; } Volume;

static float segment_distance2(float x,float z,const float* a,const float* b) {
    float dx=b[0]-a[0],dz=b[1]-a[1],length=dx*dx+dz*dz;
    float t=length>0.f?((x-a[0])*dx+(z-a[1])*dz)/length:0.f;
    if(t<0.f)t=0.f;else if(t>1.f)t=1.f;
    dx=x-(a[0]+t*dx);dz=z-(a[1]+t*dz);
    return dx*dx+dz*dz;
}

/* Callback ABI verified in native RwgSphChk at 024F3FE0..024F4004.
 * The sphere query finds every nearby triangle via the native spatial tree;
 * this exact horizontal slice excludes surfaces merely above/below a circle. */
void _gz_roll_volume_triangle(void* query,u32 vertices,u32 ia,u32 ib,u32 ic,
                              const float* plane,Volume* c) {
    (void)query;
    _gz_roll_volume_trace[3]++;
    if(!address(vertices)||!address((u32)plane)||ia>200000||ib>200000||ic>200000)return;
    for(u32 i=0;i<4;i++)if(!finite_word(W(plane,4*i)))return;
    float nx=plane[0],ny=plane[1],nz=plane[2];
    /* Match the native ground/roof/wall classification. */
    if(ny>=.5f||ny<-.8f)return;
    float h2=nx*nx+nz*nz,n2=h2+ny*ny;
    if(h2<.3f||n2<.99f||n2>1.01f)return;
    /* Slightly shallower setup: require45% of the live wall-circle radius
     * to overlap. Tiny grazing contacts still do not claim a partial setup. */
    float s=nx*c->x+ny*c->y+nz*c->z+plane[3],r=c->r*.55f;
    if(r<=0.f||s*s>=r*r*h2)return;
    u32 vi[3]={ia,ib,ic};float v[3][3],points[6][2];u32 count=0;
    for(u32 i=0;i<3;i++)for(u32 j=0;j<3;j++) {
        u32 at=vertices+vi[i]*12+j*4;
        if(!address(at)||!finite_word(W(at,0)))return;
        v[i][j]=F(at,0);
    }
    for(u32 i=0;i<3;i++) {
        u32 j=(i+1)%3;float a=v[i][1]-c->y,b=v[j][1]-c->y;
        if(a==0.f){points[count][0]=v[i][0];points[count++][1]=v[i][2];}
        if((a<0.f&&b>0.f)||(a>0.f&&b<0.f)) {
            float t=a/(a-b);
            points[count][0]=v[i][0]+t*(v[j][0]-v[i][0]);
            points[count++][1]=v[i][2]+t*(v[j][2]-v[i][2]);
        }
    }
    if(!count)return;
    float distance=segment_distance2(c->x,c->z,points[0],points[0]);
    for(u32 i=0;i<count;i++)for(u32 j=i+1;j<count;j++) {
        float d=segment_distance2(c->x,c->z,points[i],points[j]);
        if(d<distance)distance=d;
    }
    if(distance>=r*r)return;
    c->hit=1;_gz_roll_volume_trace[4]=1;
    for(u32 i=0;i<4;i++)_gz_roll_volume_trace[12+i]=W(plane,i*4);
    F(_gz_roll_volume_trace,16*4)=s;
    F(_gz_roll_volume_trace,17*4)=distance;
}

static int collision_volume(void* link) {
    u32 template=(u32)link+0x9D0,acch=(u32)link+0x80C;
    for(u32 i=0;i<24;i++)_gz_roll_volume_trace[i]=0;
    _gz_roll_volume_trace[0]=NOW;_gz_roll_volume_trace[1]=W(link,0x65F0);
    /* Verify this is Link's current three-circle collision owner. */
    if(W(acch,0x2C)!=(u32)link+0x314||W(acch,0x84)!=3||
       W(acch,0x88)!=(u32)link+0x74C||
       W(template,0)!=template+0x58||W(template,4)!=template+0x64)return 0;
    for(u32 i=0;i<3;i++)if(!finite_word(W(link,0x314+4*i)))return 0;
    u32 q[20];_gz_roll_sphere_construct(q);
    /* Retain the native Link poly/group filters, rebased inside this query. */
    for(u32 i=0;i<3;i++)q[14+i]=W(template,0x58+4*i);
    for(u32 i=0;i<2;i++)q[17+i]=W(template,0x64+4*i);
    q[9]=(u32)q+0x38;q[10]=(u32)q+0x44;q[11]=W(link,4);
    q[19]=(u32)_gz_roll_volume_triangle;
    float prior_h=0.f,prior_r=0.f;
    for(u32 i=0;i<3;i++) {
        u32 circle=(u32)link+0x74C+i*0x40;
        if(!finite_word(W(circle,0x30))||!finite_word(W(circle,0x34)))return 0;
        float h=F(circle,0x30),r=F(circle,0x34);
        if(absf(h)>300.f||r<=0.f||r>100.f)return 0;
        if(i&&h==prior_h&&r==prior_r)continue;
        prior_h=h;prior_r=r;
        Volume c={F(link,0x314),F(link,0x318)+h,F(link,0x31C),r,0};
        F(q,0)=c.x;F(q,4)=c.y;F(q,8)=c.z;F(q,12)=r;
        _gz_roll_volume_trace[2]++;
        for(u32 j=0;j<4;j++)_gz_roll_volume_trace[5+j]=W(&c,j*4);
        _gz_roll_sphere_cross(_gz_roll_bg,q,&c);
        if(c.hit)return 1;
    }
    return 0;
}

static int body_point(void* link,float* point) {
    u32 proc=W(link,0x65F0);
    for(u32 i=0;i<3;i++) point[i]=F(link,0x314+4*i);
    point[1]+=30.f;
    if(proc<43 || proc>48) return 1;
    /* Link's actor position stays at the ledge height while his animated body
       hangs below it. Read CL_JNT_CENTER(1) without getAnmMtx's dirty store. */
    u32 model=W(link,0x448),data,buffer,matrices;
    if(!address(model) || !address(data=W(model,0xAC)) || data!=W(link,0x444) ||
       W(data,4)<2 || W(data,4)>128 || !address(buffer=W(model,0x2C)) ||
       !address(matrices=W(buffer,0x10))) return 0;
    u32 center=matrices+0x30;
    for(u32 i=0;i<3;i++) {
        u32 offset=12+16*i;
        if(!finite_word(W(center,offset))) return 0;
        point[i]=F(center,offset);
        if(absf(point[i]-F(link,0x314+4*i))>300.f) return 0;
    }
    return 1;
}
static int candidate(void* bg,void* q,void* link,const float* point) {
    u8* poly=(u8*)q+0x14;
    if(!_gz_roll_poly_safe(bg,poly)) return 0;
    u32 bgw=W(poly,4),head;
    if(!address(bgw) || !address(head=W(bgw,0x94)) || H(poly,0)>=W(head,8)) return 0;
    float* plane=_gz_roll_tri_plane(bg,H(poly,2),H(poly,0));
    if(!address((u32)plane)) return 0;
    for(u32 i=0;i<4;i++) if(!finite_word(W(plane,4*i))) return 0;
    float nx=plane[0],ny=plane[1],nz=plane[2],d=plane[3];
    float length=nx*nx+nz*nz;
    if(absf(ny)>.1f || length<.9f || length>1.1f) return 0;
    float before=nx*F(CS.start,0)+ny*(F(CS.start,4)+30.f)+nz*F(CS.start,8)+d;
    float after=nx*point[0]+ny*point[1]+nz*point[2]+d;
    u32 proc=W(link,0x65F0);
    if(proc>=43 && proc<=48)
        return before>=-20.f && before<150.f && after<-.5f && after>=-20.f;
    return before>.5f && before<150.f && after<-.5f && after>=-20.f;
}
static int probe(void* link) {
    u32 proc=W(link,0x65F0);if(proc>=43&&proc<=48)return collision_volume(link);
    u32 q[27];
    u32 template=(u32)link+0x9D0;
    /* Copy the native Link query into private scratch and rebase its two
       embedded pass/group pointers. Never reuse or mutate Link's query. */
    if(W(template,0)!=template+0x58 || W(template,4)!=template+0x64) return 0;
    for(u32 i=0;i<27;i++) q[i]=W(template,4*i);
    q[0]=(u32)q+0x58;q[1]=(u32)q+0x64;
    for(u32 i=0x10;i<=0x64;i+=4) {
        if(i!=0x10 && i!=0x20 && i!=0x58 && i!=0x64) continue;
        u32 vt=W(q,i);if(!address(vt)) return 0;
    }
    q[0x4C/4]=0;B(q,0x50)=B(q,0x51)=B(q,0x52)=B(q,0x54)=0;B(q,0x53)=1;
    float point[3];if(!body_point(link,point)) return 0;
    float x=point[0],y=point[1],z=point[2];
    float start[3],end[3];
    void* bg=_gz_roll_bg;
    for(u32 axis=0;axis<4;axis++) {
        start[0]=end[0]=x;start[1]=end[1]=y;start[2]=end[2]=z;
        u32 component=axis<2?0:2;
        float offset=(axis&1)?-64.f:64.f;
        start[component]+=offset;end[component]-=offset;
        _gz_roll_line_set(q,start,end,link);
        if(_gz_roll_line_cross(bg,q) && candidate(bg,q,link,point)) return 1;
    }
    return 0;
}
void _gz_roll_clip_sample(void* link) {
    if(!RS[0] || RS[1]!=(u32)link || !CS.start_valid || FB.partial==2 || RS[27]==2) return;
    u32 now=NOW,proc=W(link,0x65F0);
    if(CS.tick_valid && CS.tick==now) return;
    CS.tick=now;CS.tick_valid=1;
    int climbing=(proc>=43 && proc<=48);
    int good=(climbing || ((proc==4 || proc==5) && RS[25])) && finite_word(W(link,0x6A08));
    for(u32 i=0;i<3;i++) good=good && finite_word(W(link,0x314+4*i)) && finite_word(CS.start[i]);
    if(climbing) {
        /* The native collision volume may intersect a wall before the center
         * crosses it. No roll, button, stick, depth or memfile preset is needed. */
        if(!good) CS.count=0;
        else if(++CS.count>=2) { publish_partial(probe(link)?2:1);CS.count=0; }
    }
    else if(!good || F(link,0x6A08)<=.2f || absf(F(link,0x318)-F(CS.start,4))>45.f) CS.count=0;
    else {
        if(CS.count && (absf(F(link,0x314)-F(CS.last,0))>.5f ||
            absf(F(link,0x31C)-F(CS.last,8))>.5f || absf(F(link,0x318)-F(CS.last,4))>1.f)) CS.count=0;
        CS.count++;
        if(CS.count>=6) { publish_partial(probe(link)?2:1);CS.count=0; }
    }
    for(u32 i=0;i<3;i++) CS.last[i]=W(link,0x314+4*i);
}
void _gz_roll_observe(void* link) {
    _gz_roll_feedback_expire();
    u32 proc=W(link,0x65F0),hang=proc>=43 && proc<=48;
    if(RS[1]!=(u32)link) {
        _gz_roll_cancel();if(hang) _gz_roll_begin(link);else return;
    }
    if(hang) {
        u32 frame=W(link,0x589C),previous=RS[14];
        int oldhang=previous>=43 && previous<=48;
        int fresh=(proc==48 && previous!=48) ||
                  ((proc==43 || proc==45) && previous!=proc);
        if(!RS[0] || RS[25] || !oldhang || fresh ||
           (proc==48 && RS[20] && frame+0x10000<RS[20])) _gz_roll_begin(link);
        if(proc==48) RS[20]=frame;
    }
    RS[14]=proc;
    if(!RS[0]) return;
    /* Allow the native post-climb roll window, then clear on an actual roll
       or a failed/interrupted attempt. No collision resample can erase a
       detected setup while the same climb is still unresolved. */
    if(FB.partial==2 && !RS[25] && !hang &&
       (proc==30 || proc==31 || (proc!=4 && proc!=5) || !RS[3] || NOW-RS[4]>8)) {
        FB.partial=0;if(!FB.valid)_gz_roll_result_tick=0;
    }
    if(RS[3] && NOW-RS[4]>90) { _gz_roll_cancel();return; }
    if(RS[3] && RS[14]!=48 && !RS[25] && RS[5]==1 && NOW-RS[6]<=1) _gz_roll_gate_finish(link);
    _gz_roll_clip_sample(link);
}
