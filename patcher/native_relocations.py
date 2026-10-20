"""Address relocation support for payloads whose TEXT/DATA move on Cafe OS."""
import struct
from native_rpx import Elf,align

def collect_address_relocations(linked,objects,text,data):
 linked_symbols={s['name']:s for s in linked.symbols() if s['name']}
 result=[]
 for obj in objects:
  symbols=obj.symbols();by_index={s['index']:s for s in symbols};bases={}
  for index in range(1,len(obj.sections)):
   if not obj.sections[index].header[2]&2:continue
   anchors=[s for s in symbols if s['section']==index and s['name'] in linked_symbols]
   if anchors:
    base=linked_symbols[anchors[0]['name']]['value']-anchors[0]['value']
    if not all(linked_symbols[s['name']]['value']-s['value']==base for s in anchors):raise ValueError('Object section anchors disagree.')
    bases[index]=base
  for relsec in obj.sections:
   if relsec.kind!=4:continue
   source_index=relsec.header[7]
   if not obj.sections[source_index].header[2]&2:continue
   if source_index not in bases:raise ValueError('Missing linked object anchor.')
   for offset,info,addend in struct.iter_unpack('>IIi',relsec.data):
    kind=info&255
    if kind in (10,11):continue
    if kind not in (1,4,6):raise ValueError(f'Unsupported payload address relocation {kind}.')
    symbol=by_index[info>>8]
    if symbol['name'] in linked_symbols:value=linked_symbols[symbol['name']]['value']
    elif symbol['section'] in bases:value=bases[symbol['section']]+symbol['value']
    else:raise ValueError('Cannot resolve payload address symbol.')
    address=bases[source_index]+offset;width=4 if kind==1 else 2
    sections=[s for s in (text,data) if s.address<=address and address+width<=s.address+len(s.data)]
    if len(sections)!=1:raise ValueError('Payload relocation source is out of bounds.')
    section=sections[0];within=address-section.address;resolved=(value+addend)&0xffffffff
    expected=resolved if kind==1 else resolved&0xffff if kind==4 else ((resolved+0x8000)>>16)&0xffff
    if int.from_bytes(section.data[within:within+width],'big')!=expected:raise ValueError('Object relocation does not match linked payload.')
    result.append(dict(address=address,target=value,kind=kind,addend=addend))
 if len({r['address'] for r in result})!=len(result):raise ValueError('Duplicate payload address relocation.')
 return result

def add_address_relocations(elf,records,table=28):
 if not records:return dict(address_relocations=0,address_symbols=0)
 original_symbols=elf.symbols();count=len(elf.sections[table].data)//16;added={};groups={}
 def section_at(value,width=1):
  matches=[i for i,s in enumerate(elf.sections) if s.header[2]&2 and s.address and s.address<=value
           and value+width<=s.address+(s.header[5] if s.kind==8 else len(s.data))]
  if len(matches)!=1:raise ValueError(f'No unique mapped relocation section at {value:08x}.')
  return matches[0]
 for record in records:
  address=record['address'];value=record['target'];kind=record['kind'];addend=record['addend']
  if kind not in (1,4,6):raise ValueError('Unsupported address relocation.')
  source=section_at(address,4 if kind==1 else 2)
  if value not in added:
   target=section_at(value);index=count+len(added)
   elf.sections[table].data+=struct.pack('>IIIBBH',0,value,0,0x10,0,target);added[value]=index
  groups.setdefault(source,[]).append((address,added[value]<<8|kind,addend))
 # Keep existing import symbol indexes, but move the expanded SYMTAB into
 # free loader space so it cannot overwrite the original STRTAB.
 if any(s['section']==table for s in original_symbols):raise ValueError('Cannot move referenced symbol table.')
 end=max(s.address+len(s.data) for i,s in enumerate(elf.sections) if i!=table and s.header[2]&2 and s.address>=0xc0000000)
 elf.sections[table].header[3]=align(end,64)
 for source,entries in groups.items():
  if len({v[0] for v in entries})!=len(entries):raise ValueError('Duplicate address relocation site.')
  elf.add(b''.join(struct.pack('>IIi',*v) for v in entries),0,0,kind=4,link=table,info=source,entrysize=12)
 return dict(address_relocations=len(records),address_symbols=len(added))
