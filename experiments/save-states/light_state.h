#ifndef GZ_SEARCHLIGHT_STATE_V3_H
#define GZ_SEARCHLIGHT_STATE_V3_H
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
enum { L3_RECORD_MAGIC=0x4c335231u, L3_PACKET_MAGIC=0x4c335031u,
       L3_RECORD_BYTES=1024, L3_PACKET_OFFSET=327680, L3_CAPACITY=8 };
typedef struct {
    u32 magic,bytes,version,valid,pid,actor,parameters,set_id,room,mode,child_pid;
    u32 guards[7],home[5],payload_bytes,checksum,reserved[7];
    u8 payload[896];
} L3Record;
typedef struct {
    u32 magic,bytes,version,valid,count,frame,epoch,stage0,stage1,room,layer,link_pid;
    u32 checksum,reserved[3];
    L3Record records[L3_CAPACITY];
} L3Packet;
typedef char L3RecordSizeCheck[sizeof(L3Record)==1024?1:-1];
typedef char L3PacketSizeCheck[sizeof(L3Packet)==8256?1:-1];
u32 _gz_l3_capture_actor(u32,L3Record*);
u32 _gz_l3_check_actor(u32,const L3Record*);
u32 _gz_l3_apply_actor(u32,const L3Record*);
u32 _gz_l3_record_hash(const L3Record*);
u32 _gz_l3_record_valid(const L3Record*);
u32 _gz_l3_packet_hash(const L3Packet*);
extern const u16 _gz_l3_fields[];
extern const u16 _gz_l3_float_offsets[];
extern const u16 _gz_l3_guard_offsets[7];
#endif
