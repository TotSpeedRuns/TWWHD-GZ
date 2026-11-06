"""Static initializer+callback-core Native review candidate. NOT FOR INSTALL."""
from pathlib import Path
import hashlib,importlib.util,json,struct,sys
sys.dont_write_bytecode=True
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def need(x,msg):
    if not x:raise ValueError(msg)
def sha(raw):return hashlib.sha256(raw).hexdigest()
def pin(p):
    raw=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(raw),sha256=sha(raw))
def checked(p,want):
    raw=p.read_bytes();need(sha(raw)==want,'exact input '+str(p));return raw
def read(p):return json.loads(p.read_text())
def save(name,obj):
    raw=obj if isinstance(obj,bytes)else(json.dumps(obj,indent=2)+'\n').encode()
    with(H/name).open('xb')as f:f.write(raw)
    need((H/name).read_bytes()==raw,'saved readback '+name)
def module(name,p):
    spec=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(spec);sys.modules[name]=m;spec.loader.exec_module(m);return m
def align(v,a):return(v+a-1)&-a
def apart(a,b):return a[1]<=b[0]or b[1]<=a[0]
def branch(at,target,word):
    need(word>>26==18 and not word&2,'relative B/BL preimage');delta=target-at
    need(delta%4==0 and -(1<<25)<=delta<(1<<25),'REL24 range/alignment');return(word&0xfc000003)|(delta&0x03fffffc)
def branch_target(at,word):
    v=word&0x03fffffc;v-=1<<26 if v&(1<<25)else 0;return at+v

S=R/'source714-context-family-slice-r1';M=R/'source714-wait-sdk-metadata-r1';A=R/'source714-minimal-native-init-candidate-r1';F=R/'source714-native-invocation-cold-family-r2'
SOURCE=S/'context-callback-family.o';PARENT=M/'Native178-minimal-init-OSWaitCond-metadata.NOT-FOR-INSTALL.bin';INIT=A/'Native178-minimal-init-integration.NOT-FOR-INSTALL.bin'
checked(SOURCE,'b5ee5b828e5b9997cf8add9501529fc72af1a52617b5040da09301b8dce1508d')
checked(S/'qualification.json','1947ad675261e706664c4aa05aba06ddbb98c5c615c0dda7707b9bf687642537')
checked(S/'extraction.json','faa82775d71b3ed78d40a1660b828f9aefa8c77356b1033b5bf039e6189d6d41')
checked(S/'source-use-proof.json','f7d10971b047ab7744a4236794fecf4c80e4a90ab48bea7e545a2a8bb41a0436')
checked(R/'source714-context-family-slice-independent-r1/qualification.json','2001ff5b4df282e8328827848fa701b8f83e84e3333c35cfa9b24c7289b52dee')
checked(F/'qualification.json','c92a443726a51350b849786390e2531f882af196c3e73979edd152030f3939cb')
checked(F/'report.json','7c52bdb66f7b2868638f42cfb2e1c934aaca1a9c904444d6a9e0008a266f2462')
checked(INIT,'2eadd54fc5f311266952da3ec8096f9c0fad9a6715d7c7dbe13b36581be9faeb')
raw=checked(PARENT,'4039d880c4f0f6fa83f1bef3aa54e64e3fb27bd93c15a3ba135fada3a9917fee')
checked(M/'qualification.json','7b74d0d590c7a57f47481160d1d2f9b2aa6d0fcf8987ebc0e1c7863412bbd994')
codec_path=B/'work/TWWHD-GZ/patcher/native_rpx.py';reader_path=R/'source714-init-sdk-metadata-r1/elf_static.py'
checked(codec_path,'52c566d12e8bb4b2182c2a1de93883e0eb40fa9db7ff2fba49c83cfe9b635cb5');checked(reader_path,'fdb9b76b6150bc161bdb9815f4abdfa07238ae070d91e31edda21ec3d5a65ceb')
codec=module('callback_core_native_rpx',codec_path);reader=module('callback_core_static_ELF',reader_path)
s,n,init=reader.ELF(SOURCE),reader.ELF(PARENT),reader.ELF(INIT);e=codec.Elf(raw);q=read(S/'extraction.json');use=read(S/'source-use-proof.json');native=read(F/'report.json')
need(len(n.headers)==51 and len(n.symbols)==5026 and len(n.data[29])==9994,'actual WaitCond successor shape')
need(len(s.rows)==199 and len(s.symbols)==59 and len(q['CODE_pieces'])==14,'actual independently reviewed module')
need(n.named('OSWaitCond')==dict(name='OSWaitCond',value=0xc000a8c0,size=0,info=18,other=0,section=25,table=28,index=5025),'genuine qualified WaitCond import')
need(n.data[25]==s.data[23]and len(n.data[25])==8264,'one genuine import payload, no duplicate Source payload')
need(not[f for f in n.symbols.values()if f['section']in(28,29,30)]and not[r for r in n.rows if r['symbol']['section']in(28,29,30)],'no existing declared/typed metadata-address targets')
need(n.data[31]==init.data[31]and n.data[46]==init.data[46]and n.data[37]==init.data[37]and n.data[47]==init.data[47],'SDK metadata predecessor preserves initialized code/relocations')
need(n.headers[46][1:4]==(1,6,0x02cbe600)and len(n.data[46])==138540,'real retained CODE46 boundary')
code=bytearray(n.data[46]);positions={};pieces=[];sampler=n.named('_gz_jwf_atomic_load')
need(sampler['section']==46 and sampler['info']==18 and sampler['size']==28 and sampler['other']==0,'real prior owned Source sampler')
need(not[r for r in n.rows if r['source']==46 and sampler['value']<=r['site']<sampler['value']+28],'sampler has no unresolved operand')
for item in q['CODE_pieces']:
    sec=item['output_section'];body=s.data[sec];need(s.headers[sec][1:4]==(1,6,0),'actual Source code definition')
    reused=item['Source_section']==56
    if reused:
        need(len(body)==28 and n.data[46][sampler['value']-n.headers[46][3]:sampler['value']-n.headers[46][3]+28]==body,'sampler byte/extent equality')
        base=sampler['value'];off=base-n.headers[46][3]
    else:
        at=align(len(code),max(4,s.headers[sec][8]));code.extend(bytes(at-len(code)));off=at;base=n.headers[46][3]+at;code.extend(body)
        for i,h in enumerate(n.headers):
            if h[1]in(1,8)and h[2]&2 and h[3]and h[5]:
                size=h[5]if h[1]==8 else len(n.data[i]);need(apart((base,base+len(body)),(h[3],h[3]+size)),'new code outside every retained real span')
    need(0x02000000<=base<base+len(body)<0x10000000 and base%max(4,s.headers[sec][8])==0,'actual Source code domain/alignment')
    positions[sec]=(base,off);pieces.append(dict(capsule_section=sec,Source_section=item['Source_section'],Source_offset=item['offset'],preferred=base,CODE46_offset=off,bytes=len(body),Source_body_sha256=sha(body),reused_existing_owned_sampler=reused))
need(sum(x['bytes']for x in pieces if not x['reused_existing_owned_sampler'])==7276 and len(code)-len(n.data[46])==7276,'append only7276 CODE, zero extra alignment')

symraw=bytearray(n.data[28]);strings=bytearray(n.data[29]);functions=[];function_bind={}
for key,f in s.symbols.items():
    if f['section']not in positions or f['info']!=18:continue
    need(f['size']and f['other']==0,'genuine defined global Source FUNC')
    base,_=positions[f['section']];existing=[x for x in n.symbols.values()if x['name']==f['name']]
    if existing:
        need(f['name']=='_gz_jwf_atomic_load'and len(existing)==1 and existing[0]==sampler and base+f['value']==sampler['value'],'only exact genuine sampler reuse')
        function_bind[key]=sampler['index'];continue
    index=len(symraw)//16;st_name=len(strings);strings.extend(f['name'].encode()+b'\0');value=base+f['value']
    target=dict(name=f['name'],value=value,size=f['size'],info=18,other=f['other'],section=46,table=28,index=index)
    symraw.extend(struct.pack('>IIIBBH',st_name,value,f['size'],18,f['other'],46));function_bind[key]=index;functions.append(dict(Source_symbol=f,Native_symbol=target))
need(len(functions)==18 and len(strings)==10357,'all18 truthful new aliases/names retained')
e.sections[28].data=bytes(symraw);e.sections[29].data=bytes(strings)
# New names exceed the original STR VMA gap by37 bytes. No old symbol or
# typed target refers to metadata sections; retain st_name and all old raw
# records while placing SYM,SHSTR,STR in nonoverlapping loader storage.
symend=n.headers[28][3]+len(symraw);shstr_at=align(symend,n.headers[30][8]);str_at=align(shstr_at+len(n.data[30]),4)
e.sections[30].header[3]=shstr_at;e.sections[29].header[3]=str_at
need(str_at>=symend and str_at>=shstr_at+len(n.data[30])and str_at%4==0,'STR moved after complete grown SYM+SHSTR')

sdk_bind={}
for f in s.symbols.values():
    if f['section']!=23:continue
    target=n.named(f['name']);need(f['info']==target['info']==18 and f['other']==target['other']==f['size']==target['size']==0 and target['section']==25,'real named same-module SDK FUNC')
    off=target['value']-n.headers[25][3];need(off==f['value']and s.data[23][off:off+8]==n.data[25][off:off+8]==bytes(8),'genuine SDK slot identity')
    sdk_bind[f['table'],f['index']]=target
need(len(sdk_bind)==6,'all six true named SDK roles resolved')
owners={x['output_section']:x['Source_section']for x in q['data_owner_definitions']}
affine={487:(4973,0,872),484:(4973,872,44),483:(4973,916,4),485:(4973,920,7168),480:(4973,8088,288),411:(4974,0,256),409:(4979,0,256)}
for mapping in native['mappings']:
    target=n.symbols[mapping['Native_symbol']['table'],mapping['Native_symbol']['index']];need(target==mapping['Native_symbol'],'genuine Native context coordinate')
def native_body(sec,at,z):
    h=n.headers[sec];need(h[1]==1 and h[2]&3==3 and h[3]<=at and at+z<=h[3]+len(n.data[sec]),'complete writable native owner extent');return n.data[sec][at-h[3]:at-h[3]+z]
for capsec,oldsec in owners.items():
    if oldsec==478:
        need(s.headers[capsec][1]==8 and s.headers[capsec][5]==7172 and native_body(34,0x127b47f4,7172)==bytes(7172),'fresh native JWF pool+serial complete, not copied')
    else:
        index,delta,size=affine[oldsec];target=n.symbols[28,index];body=native_body(target['section'],target['value']+delta,size)
        need(s.headers[capsec][5]==size,'exact source owner extent')
        need(body==(bytes(size)if s.headers[capsec][1]==8 else s.data[capsec]),'fresh native owner/report representation exact')
need(native_body(32,0x10c07df4,128)[:4]==b'JOS1','intervening JOS1 excluded')
new_by_index={x['Native_symbol']['index']:x['Native_symbol']for x in functions}
def resolve(row):
    f=row['symbol'];key=(f['table'],f['index']);offset=f['value']+row['addend']
    if key in sdk_bind:return sdk_bind[key],row['addend'],'genuine named SDK import'
    if key in function_bind:
        idx=function_bind[key];target=n.symbols.get((28,idx))or new_by_index[idx];return target,row['addend'],'owned Source function (sampler reused)'if idx==sampler['index']else'new owned Source function'
    need(f['section']in owners,'no unhandled Source/interface target')
    sec=owners[f['section']]
    if sec==478:
        need((row['source'],row['site'])in{(x['output_code_section'],x['output_site'])for x in q['typed_relocations']if x['Source_row']['symbol']['section']==478},'only qualified split-owner fields')
        need(offset in(0,4,7172),'strict serial/base/end-only split expression')
        if offset==7172:need(row['source']==1 and row['site']in(2134,2146),'only authenticated comparison sentinel')
        delta=0 if offset==4 else 7168;role='JWF record base'if offset==4 else'JWF serial'if offset==0 else'JWF comparison-only one-past sentinel'
        return n.symbols[28,4980],delta,role
    idx,delta,size=affine[sec];need(0<=offset<size,'bounded affine owner target');return n.symbols[28,idx],delta+offset,'borrowed coherent Native owner'

fields=[];relas=[];occupied=[]
for row in s.rows:
    need(row['source']in positions and row['kind']in(4,6,10),'only qualified core relocation sections/types')
    base,off=positions[row['source']];at=base+row['site'];field=off+row['site'];target,add,role=resolve(row);address=target['value']+add;size=4 if row['kind']==10 else 2
    span=(at,at+size);need(all(apart(span,x)for x in occupied),'all199 field spans disjoint');occupied.append(span)
    before=bytes(code[field:field+size]);need(len(before)==size,'complete actual relocation field')
    if row['kind']==10:
        word=int.from_bytes(before,'big');need(word>>26==18 and not word&2,'typed B/BL AA0/LK preserved')
        if role=='genuine named SDK import':need(word&0x03fffffc==0,'canonical SDK loader call placeholder');after=before
        else:after=branch(at,address,word).to_bytes(4,'big')
    else:
        need(row['site']%4==2 and target['section']in(32,34)and 0x10000000<=address<0xc0000000,'aligned genuine Native DATA target')
        half=((address+0x8000)>>16)&0xffff if row['kind']==6 else address&0xffff;after=half.to_bytes(2,'big')
    code[field:field+size]=after;relas.append(struct.pack('>IIi',at,(target['index']<<8)|row['kind'],add))
    fields.append(dict(capsule_row=row,Native_source_section=46,preferred_site=at,CODE46_offset=field,bytes=size,before=before.hex(),after=after.hex(),Native_symbol=target,Native_addend=add,kind=row['kind'],role=role,SDK_implementation_value_resolved_by_loader_only=role=='genuine named SDK import'))
need(len(fields)==199 and sum(x['role']=='genuine named SDK import'for x in fields)==28,'all199 expressions/28 SDK edges')
e.sections[46].data=bytes(code);e.sections[47].data=n.data[47]+b''.join(relas)
by_name={x['Native_symbol']['name']:x['Native_symbol']for x in functions};patches=[];patchrel=[]
for at,name,oldtarget,oldword in((0x02b141e4,'_gz_jwa_enter',0x02b13300,0x4bfff11d),(0x02b15620,'_gz_jwa_after',0x02b13778,0x4bffe159),(0x02b15c1c,'_gz_jwf_enter',0x02b17374,0x48001759),(0x02b16440,'_gz_jwf_after',0x02b176fc,0x480012bd)):
    index,section,off=e.section_at(at);word=int.from_bytes(section.data[off:off+4],'big')
    need(index==31 and word==oldword and word&1 and branch_target(at,word)==oldtarget,'exact supported Native callback BL')
    need(not[r for r in n.rows if r['source']==31 and at<=r['site']<at+4],'no original callback relocation collision')
    target=by_name[name];after=branch(at,target['value'],word);e.write_word(at,after);patchrel.append(struct.pack('>IIi',at,(target['index']<<8)|10,0));patches.append(dict(site=at,before=word,after=after,original_target=oldtarget,Source_target=target,link=1,relative_AA=0))
e.sections[37].data=n.data[37]+b''.join(patchrel)
fi=bytearray(e.sections[50].data);old_tramp=struct.unpack_from('>I',fi,72)[0];need(old_tramp==432,'incoming initializer trampoline budget')
struct.pack_into('>I',fi,72,460);e.sections[50].data=bytes(fi);old_reserve=e.text_trampoline_reserve;need(old_reserve>=4096,'preserve incoming actual posttrampoline reserve')
e.text_trampoline_reserve=old_reserve+28*16-4096
artifact=e.encode_rpx();o_codec=codec.Elf(artifact);o_codec.verify_crcs()
name='Native178-initializer-and-callback-core.NOT-FOR-INSTALL.bin';save(name,artifact);o=reader.ELF(H/name)

need(len(o.headers)==51 and len(o.symbols)==5044,'51indices and all18 real new function aliases')
need(o.data[28][:len(n.data[28])]==n.data[28]and o.data[29][:len(n.data[29])]==n.data[29],'raw old SYM/st_other/st_name/STR prefix retained')
need(all(o.symbols[k]==f for k,f in n.symbols.items()),'all prior named/unnamed symbol records/roles retained')
for x in functions:need(o.symbols[28,x['Native_symbol']['index']]==x['Native_symbol'],'new real owned Source FUNC extent/role')
need(o.data[46][:len(n.data[46])]==n.data[46],'entire initializer/prior gameplay CODE46 prefix unchanged')
for piece in pieces:
    expect=bytearray(s.data[piece['capsule_section']])
    for f in fields:
        if f['capsule_row']['source']==piece['capsule_section']:at=f['capsule_row']['site'];expect[at:at+f['bytes']]=bytes.fromhex(f['after'])
    actual=o.data[46][piece['CODE46_offset']:piece['CODE46_offset']+piece['bytes']];need(actual==bytes(expect),'each actual Source body differs only at qualified typed operands')
local=[]
for edge in q['untyped_local_function_edges']:
    base,off=positions[edge['output_section']];word=int.from_bytes(o.data[46][off+edge['site']:off+edge['site']+4],'big')
    need(word==edge['word']and branch_target(base+edge['site'],word)==base+edge['target'],'all53 private local edges remain exact');local.append(edge|dict(Native_site=base+edge['site'],Native_target=base+edge['target']))
expected31=bytearray(n.data[31])
for p in patches:struct.pack_into('>I',expected31,p['site']-n.headers[31][3],p['after'])
need(o.data[31]==bytes(expected31),'exactly four additional Native words; all rewrite/kind/request/probe gates retained')
need(o.data[37]==n.data[37]+b''.join(patchrel)and o.data[47]==n.data[47]+b''.join(relas),'all old relocations exact prefix, 203 actual new records')
for i in range(51):
    if i not in(28,29,31,37,46,47,49,50):need(o.data[i]==n.data[i],'all other decoded payloads retained '+str(i))
need(o.data[32]==init.data[32]and o.data[34]==init.data[34]and o.headers[6]==n.headers[6]and o.headers[33]==n.headers[33],'ordinary game DATA/BSS/record/report/cached owner bytes unchanged')
need(o.data[25]==n.data[25]and o.data[30]==n.data[30],'import and SHSTR bytes exact')
actual_rows=[r for r in o.rows if r['source']==46 and r['site']>=n.headers[46][3]+len(n.data[46])]
need(len(actual_rows)==199,'actual runtime records for all selected core fields')
for a,b in zip(actual_rows,fields):need((a['site'],a['kind'],a['addend'],a['symbol'])==(b['preferred_site'],b['kind'],b['Native_addend'],b['Native_symbol']),'exact actual symbolic relocation receiver')
mapped=[]
for i,h in enumerate(o.headers):
    if h[2]&2 and h[3]and h[5]:
        a,b=h[3],h[3]+(h[5]if h[1]==8 else len(o.data[i]));need(all(apart((a,b),(x['begin'],x['end']))for x in mapped),'all ordinary/import/nonordinary loader spans disjoint');mapped.append(dict(section=i,name=o.names[i],kind=h[1],begin=a,end=b))
fi=o.data[50];oldfi=n.data[50];pad=struct.unpack_from('>I',fi,76)[0];load=struct.unpack_from('>I',fi,20)[0]
need(pad==136 and struct.unpack_from('>I',fi,24)[0]==4,'true loader pad136/alignment4')
need(all(x['end']<=0xc0000000+load-pad for x in mapped if x['begin']>=0xc0000000),'all loader spans within usable declared bounds')
textend=max(h[3]+len(o.data[i])for i,h in enumerate(o.headers)if h[1]==1 and h[2]&6==6);ta=struct.unpack_from('>I',fi,8)[0];tb=struct.unpack_from('>I',fi,32)[0];text=struct.unpack_from('>I',fi,4)[0]
need(text==align(textend-0x02000000,ta)+tb+old_reserve+448 and struct.unpack_from('>I',fi,72)[0]==460,'exact prior reserve plus28x16, no unexplained4096')
need(text-tb-align(textend-0x02000000,16)>=4096+448,'minimum trailing capacity')
need(o.headers[29][3]==str_at and o.headers[30][3]==shstr_at and o.headers[28][3]==n.headers[28][3],'actual metadata VMA plan, no old symbol rebasing')
changed=[i for i in range(51)if o.data[i]!=n.data[i]];need(changed==[28,29,31,37,46,47,49,50],'complete decoded change set')
headers=[dict(section=i,field=j,before=x,after=y)for i,(aa,bb)in enumerate(zip(n.headers,o.headers))for j,(x,y)in enumerate(zip(aa,bb))if x!=y]
save('integration.json',dict(schema='Native178-init-plus-callback-core-NOT-FOR-INSTALL-static-integration/1',passed_within_static_serialization_scope=True,artifact=pin(H/name),metadata_parent=pin(PARENT),initializer_parent=pin(INIT),Source_module=pin(SOURCE),module_qualification=pin(S/'qualification.json'),module_peer=pin(R/'source714-context-family-slice-independent-r1/qualification.json'),Native_family_r2=pin(F/'qualification.json'),source_split_use_proof=pin(S/'source-use-proof.json'),CODE_pieces=pieces,new_CODE_bytes=7276,alignment_growth=0,existing_owned_sampler_reused=sampler,new_global_Source_FUNC_definitions=functions,private_helpers_retained_only_as_local_edges=True,selected_fields=fields,selected_field_count=199,SDK_loader_REL24_fields=28,new_RELA47_bytes=2388,new_RELA37_bytes=48,Native_callback_word_changes=patches,untyped_local_edges=local,logical_Source_owners=q['data_owner_definitions'],Native_context_mapping=native['mappings'],comparison_sentinel_mapping=native['JWF_one_past_pool_expression'],all16060_ordinary_owner_bytes_borrowed_no_allocation_copy_or_clear=True,JOS1_and_all_initialization_GFO_frame_cache_and_original_clear_paths_unchanged=True,Source_workers_zero_read_save_lifetime_world_services_not_included=True,all_old_Native_callback_bodies_retained_except_their_four_external_supported_callers=True,LR_rewrite_and_kind2_6_gates_literal_unchanged=True,old_SYM_st_name_st_other_and_RELA_prefixes_literal_exact=True,all_51_indices_retained=True,decoded_changed_sections=changed,header_changes=headers,ordinary_Game_DATA_BSS_growth_bytes=0,metadata_layout=dict(old_STR_preferred=n.headers[29][3],abandoned_old_STR_VMA_span=[n.headers[29][3],n.headers[29][3]+len(n.data[29])],old_STR_bytes=len(n.data[29]),new_STR_preferred=str_at,new_STR_bytes=len(strings),new_SYM_bytes=len(symraw),new_SHSTR_preferred=shstr_at,old_metadata_symbols_and_typed_targets=[],all_st_name_offsets_preserved=True,loader_metadata_allocation_growth=load-struct.unpack_from('>I',oldfi,20)[0]),FileInfo=dict(before_hex=oldfi.hex(),after_hex=fi.hex(),old_posttrampoline_reserve=old_reserve,additional_SDK_reserve=448,encoder4096_compensated=True,trampAddition_before=432,trampAddition_after=460,textSize=text,text_actual_end=textend,loadSize=load,loadReservedPad=pad,loadAlign=4,tempSize=struct.unpack_from('>I',fi,28)[0]),all_mapped_spans=mapped,all_CRCs_and_bounds_checked=True,SDK_import_targets_metadata_not_runtime_VMA=True,record_order_is_disjoint_field_permutation=True,installable_release=False,hardware_runtime_or_read_save_authority=False,no_compiler_target_PPC_SDK_fixture_emulator_runtime_stopped_suite_console_network_CP_Source_or_existing_Native_changes=True))
print(json.dumps(dict(artifact=pin(H/name),integration=pin(H/'integration.json'),new_CODE_bytes=7276,reused_sampler_bytes=28,selected_fields=199,SDK_loader_edges=28,callback_words=4,loader_metadata_allocation_growth=load-struct.unpack_from('>I',oldfi,20)[0],terminal_exit=0),indent=2))
