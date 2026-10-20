"""Strict ELF32/PPC RPX parsing and independent native section serialization."""
import struct,zlib
from dataclasses import dataclass

COMPRESSED=0x08000000
def align(n,a):return (n+a-1)&-a
@dataclass
class Section:
 header:list
 data:bytes
 @property
 def address(self):return self.header[3]
 @property
 def kind(self):return self.header[1]

class Elf:
 def __init__(self,raw):
  if len(raw)<52 or raw[:7]!=b'\x7fELF\x01\x02\x01':raise ValueError('Expected big-endian ELF32.')
  if struct.unpack_from('>H',raw,18)[0]!=20:raise ValueError('Expected PowerPC executable.')
  self.header=bytearray(raw[:52]);off=struct.unpack_from('>I',raw,32)[0]
  stride,count,self.names_index=struct.unpack_from('>3H',raw,46)
  if stride!=40 or not 1<=count<=256 or off+count*40>len(raw):raise ValueError('Invalid section headers.')
  self.sections=[]
  for i in range(count):
   h=list(struct.unpack_from('>10I',raw,off+i*stride))
   if h[1] in (0,8):payload=b''
   else:
    if h[4]+h[5]>len(raw):raise ValueError('Section extends beyond file.')
    payload=raw[h[4]:h[4]+h[5]]
    if h[2]&COMPRESSED:
     if len(payload)<4:raise ValueError('Invalid compressed section.')
     expected=struct.unpack_from('>I',payload)[0]
     if expected>64*1024*1024:raise ValueError('Section expansion exceeds limit.')
     decoder=zlib.decompressobj();decoded=decoder.decompress(payload[4:],expected+1)
     if len(decoded)!=expected or not decoder.eof or decoder.unused_data:raise ValueError('Compressed section length mismatch.')
     payload=decoded
   self.sections.append(Section(h,payload))
  info=next((s.data for s in self.sections if s.kind==0x80000004),None)
  text_sections=[s for s in self.sections if s.header[2]&2 and s.header[2]&4 and s.kind not in (0x80000001,0x80000002)]
  self.text_trampoline_reserve=0
  if info and text_sections:
   start=min(s.address for s in text_sections)
   end=max(s.address+len(s.data) for s in text_sections)
   self.text_trampoline_reserve=max(0,struct.unpack_from('>I',info,4)[0]-struct.unpack_from('>I',info,32)[0]-align(end-start,16))
 def name(self,index):
  s=self.sections[index];data=self.sections[self.names_index].data
  return data[s.header[0]:].split(b'\0',1)[0].decode('ascii')
 def symbols(self):
  result=[]
  for si,s in enumerate(self.sections):
   if s.kind!=2:continue
   strings=self.sections[s.header[6]].data
   for i in range(0,len(s.data),16):
    name,value,size,info,other,section=struct.unpack_from('>IIIBBH',s.data,i)
    result.append(dict(index=i//16,table=si,name=strings[name:].split(b'\0',1)[0].decode('ascii'),value=value,size=size,info=info,section=section))
  return result
 def section_at(self,address,size=4):
  matches=[(i,s,address-s.address) for i,s in enumerate(self.sections) if s.kind!=8 and s.address and s.address<=address and address+size<=s.address+len(s.data)]
  if len(matches)!=1:raise ValueError(f'Address {address:08x} is not in one section.')
  return matches[0]
 def write_word(self,address,value):
  i,s,o=self.section_at(address);b=bytearray(s.data);struct.pack_into('>I',b,o,value);s.data=bytes(b)
 def add(self,data,address,flags,kind=1,link=0,info=0,entrysize=0):
  self.sections.append(Section([0,kind,flags,address,0,len(data),link,info,64,entrysize],bytes(data)))
  return len(self.sections)-1
 def encode_rpx(self):
  # Cafe OS/Cemu require CRC and FILEINFO to be the last two section headers.
  # New sections go before them; remap all affected section-index references.
  metadata=[i for i,s in enumerate(self.sections) if s.kind in (0x80000003,0x80000004)]
  if len(metadata)!=2:raise ValueError('RPX must have one CRC and one FILEINFO section.')
  crc_old=next(i for i in metadata if self.sections[i].kind==0x80000003)
  info_old=next(i for i in metadata if self.sections[i].kind==0x80000004)
  order=[i for i in range(len(self.sections)) if i not in metadata]+[crc_old,info_old]
  remap={old:new for new,old in enumerate(order)}
  self.sections=[self.sections[i] for i in order]
  self.names_index=remap[self.names_index];struct.pack_into('>H',self.header,50,self.names_index)
  for s in self.sections:
   if s.header[6]:s.header[6]=remap[s.header[6]]
   if s.kind==4:s.header[7]=remap[s.header[7]]
   if s.kind==2:
    symbols=bytearray(s.data)
    for offset in range(0,len(symbols),16):
     index=struct.unpack_from('>H',symbols,offset+14)[0]
     if 0<index<0xff00:struct.pack_into('>H',symbols,offset+14,remap[index])
    s.data=bytes(symbols)
  crc_index=next(i for i,s in enumerate(self.sections) if s.kind==0x80000003)
  fi=next(s for s in self.sections if s.kind==0x80000004)
  info=bytearray(fi.data)
  for base,size_offset,alignment_offset in [(0x02000000,4,8),(0x10000000,12,16),(0xc0000000,20,24)]:
   ceiling=0x10000000 if base==0x02000000 else 0xc0000000 if base==0x10000000 else 0x100000000
   end=max(s.address+(s.header[5] if s.kind==8 else len(s.data)) for s in self.sections if base<=s.address<ceiling)
   boundary=struct.unpack_from('>I',info,alignment_offset)[0]
   if not boundary or boundary&(boundary-1):raise ValueError('Invalid FILEINFO alignment.')
   reserved=struct.unpack_from('>I',info,76)[0] if base==0xc0000000 else 0
   if base==0x02000000:
    # Cafe's linker derives post-trampoline capacity from TEXT size itself.
    # trampAddition alone does not restore the original trailing allocation.
    # Preserve the original reserve and add space for the new import calls.
    reserved=struct.unpack_from('>I',info,32)[0]+self.text_trampoline_reserve+4096
   struct.pack_into('>I',info,size_offset,align(end-base,boundary)+reserved)
  temp=sum(len(s.data)+128 for s in self.sections if not s.address and s.kind not in (0,0x80000003,0x80000004))
  old_temp=struct.unpack_from('>I',info,28)[0];struct.pack_into('>I',info,28,max(old_temp,temp))
  # Extra native import branch sites may need loader-created trampolines.
  extra=struct.unpack_from('>I',info,72)[0];struct.pack_into('>I',info,72,max(extra,256))
  fi.data=bytes(info)
  self.sections[crc_index].data=b''.join(struct.pack('>I',0 if i==crc_index or s.kind in (0,8) else zlib.crc32(s.data)&0xffffffff) for i,s in enumerate(self.sections))
  def group(s):
   if s.kind==0x80000003:return 0
   if s.kind==0x80000004:return 1
   if s.header[2]&3==3 and not s.header[2]&4 and s.kind not in (0x80000001,0x80000002):return 2
   if s.header[2]&2 and (s.kind in (0x80000001,0x80000002) or not s.header[2]&5):return 3
   if s.header[2]&4:return 4
   return 5
  count=len(self.sections);out=bytearray(align(64+40*count,64))
  out[:52]=self.header;struct.pack_into('>I',out,32,64);struct.pack_into('>H',out,48,count)
  for i in sorted(range(count),key=lambda i:(group(self.sections[i]),i)):
   s=self.sections[i];h=s.header
   if s.kind in (0,8):h[4]=0;continue
   data=s.data
   if h[2]&COMPRESSED:data=struct.pack('>I',len(data))+zlib.compress(data,9)
   pos=align(len(out),64);out.extend(bytes(pos-len(out)));h[4]=pos;h[5]=len(data);out.extend(data)
  for i,s in enumerate(self.sections):struct.pack_into('>10I',out,64+40*i,*s.header)
  # Reference wiiurpxtool pads after every section, including the final one.
  # Keep the whole RPX aligned for the native loader's file reads.
  out.extend(bytes(align(len(out),64)-len(out)))
  return bytes(out)
 def verify_crcs(self):
  if [s.kind for s in self.sections[-2:]]!=[0x80000003,0x80000004]:raise ValueError('RPX section headers must end with CRC and FILEINFO.')
  i=next(i for i,s in enumerate(self.sections) if s.kind==0x80000003)
  table=self.sections[i].data
  if len(table)!=4*len(self.sections):raise ValueError('CRC table has wrong length.')
  for n,s in enumerate(self.sections):
   expected=0 if n==i or s.kind in (0,8) else zlib.crc32(s.data)&0xffffffff
   if struct.unpack_from('>I',table,n*4)[0]!=expected:raise ValueError(f'CRC mismatch: section {n}')
  # Expanded symbol/data tables must not overwrite adjacent mapped sections.
  mapped=[(n,s.address,s.address+(s.header[5] if s.kind==8 else len(s.data))) for n,s in enumerate(self.sections) if s.header[2]&2 and s.address]
  for offset,(n,start,end) in enumerate(mapped):
   for other,other_start,other_end in mapped[offset+1:]:
    if start<other_end and other_start<end:raise ValueError(f'Mapped RPX sections overlap: {n} and {other}')
  info=self.sections[-1].data
  # Native LiSetupOneRPL subtracts FILEINFO's reserved loader padding.
  loader=[s for s in self.sections if s.header[2]&2 and (not s.header[2]&5 or s.kind in (0x80000001,0x80000002)) and s.address>=0xc0000000]
  if loader:
   start=min(s.address for s in loader);end=max(s.address+len(s.data) for s in loader)
   capacity=struct.unpack_from('>I',info,20)[0]-struct.unpack_from('>I',info,76)[0]
   if end-start>capacity:raise ValueError('Native loader section bounds exceed FILEINFO capacity after reserved padding.')
  text=[s for s in self.sections if s.header[2]&2 and s.header[2]&4 and s.kind not in (0x80000001,0x80000002)]
  if any(s.address==0x02900000 for s in text):
   start=min(s.address for s in text);end=max(s.address+len(s.data) for s in text)
   available=struct.unpack_from('>I',info,4)[0]-struct.unpack_from('>I',info,32)[0]-align(end-start,16)
   if available<4096:raise ValueError('Native patched RPX has insufficient post-trampoline allocation.')

def relative_branch(source,target,link=False):
 delta=target-source
 if source%4 or target%4 or not -(1<<25)<=delta<(1<<25):raise ValueError('Native branch is out of range or unaligned.')
 return 0x48000000|(delta&0x03fffffc)|int(link)
