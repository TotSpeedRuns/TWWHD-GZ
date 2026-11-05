/* Authored recovery of Native0291d2a0 location formatting only.
 * No save, capture, slot publication, restore, owner or admission is supplied.
 */
#include "slot_location.h"
static SlotLocationPair pair(const char *text,SlotU room)
{
    return ((SlotLocationPair)(SlotU)text<<32)|room;
}
SlotLocationPair _gz_slot_location(const volatile SlotDescriptor *descriptor)
{
    SlotU room=(descriptor->room_fields>>8)&255u;
    SlotU i,first,second;
    volatile SlotB *scratch=(volatile SlotB *)_gz_slot_stage_buffer;
    const volatile SlotNameRow *row;
    _gz_slot_stage_buffer[0]=0;
    _gz_slot_stage_buffer[1]=0;
    _gz_slot_stage_buffer[2]=0;
    for(i=0;i<8;i++){
        SlotB c=descriptor->stage[i];
        if(!c)break;
        scratch[i]=c;
    }
    first=_gz_slot_stage_buffer[0];
    second=_gz_slot_stage_buffer[1];
    if(first==0x73656100u&&!second&&room>=1&&room<=49)
        return pair(_gz_map_quadrants[room-1].name,room);
    for(row=_gz_slot_location_table;;row++){
        SlotU key=row->stage0;
        if(!key)break;
        if(first==key&&second==row->stage1)return pair(row->name,room);
    }
    _gz_slot_stage_buffer[0]=first;
    _gz_slot_stage_buffer[1]=second;
    _gz_slot_stage_buffer[2]=0;
    return pair(first?(const char *)_gz_slot_stage_buffer:_gz_slot_text_unknown,room);
}
