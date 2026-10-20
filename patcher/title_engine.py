"""Apply the approved HD-GZ badge to the user's own native title archive."""
import struct,hashlib,json
from pathlib import Path

def unpack(b):
 if b[:4]!=b'Yaz0':return b
 size=int.from_bytes(b[4:8],'big')
 if not 0<size<=32*1024*1024:raise ValueError('Invalid title archive size.')
 out=bytearray();pos=16;bits=0;code=0
 try:
  while len(out)<size:
   if not bits:code=b[pos];pos+=1;bits=8
   if code&128:out.append(b[pos]);pos+=1
   else:
    a,c=b[pos:pos+2];pos+=2;distance=((a&15)<<8)|c;count=a>>4
    if not count:count=b[pos]+18;pos+=1
    else:count+=2
    for _ in range(count):out.append(out[-distance-1])
   code=(code<<1)&255;bits-=1
 except (IndexError,ValueError):raise ValueError('Corrupt native title archive.') from None
 if len(out)!=size:raise ValueError('Title archive decompression size mismatch.')
 return bytes(out)

def entries(b):
 raw=unpack(b)
 if raw[:4]!=b'SARC' or raw[6:8]!=b'\xfe\xff':raise ValueError('Unsupported native title archive.')
 sfat=struct.unpack_from('>H',raw,4)[0];count=struct.unpack_from('>H',raw,sfat+6)[0]
 nodes=sfat+12;strings=nodes+16*count+8;data_start=struct.unpack_from('>I',raw,12)[0];found={}
 for i in range(count):
  attr,start,end=struct.unpack_from('>3I',raw,nodes+i*16+4)
  name=raw[strings+(attr&0xffffff)*4:].split(b'\0')[0].decode()
  if start>end or data_start+end>len(raw):raise ValueError('Title resource extends beyond its archive.')
  found[name]=raw[data_start+start:data_start+end]
 return raw,found,nodes,count

def patch_title(original,resources):
 resources=Path(resources);metadata=json.loads((resources/'title_badge_manifest.json').read_text())
 if hashlib.sha256(original).hexdigest()!=metadata['original_title_sha256']:
  raise ValueError('Title_00.szs differs from the verified EUR v0 title archive. Use the original base game.')
 badge=(resources/'title_badge.bflim').read_bytes()
 if hashlib.sha256(badge).hexdigest()!=metadata['badge_sha256']:raise ValueError('HD-GZ badge integrity check failed.')
 raw,old,nodes,count=entries(original);layout=bytearray(old['blyt/Title_00.bflyt'])
 offset=struct.unpack_from('>H',layout,6)[0];panes=[]
 while offset<len(layout):
  magic=layout[offset:offset+4];size=struct.unpack_from('>I',layout,offset+4)[0]
  if size<8 or offset+size>len(layout):raise ValueError('Invalid title pane.')
  if magic==b'pic1':
   name=layout[offset+12:offset+36].split(b'\0')[0].decode()
   if name in ('P_TitleLogoHD_00','P_TitleLogoHDJ_00'):
    struct.pack_into('>f',layout,offset+44,76.0 if name=='P_TitleLogoHD_00' else 170.0)
    struct.pack_into('>2f',layout,offset+76,208.0,46.0);panes.append(name)
  offset+=size
 if len(panes)!=2:raise ValueError('Expected native HD title panes were not found.')
 replacements={'timg/TitleLogoHD_00^l.bflim':badge,'blyt/Title_00.bflyt':bytes(layout)}
 data_start=struct.unpack_from('>I',raw,12)[0];sarc=bytearray(raw[:data_start]);data=bytearray()
 sfat=struct.unpack_from('>H',sarc,4)[0];strings=nodes+count*16+8
 for i in range(count):
  node=nodes+i*16;attr=struct.unpack_from('>I',sarc,node+4)[0]
  name=sarc[strings+(attr&0xffffff)*4:].split(b'\0')[0].decode()
  body=replacements.get(name,old[name]);data.extend(bytes((-len(data))%2048));start=len(data);data.extend(body)
  struct.pack_into('>2I',sarc,node+8,start,len(data))
 sarc.extend(data);struct.pack_into('>I',sarc,8,len(sarc))
 result=b'Yaz0'+struct.pack('>I',len(sarc))+bytes(8)+b''.join(b'\xff'+sarc[i:i+8] for i in range(0,len(sarc),8))
 _,new,_,_=entries(result)
 if set(new)!=set(old) or sorted(k for k in old if old[k]!=new[k])!=sorted(replacements):
  raise ValueError('Unexpected native title resource changes.')
 return result
