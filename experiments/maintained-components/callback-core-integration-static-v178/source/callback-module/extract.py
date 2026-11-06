"""Serialize only the actual callback core as an unselected logical ET_REL."""
from pathlib import Path
import ast,hashlib,json,struct,types,zlib
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
    raw=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest())
def save(name,obj):
    raw=obj if isinstance(obj,bytes)else(json.dumps(obj,indent=2)+'\n').encode()
    with(H/name).open('xb')as f:f.write(raw)
    assert(H/name).read_bytes()==raw
d=json.loads((H/'discovery.json').read_text())
assert pin(H/'discovery.json')['sha256']=='a5e9afd08f99fdae9d7005904bad3c88232935b70a1584c80ada7362f5d7d3c2'
assert(d['function_count'],d['CODE_bytes'],d['DATA_bytes'],d['typed_relocation_count'])==(23,7304,16060,199)
assert not d['external_UND']and not d['unhandled_direct_or_literal']and not d['indirect_branch_frontier']
for key in('source','Native','initializer_candidate','reader'):assert pin(B/d[key]['path'])==d[key]
source=B/d['source']['path'];reader=B/d['reader']['path']
node=next(v for v in ast.parse(reader.read_text()).body if isinstance(v,ast.ClassDef)and v.name=='Raw')
m=types.ModuleType('context_extract_Raw_only');m.__dict__.update(struct=struct,zlib=zlib);exec(compile(ast.Module(body=[node],type_ignores=[]),str(reader),'exec'),m.__dict__)
s=m.Raw(source);headers=[[0]*10];payloads=[b''];names=[''];locations={};pieces=[];owners=[]
def section(name,h,raw):
    out=len(headers);h=list(h);h[0]=h[4]=0;headers.append(h);payloads.append(raw);names.append(name);return out
for piece in d['CODE_slices']:
    sec,start,size=piece['Source_section'],piece['offset'],piece['bytes'];h=s.h[sec]
    assert h[1:4]==(1,6,0)and not h[2]&0x08000000
    out=section(s.names[sec],[0,1,6,0,0,size,0,0,h[8],0],s.d[sec][start:start+size]);pieces.append(piece|dict(output_section=out,output_name=s.names[sec]))
    for item in d['functions']:
        f=item['symbol']
        if f['section']==sec and start<=f['value']and f['value']+f['size']<=start+size:
            for alias in item['aliases']:locations[alias['table'],alias['index']]=(out,alias['value']-start)
for item in sorted(d['DATA_pieces'],key=lambda v:v['section']):
    sec=item['section'];h=s.h[sec];assert h[1]in(1,8)and h[2]==3 and h[3]==0 and not item['typed_rows']
    out=section(s.names[sec],h,s.d[sec]);owners.append(dict(Source_section=sec,output_section=out,name=s.names[sec],kind=h[1],bytes=h[5],logical_Source_definition_only=True))
    for f in item['aliases']:locations[f['table'],f['index']]=(out,f['value'])
sdksec=212;assert s.names[sdksec]=='.fimport_coreinit'and s.h[sdksec][1]==0x80000002 and len(s.d[sdksec])==8264
sdkout=section(s.names[sdksec],s.h[sdksec],s.d[sdksec]);sdk=[x['Source_symbol']for x in d['SDK_roles']];assert len(sdk)==6
for f in sdk:locations[f['table'],f['index']]=(sdkout,f['value'])
selected=[next(f for f in s.sy if(f['table'],f['index'])==key)for key in locations];assert len(selected)==58
selected.sort(key=lambda f:(f['info']>>4!=0,f['table'],f['index']))
symbols=[dict(name='',value=0,size=0,info=0,other=0,section=0)];mapping={};symbol_records=[]
for f in selected:
    sec,value=locations[f['table'],f['index']];new={k:f[k]for k in('name','size','info','other')}|dict(value=value,section=sec)
    mapping[f['table'],f['index']]=len(symbols);symbols.append(new);symbol_records.append(dict(Source_symbol=f,output_index=len(symbols)-1,output_symbol=new))
first_global=next(i for i,f in enumerate(symbols)if f['info']>>4);assert all(f['info']>>4==0 for f in symbols[:first_global])and all(f['info']>>4 for f in symbols[first_global:])
rows=[]
for piece in pieces:
    selected_rows=[x['row']for x in d['all_typed_relocations']if x['row']['source']==piece['Source_section']and piece['offset']<=x['row']['site']<piece['offset']+piece['bytes']]
    encoded=[]
    for row in selected_rows:
        key=(row['symbol']['table'],row['symbol']['index']);assert key in mapping
        site=row['site']-piece['offset'];index=mapping[key];encoded.append(struct.pack('>IIi',site,(index<<8)|row['kind'],row['addend']))
        rows.append(dict(Source_row=row,output_code_section=piece['output_section'],output_site=site,output_symbol_index=index,kind=row['kind'],addend=row['addend']))
    if encoded:section('.rela'+piece['output_name'],[0,4,0,0,0,len(encoded)*12,0,piece['output_section'],4,12],b''.join(encoded))
assert len(rows)==199
strings=bytearray(b'\0');symraw=bytearray()
for f in symbols:
    at=0
    if f['name']:at=len(strings);strings.extend(f['name'].encode()+b'\0')
    symraw.extend(struct.pack('>IIIBBH',at,f['value'],f['size'],f['info'],f['other'],f['section']))
symsec=section('.symtab',[0,2,0,0,0,len(symraw),0,first_global,4,16],bytes(symraw));strsec=section('.strtab',[0,3,0,0,0,len(strings),0,0,1,0],bytes(strings));headers[symsec][6]=strsec
for h in headers:
    if h[1]==4:h[6]=symsec
shstr=bytearray(b'\0')
for i,name in enumerate(names):
    if name:headers[i][0]=len(shstr);shstr.extend(name.encode()+b'\0')
name_at=len(shstr);shstr.extend(b'.shstrtab\0');shsec=section('.shstrtab',[0,3,0,0,0,len(shstr),0,0,1,0],bytes(shstr));headers[shsec][0]=name_at
raw=bytearray((64+len(headers)*40+15)&~15);raw[:52]=source.read_bytes()[:52];assert struct.unpack_from('>HH',raw,16)==(1,20)
struct.pack_into('>I',raw,32,64);struct.pack_into('>HH',raw,48,len(headers),shsec)
for i,h in enumerate(headers):
    if h[1]in(0,8):h[4]=0;continue
    align=max(1,h[8]);assert align&(align-1)==0;at=(len(raw)+align-1)&-align;raw.extend(bytes(at-len(raw)));h[4]=at;h[5]=len(payloads[i]);raw.extend(payloads[i])
for i,h in enumerate(headers):struct.pack_into('>10I',raw,64+i*40,*h)
save('context-callback-family.o',bytes(raw));o=m.Raw(H/'context-callback-family.o')
assert len(o.sy)==59 and len(o.rows)==199 and not[f for f in o.sy if f['name']and not f['section']]
for piece in pieces:assert o.d[piece['output_section']]==s.d[piece['Source_section']][piece['offset']:piece['offset']+piece['bytes']]
for item in symbol_records:
    out=next(f for f in o.sy if f['table']==symsec and f['index']==item['output_index']);assert{k:out[k]for k in('name','value','size','info','other','section')}==item['output_symbol']
for actual,expected in zip(o.rows,rows):assert(actual['source'],actual['site'],actual['kind'],actual['addend'],actual['symbol']['index'])==(expected['output_code_section'],expected['output_site'],expected['kind'],expected['addend'],expected['output_symbol_index'])
for x in owners:assert o.h[x['output_section']][1]==s.h[x['Source_section']][1]and o.h[x['output_section']][5]==x['bytes']and o.d[x['output_section']]==s.d[x['Source_section']]
assert o.d[sdkout]==s.d[sdksec]and o.h[sdkout][1:4]==s.h[sdksec][1:4]
local=[]
for edge in d['Source_internal_edges']:
    if 'row'in edge:continue
    p=next(p for p in pieces if p['Source_section']==edge['source_section']and p['offset']<=edge['source_site']<p['offset']+p['bytes']);target=edge['target']['value']
    assert p['offset']<=target<p['offset']+p['bytes'];site=edge['source_site']-p['offset'];word=int.from_bytes(o.d[p['output_section']][site:site+4],'big');assert word==edge['word']and word>>26==18 and not word&2
    disp=word&0x03fffffc;disp-=1<<26 if disp&(1<<25)else 0;assert site+disp==target-p['offset'];local.append(dict(output_section=p['output_section'],site=site,target=target-p['offset'],word=word,raw_Source_word_preserved=True))
assert len(local)==53
assert sum(h[5]for h in o.h if h[1]==1 and h[2]&6==6)==7304 and sum(h[5]for h in o.h if h[1]==8)==15548
save('extraction.json',dict(schema='current-ea1-context-callback-family-static-ET_REL-extraction/1',passed_within_Source_extraction_scope=True,source=pin(source),discovery=pin(H/'discovery.json'),candidate=pin(H/'context-callback-family.o'),extractor=pin(Path(__file__)),CODE_pieces=pieces,CODE_bytes=7304,selected_distinct_function_bodies=23,defined_FUNC_symbols=24,data_owner_definitions=owners,logical_owner_bytes=16060,NOBITS_bytes=15548,initialized_report_bytes=512,SDK_import_section=dict(Source_section=212,output_section=sdkout,bytes=8264,genuine_kind=0x80000002,genuine_whole_payload_retained=True,selected_SDK_roles=6,metadata_not_runtime_executable_CODE=True),symbols=symbol_records,symbol_count_including_null=59,external_UND_count=0,typed_relocations=rows,typed_relocation_count=199,kind_counts=dict(REL24=79,HA=57,LO=63,ADDR32=0),untyped_local_function_edges=local,untyped_local_function_edge_count=53,all_selected_bodies_and_genuine_st_info_st_other_preserved=True,Source478_layout_is_still_serial0_records4=True,no_Native_physical_alias_or_layout_selected=True,Native_sdk_OSWaitCond_role_unresolved=True,no_read_save_lifetime_or_world_service_included=True,workers_zero348_suffix_not_included=True,whole_Source_or_initializer_profiles_not_applicable=True,compiler_target_PPC_SDK_runtime_emulator_stopped_suite_or_Native_Source_CP_network_changes=False))
print(json.dumps(dict(candidate=pin(H/'context-callback-family.o'),extraction=pin(H/'extraction.json'),sections=len(o.h),symbols=len(o.sy),relocations=len(o.rows),local_edges=len(local)),indent=2))
