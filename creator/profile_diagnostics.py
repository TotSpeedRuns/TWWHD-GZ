import struct
from memfile_core import CaptureError

def diagnose(m):
 if not getattr(m,'profile_map',None):raise CaptureError('A validated v75 GZ profile number is required for diagnosis.')
 raw=m._read_raw(m.profile_address+64,40)
 magic,version,*refs=struct.unpack('>10I',raw)
 if magic!=0x43504931 or version!=1 or any(not 0x10000000<=a<0x50000000-1024 or a%4 for a in refs):raise CaptureError('Unsupported GZ diagnostic profile.')
 queue,trace,full,status,valid,enemies,bridges,last=refs
 def word(a):return int.from_bytes(m._read_raw(a,4),'big')
 def pointer(a):
  if not 0x10000000<=a<0x50000000-0x4000 or a%4:raise CaptureError('Actor queue pointer outside mapped game memory.')
  return a
 out=dict(read_only=True,source=m.source,stage=m.read(0x104741e4,8).split(b'\0')[0].decode(),room=m.read(0x1047e6c8,1)[0],status=word(status),prior_snapshot_valid=word(valid),animation_trace_full=word(full),last_memfile=word(last),actors=[])
 node=word(queue);seen=set()
 while node and node not in seen and len(seen)<2048:
  pointer(node);seen.add(node);row=m._read_raw(node,16);actor=int.from_bytes(row[12:16],'big')
  if actor:
   pointer(actor);profile=int.from_bytes(m._read_raw(actor+8,2),'big')
   if profile in (66,89,189):
    item=dict(profile=profile,process_id=word(actor+4),address=hex(actor))
    if profile==66:item.update(mode=word(actor+0x3ac),configuration=m._read_raw(actor+0x9f0,1)[0])
    if profile==89:item.update(mode=int.from_bytes(m._read_raw(actor+0x3b4,2),'big'),segments=m._read_raw(actor+0x544,1)[0],configuration=m._read_raw(actor+0x540,1)[0],parameters=m._read_raw(actor+0xb0,4).hex(),room=m._read_raw(actor+0x326,1)[0])
    out['actors'].append(item)
  node=int.from_bytes(row[8:12],'big')
 out['queue_nodes']=len(seen);out['queue_cycle']=bool(node)
 data=m._read_raw(trace,1024);out['tracked_process_ids']=[struct.unpack_from('>I',data,i*32)[0] for i in range(32) if struct.unpack_from('>I',data,i*32)[0]]
 return out
