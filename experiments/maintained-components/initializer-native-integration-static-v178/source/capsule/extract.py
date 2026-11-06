"""Deterministic static dead-code extraction; no compiler or Native binding."""
from pathlib import Path
import ast,hashlib,json,struct,types,zlib
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
 raw=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest())
def save(name,obj):
 raw=obj if isinstance(obj,bytes)else(json.dumps(obj,indent=2)+'\n').encode()
 with(H/name).open('xb')as stream:stream.write(raw)
 assert(H/name).read_bytes()==raw
d=json.loads((H/'discovery.json').read_text())
assert pin(H/'discovery.json')['sha256']=='b7e8fca5082ed93dbdc8a234daa59a4daf71dfe2f350c9b6bca430a4e5d2f96e'
assert(d['function_count'],d['CODE_bytes'],d['DATA_bytes'])==(10,2740,1172)
assert not d['unhandled_direct_or_literal']and not d['indirect_branch_frontier']
for key in('source','Native','reader'):assert pin(B/d[key]['path'])==d[key]
source=B/d['source']['path'];reader=B/d['reader']['path']
node=next(v for v in ast.parse(reader.read_text()).body if isinstance(v,ast.ClassDef)and v.name=='Raw')
m=types.ModuleType('minimal_extract_Raw_only');m.__dict__.update(struct=struct,zlib=zlib);exec(compile(ast.Module(body=[node],type_ignores=[]),str(reader),'exec'),m.__dict__)
s=m.Raw(source)
pieces=[];headers=[[0]*10];payloads=[b''];names=[''];section_map={};symbol_location={}
def section(name,header,body):
 i=len(headers);h=list(header);h[0]=0;h[4]=0
 headers.append(h);payloads.append(body);names.append(name);return i
for desc in d['CODE_slices']:
 sec,begin,z=desc['Source_section'],desc['offset'],desc['bytes'];h=s.h[sec]
 assert h[1:4]==(1,6,0)and not h[2]&0x08000000
 name=('.text.init.thread'if sec==1 and z==1000 else'.text.init.gfo'if sec==1 else s.names[sec])
 idx=section(name,[0,1,6,0,0,z,0,0,h[8],0],s.d[sec][begin:begin+z])
 piece=desc|dict(output_section=idx,output_name=name);pieces.append(piece)
 for fn in d['functions']:
  f=fn['symbol']
  if f['section']==sec and begin<=f['value']and f['value']+f['size']<=begin+z:
   for alias in fn['aliases']:symbol_location[alias['table'],alias['index']]=(idx,alias['value']-begin)
data_sections=[]
for oldsec in(487,484,411):
 h=s.h[oldsec];assert h[1]in(1,8)and h[2]==3 and h[3]==0
 idx=section(s.names[oldsec],h,s.d[oldsec]);section_map[oldsec]=idx
 data_sections.append(dict(Source_section=oldsec,output_section=idx,name=s.names[oldsec],kind=h[1],bytes=h[5],logical_Source_definition_only=True))
 for sym in s.sy:
  if sym['section']==oldsec:symbol_location[sym['table'],sym['index']]=(idx,sym['value'])
sdk_section=212;assert s.names[sdk_section]=='.fimport_coreinit'and s.h[sdk_section][1]==0x80000002 and s.h[sdk_section][5]==8264
assert s.d[sdk_section][8:17]==b'coreinit\0'and not s.h[sdk_section][2]&0x08000000
sdk_output=section(s.names[sdk_section],s.h[sdk_section],s.d[sdk_section])
SDK={v['Source_symbol']['name']:v['Source_symbol']for v in d['SDK_roles']};assert len(SDK)==5
for sym in SDK.values():symbol_location[sym['table'],sym['index']]=(sdk_output,sym['value'])
external={v['row']['symbol']['name']:v['row']['symbol']for v in d['external_UND']};assert len(external)==4
selected={k:next(v for v in s.sy if(v['table'],v['index'])==k)for k in symbol_location}
for sym in external.values():selected[sym['table'],sym['index']]=sym
assert len(selected)==33
selected=sorted(selected.values(),key=lambda v:(v['info']>>4!=0,v['table'],v['index']))
symbols=[dict(name='',value=0,size=0,info=0,other=0,section=0)];symbol_map={};symbol_manifest=[]
for old in selected:
 key=(old['table'],old['index']);outsec,outvalue=symbol_location.get(key,(0,old['value']))
 new=dict(name=old['name'],value=outvalue,size=old['size'],info=old['info'],other=old['other'],section=outsec)
 if old['section']==0:assert new['section']==0 and new['value']==new['size']==new['other']==0
 symbols.append(new);symbol_map[key]=len(symbols)-1;symbol_manifest.append(dict(Source_symbol=old,output_index=len(symbols)-1,output_symbol=new))
first_global=next(i for i,v in enumerate(symbols)if v['info']>>4!=0)
assert all(v['info']>>4==0 for v in symbols[:first_global])and all(v['info']>>4!=0 for v in symbols[first_global:])
allrefs=[]
for piece in pieces:
 refs=[v['row']for v in d['all_typed_relocations']if v['row']['source']==piece['Source_section']and piece['offset']<=v['row']['site']<piece['offset']+piece['bytes']]
 records=[]
 for row in refs:
  key=(row['symbol']['table'],row['symbol']['index']);assert key in symbol_map
  off=row['site']-piece['offset'];idx=symbol_map[key]
  records.append(struct.pack('>IIi',off,(idx<<8)|row['kind'],row['addend']))
  allrefs.append(dict(Source_row=row,output_code_section=piece['output_section'],output_site=off,output_symbol_index=idx,kind=row['kind'],addend=row['addend']))
 if records:
  section('.rela'+piece['output_name'],[0,4,0,0,0,len(records)*12,0,piece['output_section'],4,12],b''.join(records))
assert len(allrefs)==80
strtab=bytearray(b'\0');symraw=bytearray()
for sym in symbols:
 no=0
 if sym['name']:no=len(strtab);strtab.extend(sym['name'].encode()+b'\0')
 symraw.extend(struct.pack('>IIIBBH',no,sym['value'],sym['size'],sym['info'],sym['other'],sym['section']))
symtab_index=section('.symtab',[0,2,0,0,0,len(symraw),0,first_global,4,16],bytes(symraw))
strtab_index=section('.strtab',[0,3,0,0,0,len(strtab),0,0,1,0],bytes(strtab));headers[symtab_index][6]=strtab_index
for h in headers:
 if h[1]==4:h[6]=symtab_index
shstr=bytearray(b'\0')
for i,name in enumerate(names):
 if name:headers[i][0]=len(shstr);shstr.extend(name.encode()+b'\0')
shstr_name=len(shstr);shstr.extend(b'.shstrtab\0')
shstr_index=section('.shstrtab',[shstr_name,3,0,0,0,len(shstr),0,0,1,0],bytes(shstr));headers[shstr_index][0]=shstr_name
raw=bytearray((64+40*len(headers)+15)&~15);raw[:52]=source.read_bytes()[:52]
assert struct.unpack_from('>HH',raw,16)==(1,20)
struct.pack_into('>I',raw,32,64);struct.pack_into('>HH',raw,48,len(headers),shstr_index)
for i,h in enumerate(headers):
 if h[1]in(0,8):h[4]=0;continue
 align=max(1,h[8]);assert align&(align-1)==0
 at=(len(raw)+align-1)&-align;raw.extend(bytes(at-len(raw)));h[4]=at;h[5]=len(payloads[i]);raw.extend(payloads[i])
for i,h in enumerate(headers):struct.pack_into('>10I',raw,64+40*i,*h)
save('minimal-init-source.o',bytes(raw))
o=m.Raw(H/'minimal-init-source.o')
assert len(o.rows)==80 and len(o.sy)==34
assert [v['name']for v in o.sy if v['section']==0 and v['name']]==[v['name']for v in symbols if v['section']==0 and v['name']]
for piece in pieces:assert o.d[piece['output_section']]==s.d[piece['Source_section']][piece['offset']:piece['offset']+piece['bytes']]
for record in symbol_manifest:
 new=next(v for v in o.sy if v['table']==symtab_index and v['index']==record['output_index'])
 assert {k:new[k]for k in('name','value','size','info','other','section')}==record['output_symbol']
for a,b in zip(o.rows,allrefs):assert(a['source'],a['site'],a['kind'],a['addend'],a['symbol']['index'])==(b['output_code_section'],b['output_site'],b['kind'],b['addend'],b['output_symbol_index'])
for oldsec,newsec in section_map.items():assert o.h[newsec][1]==s.h[oldsec][1]and o.h[newsec][5]==s.h[oldsec][5]and o.d[newsec]==s.d[oldsec]
assert o.d[sdk_output]==s.d[212]and o.h[sdk_output][1:4]==s.h[212][1:4]
gfo=next(v for v in pieces if v['Source_section']==1 and v['bytes']==304)
word=int.from_bytes(o.d[gfo['output_section']][300:304],'big');disp=word&0x03fffffc;disp-=1<<26 if disp&0x02000000 else 0
assert 300+disp==0 and word==int.from_bytes(s.d[1][315956:315960],'big')
assert sum(h[5]for h in o.h if h[1]==1 and h[2]&6==6)==2740
assert sum(h[5]for h in o.h if h[1]==8)==916
save('extraction.json',dict(schema='current-ea1-minimal-initializer-static-ET_REL-extraction/1',passed=True,source=pin(source),discovery=pin(H/'discovery.json'),candidate=pin(H/'minimal-init-source.o'),extractor=pin(Path(__file__)),CODE_pieces=pieces,CODE_bytes=2740,selected_function_bodies=10,defined_FUNC_symbols=11,data_owner_definitions=data_sections,owner_bytes=1172,NOBITS_bytes=916,initialized_owner_PROGBITS_bytes=256,SDK_import_section=dict(Source_section=212,output_section=sdk_output,bytes=8264,kind=0x80000002,genuine_whole_section_retained=True,only_selected_SDK_symbol_roles=5,metadata_not_executable_CODE=True),symbols=symbol_manifest,symbol_count=34,external_UND_roles=list(external.values()),external_UND_count=4,typed_relocations=allrefs,typed_relocation_count=80,kind_counts=dict(__import__('collections').Counter(str(r['kind'])for r in o.rows)),local_GFO_tail=dict(output_section=gfo['output_section'],offset=300,target_offset=0,word=word,source_word_exact=True),all_selected_bodies_and_genuine_st_info_other_preserved=True,all_pointer_and_direct_function_dependencies_closed_or_explicit=True,no_native_addresses_or_physical_owner_aliases_created=True,Source_owner_definitions_require_future_borrow_omit_allocate_copy_clear=True,SDK5_and_external4_bindings_still_unselected=True,full_source_profiles_not_usable_on_extracted_section_symbol_layout=True,physical_serialization_SDK_context_locks_cold_lifetime_or_activation_proved=False,compiler_target_PPC_SDK_runtime_emulator_or_stopped_suite_execution=False,currentSource_Native_CP_repo_console_or_network_changes=False))
print(json.dumps(dict(candidate=pin(H/'minimal-init-source.o'),extraction=pin(H/'extraction.json'),sections=len(o.h),symbols=len(o.sy),relocations=len(o.rows)),indent=2))
