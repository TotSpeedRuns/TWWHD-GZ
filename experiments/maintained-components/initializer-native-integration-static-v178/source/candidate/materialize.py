"""Static, NOT-FOR-INSTALL Native178 initializer integration serialization.

No target execution, SDK calls, installer or stopped composition is imported.
"""
from pathlib import Path
import argparse,hashlib,importlib.util,json,struct,sys
sys.dont_write_bytecode=True
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def need(x,m):
    if not x:raise ValueError(m)
def sha(b):return hashlib.sha256(b).hexdigest()
def pin(p):
    b=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(b),sha256=sha(b))
def checked(p,want):
    need(pin(p)['sha256']==want,'exact input '+str(p));return p.read_bytes()
def document(p):return json.loads(p.read_text())
def save(n,v):
    b=v if isinstance(v,bytes)else(json.dumps(v,indent=2)+'\n').encode()
    with(H/n).open('xb')as f:f.write(b)
    need((H/n).read_bytes()==b,'saved readback '+n)
def module(name,p):
    spec=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(spec);sys.modules[name]=m;spec.loader.exec_module(m);return m
def aligned(x,a):return(x+a-1)&-a
def disjoint(a,b):return a[1]<=b[0]or b[1]<=a[0]
def branch(at,target,word):
    need(word>>26==18 and not word&2,'relative branch opcode/AA')
    d=target-at;need(d%4==0 and -(1<<25)<=d<(1<<25),'REL24 reachable')
    return(word&0xfc000003)|(d&0x03fffffc)
def entry_target(at,w):
    d=w&0x03fffffc;d-=1<<26 if d&(1<<25)else 0;return at+d

ap=argparse.ArgumentParser();ap.add_argument('--capsule-peer',type=Path,required=True);ap.add_argument('--capsule-peer-sha',required=True);ap.add_argument('--lifetime-report',type=Path,required=True);ap.add_argument('--lifetime-report-sha',required=True);args=ap.parse_args();args.capsule_peer=args.capsule_peer.resolve();args.lifetime_report=args.lifetime_report.resolve()
need(args.capsule_peer.is_relative_to(B)and args.lifetime_report.is_relative_to(B),'review inputs remain workspace evidence')
checked(args.capsule_peer,args.capsule_peer_sha);checked(args.lifetime_report,args.lifetime_report_sha)
peer=document(args.capsule_peer);need(peer.get('frozen')is True,'frozen capsule peer receipt')
need(any(v is True for k,v in peer.items()if k.startswith('passed')),'capsule peer passed in its static scope')

S=R/'source714-ea1-init-integration-slice-r1';M=R/'source714-init-sdk-metadata-r1';C=R/'source714-init-sdk-role-r1'
capsule=checked(S/'minimal-init-source.o','425a49616e2c63ccdd5b436b168205fb3d0a294cadb34d850a5f6c1ec7d71023')
checked(S/'qualification.json','2c8743a4ce66e91abdee34ac6b43468d36f80ebe80375b8d1cfe217fd2777fcb')
checked(S/'extraction.json','05607e0085ca0d195a6b50385f17ade6f031e6672769bc8a4c8146c9e99ddcc4')
baseline=checked(R/'v178-current-features-test-r1/cking.rpx','37ea3c790f607b95814ece1d5f176c2b5cb8981344ca5fb2747a0e2c426623ac')
metadata=checked(M/'Native178-OSInitCond-metadata-only.NOT-FOR-INSTALL.bin','c883ed0d4c4a0ccc1e5a394fda0e928ec44bbc1a3937f65fb3d238389f2fc3d5')
checked(C/'contract.json','e67ee3612693717cbba6080fa3107e2b75efad339507f6334c682a59cb2b5731')
checked(R/'source714-init-sdk-metadata-independent-r2/qualification.json','5a4e33130781144e579867c4222e72c780877e393e68d11e961c52e24ffbc071')
checked(R/'source714-native-jwa-owner-closure-r1/binder-contract.json','9f5ae96193c2e25c93d5a97e3c0adac718c6a5a4d33303160a086cbe571c2bc9')
codec_path=B/'work/TWWHD-GZ/patcher/native_rpx.py';checked(codec_path,'52c566d12e8bb4b2182c2a1de93883e0eb40fa9db7ff2fba49c83cfe9b635cb5')
reader_path=M/'elf_static.py';checked(reader_path,'fdb9b76b6150bc161bdb9815f4abdfa07238ae070d91e31edda21ec3d5a65ceb')
codec=module('minimal_initializer_native_rpx',codec_path);reader=module('minimal_initializer_data_ELF',reader_path)
q=document(S/'extraction.json');contract=document(C/'contract.json');owners=document(R/'source714-native-jwa-owner-closure-r1/binder-contract.json')
s=reader.ELF(S/'minimal-init-source.o');old=reader.ELF(R/'v178-current-features-test-r1/cking.rpx');n=reader.ELF(M/'Native178-OSInitCond-metadata-only.NOT-FOR-INSTALL.bin');e=codec.Elf(metadata)
need(len(n.headers)==len(e.sections)==51 and len(s.rows)==80 and len(s.symbols)==34,'exact source/native shapes')
need(n.data[25]==s.data[13]and n.data[25][8:17]==b'coreinit\0','genuine import payload identical, never copied again')
need(len(n.data[46])==135780 and n.headers[46][1:4]==(1,6,0x02cbe600),'append only real CODE46')
need(n.headers[28][6]==29 and n.headers[28][9]==16 and len(n.symbols)==5015,'real symbol namespace')
need(n.symbols[28,9]==dict(name='.bss',value=0x101fce00,size=3003592,info=3,other=0,section=6,table=28,index=9),'OriginalGame BSS anchor')
for idx,section,value in((4959,34,0x127b20f4),(4973,34,0x127b26e8),(4974,32,0x10c07cf4),(4976,31,0x02b15424)):
    need(n.symbols[28,idx]==dict(name='',value=value,size=0,info=16,other=0,section=section,table=28,index=idx),'genuine Native coordinate '+str(idx))
def native_span(at,z):
    matches=[n.data[i][at-h[3]:at-h[3]+z]for i,h in enumerate(n.headers)if h[1]==1 and h[3]and h[3]<=at and at+z<=h[3]+len(n.data[i])]
    need(len(matches)==1,'one real complete Native span');return matches[0]
need(native_span(0x127b26e8,916)==bytes(916)and native_span(0x127b20f4,32)==bytes(32),'serialized fresh owner/GFO bytes')
need(native_span(0x10c07cf4,256)==s.data[12]and len(s.data[12])==256,'identical initialized report256')
need(native_span(0x10c07df4,4)==b'JOS1','adjacent JOS1 excluded')

# Physical placement exists ONLY in this private review candidate. It grants no
# cold lifetime, readiness or permission to run it.
code=bytearray(n.data[46]);piece_map=[];positions={}
for piece in q['CODE_pieces']:
    sec=piece['output_section'];need(s.headers[sec][1:4]==(1,6,0),'owned Source executable piece')
    align=max(4,s.headers[sec][8]);offset=aligned(len(code),align);code.extend(bytes(offset-len(code)));base=n.headers[46][3]+offset
    need(0x02000000<=base<base+len(s.data[sec])<0x10000000,'CODE domain')
    code.extend(s.data[sec]);positions[sec]=(base,offset)
    piece_map.append(dict(capsule_section=sec,Source_section=piece['Source_section'],Source_offset=piece['offset'],preferred=base,CODE46_offset=offset,bytes=len(s.data[sec]),alignment=align,body_sha256=sha(s.data[sec])))
need(sum(p['bytes']for p in piece_map)==2740,'exact selected CODE size')
for p in piece_map:
    for i,h in enumerate(n.headers):
        if h[1]in(1,8)and h[2]&2 and h[3]and h[5]:
            count=h[5]if h[1]==8 else len(n.data[i]);need(disjoint((p['preferred'],p['preferred']+p['bytes']),(h[3],h[3]+count)),'selected CODE outside every retained real Native span')

# Declare only genuine newly-emitted global Source functions. The private
# gfo_match remains a local in-piece tail target; no Native FUNC is fabricated.
symraw=bytearray(n.data[28]);strings=bytearray(n.data[29]);new_functions=[];func_bind={}
for key,f in s.symbols.items():
    if f['section']not in positions or f['info']!=18:continue
    need(f['size']>0 and f['other']==0 and not[x for x in n.symbols.values()if x['name']==f['name']],'unique real owned Source FUNC')
    base,_=positions[f['section']];value=base+f['value'];idx=len(symraw)//16;name_at=len(strings);strings.extend(f['name'].encode()+b'\0')
    symraw.extend(struct.pack('>IIIBBH',name_at,value,f['size'],f['info'],f['other'],46));func_bind[key]=(idx,0)
    new_functions.append(dict(capsule_symbol=f,Native_symbol=dict(name=f['name'],value=value,size=f['size'],info=18,other=0,section=46,table=28,index=idx)))
need(len(new_functions)==10,'ten genuine global Source function definitions (one real shared-body alias)')
need(len(strings)<=0xc00043c0-n.headers[29][3],'expanded STR remains below actual first import')
e.sections[28].data=bytes(symraw);e.sections[29].data=bytes(strings)
symend=n.headers[28][3]+len(symraw);e.sections[30].header[3]=aligned(symend,n.headers[30][8])

sdk_bind={}
for role in contract['roles']:
    ss=s.named(role['name']);ns=n.named(role['name']);need(ss['info']==ns['info']==18 and ss['section']==13 and ns['section']==25 and ss['size']==ns['size']==ss['other']==ns['other']==0,'real named SDK role')
    need(ns==role['Native_symbol'],'qualified exact Native SDK declaration')
    need(s.data[13][ss['value']:ss['value']+8]==n.data[25][ns['value']-n.headers[25][3]:ns['value']-n.headers[25][3]+8]==bytes(8),'genuine existing SDK slots')
    sdk_bind[ss['table'],ss['index']]=(ns['index'],0)
need(len(sdk_bind)==5,'five true SDK role mappings')
owner_bind={10:(4973,0,872),11:(4973,872,44),12:(4974,0,256)}
external={'_gz_pbac_frame':(9,0x2758),'_gz_gfo_native_invocation':(4959,0),'_gz_gfo_native_after_return_adapter':(4976,-35124),'_gz_jwa_native_sdk_prepare':(4976,6604)}
need(n.symbols[28,9]['value']+0x2758==0x101ff558,'genuine scalar frame address')
need(n.symbols[28,4976]['value']-35124==0x02b0caf0 and n.symbols[28,4976]['value']+6604==0x02b16df0,'genuine adapter/prepare coordinates, not fake Native FUNC')
def resolve(f):
    key=(f['table'],f['index'])
    if key in func_bind:return(*func_bind[key],'new owned Source FUNC')
    if key in sdk_bind:return(*sdk_bind[key],'true SDK import metadata')
    if f['section']in owner_bind:
        idx,delta,z=owner_bind[f['section']];need(0<=f['value']<=z and f['value']+f['size']<=z,'complete owner alias extent');return idx,delta+f['value'],'borrowed Native owner'
    if f['section']==0:
        need(f['name']in external and f['value']==f['size']==f['other']==0 and f['info']==(17 if f['name']in('_gz_pbac_frame','_gz_gfo_native_invocation')else 18),'exact external Source role')
        return(*external[f['name']],'genuine Native external role')
    raise ValueError('unhandled capsule typed target '+str(f))

rows=[];fields=[];used=[]
for row in s.rows:
    need(row['source']in positions and row['kind']in(4,6,10),'selected exact relocation kinds/sections')
    base,off=positions[row['source']];at=base+row['site'];index,delta,role=resolve(row['symbol']);add=delta+row['addend'];need(-(1<<31)<=add<(1<<31),'signed RELA addend')
    # New function definitions are not in the parent Native symbol dictionary.
    target_symbol=n.symbols.get((28,index))or next(x['Native_symbol']for x in new_functions if x['Native_symbol']['index']==index)
    target=target_symbol['value']+add;field=off+row['site'];width=4 if row['kind']==10 else 2
    span=(at,at+width);need(all(disjoint(span,x)for x in used),'disjoint selected relocation fields');used.append(span)
    before=bytes(code[field:field+width]);need(len(before)==width,'complete relocation field')
    if row['kind']==10:
        word=int.from_bytes(before,'big');need(word>>26==18 and not word&2,'REL24 preserves B/BL with AA0')
        if role=='true SDK import metadata':
            need(word&0x03fffffc==0,'SDK call retains zero-displacement loader placeholder');after=before
        else:after=branch(at,target,word).to_bytes(4,'big')
    else:
        need(row['site']%4==2 and target_symbol['section']in(6,32,34),'aligned real DATA halfword target')
        need(0x10000000<=target<0xc0000000,'actual DATA role domain')
        value=((target+0x8000)>>16)&0xffff if row['kind']==6 else target&0xffff;after=value.to_bytes(2,'big')
    code[field:field+width]=after
    encoded=struct.pack('>IIi',at,(index<<8)|row['kind'],add);rows.append(encoded)
    fields.append(dict(capsule_row=row,Native_source_section=46,preferred_site=at,CODE46_offset=field,bytes=width,before=before.hex(),after=after.hex(),Native_symbol=target_symbol,Native_addend=add,kind=row['kind'],role=role,SDK_runtime_value_left_to_native_loader=role=='true SDK import metadata'))
need(len(rows)==80 and sum(x['role']=='true SDK import metadata'for x in fields)==24,'all80 fields and24 SDK loader edges')
e.sections[46].data=bytes(code);e.sections[47].data=n.data[47]+b''.join(rows)

patches=[];patch_relas=[];fn={x['Native_symbol']['name']:x['Native_symbol']for x in new_functions}
for at,name,oldword,link in((0x02b0cfcc,'_gz_jwa_common_after_native_frame',0x4bfffb24,0),(0x02b0d004,'_gz_jwa_common_after_native_frame',0x4bfffaed,1),(0x02b0fdcc,'_gz_jwa_init_for_native_jwt',0x48002fa1,1),(0x02b12d6c,'_gz_jwa_init_cold_for_native_entry',0x9421ffe8,0)):
    i,section,offset=e.section_at(at);need(i==31 and int.from_bytes(section.data[offset:offset+4],'big')==oldword,'exact four Native preimages')
    need(not[r for r in n.rows if r['source']==31 and at<=r['site']<at+4],'no preexisting Native patch relocation collision')
    target=fn[name];word=branch(at,target['value'],0x48000000|link);e.write_word(at,word)
    patch_relas.append(struct.pack('>IIi',at,(target['index']<<8)|10,0));patches.append(dict(site=at,Native_source_section=31,before=oldword,after=word,target=target,link=link,relative_AA=0,bytes=4))
e.sections[37].data=n.data[37]+b''.join(patch_relas)

# Exact retained native-RPX policy: preserve original trailing reserve and
# add16B per new SDK REL24. Compensate the encoder's unconditional4096B so
# its generic policy cannot silently add an unexplained second reserve.
fi=bytearray(e.sections[50].data);need(struct.unpack_from('>I',fi,72)[0]==408,'actual initial trampoline count')
struct.pack_into('>I',fi,72,432);e.sections[50].data=bytes(fi)
old_reserve=e.text_trampoline_reserve;need(old_reserve>=4096,'retained post-trampoline reserve')
e.text_trampoline_reserve=old_reserve+24*16-4096
artifact=e.encode_rpx();reopened=codec.Elf(artifact);reopened.verify_crcs()
need(len(reopened.sections)==51 and reopened.header[:32]==codec.Elf(metadata).header[:32],'unchanged entry/header namespace')
output=H/'Native178-minimal-init-integration.NOT-FOR-INSTALL.bin';need(not output.exists(),'separate noninstallable artifact')
save(output.name,artifact);o=reader.ELF(output)

# Complete exact decoded diff and independently derived symbolic row/body
# reconstruction; never execute selected code or the SDK import loader.
need(len(o.symbols)==5025 and o.data[28][:len(n.data[28])]==n.data[28]and o.data[29][:len(n.data[29])]==n.data[29],'all old symbol/string bytes exact prefix')
need(all(o.symbols[k]==v for k,v in n.symbols.items()),'all old Native symbol roles unchanged')
for x in new_functions:need(o.symbols[28,x['Native_symbol']['index']]==x['Native_symbol'],'truthful actual owned Source FUNC definition')
for p in piece_map:
    body=o.data[46][p['CODE46_offset']:p['CODE46_offset']+p['bytes']];expected=bytearray(s.data[p['capsule_section']])
    for f in fields:
        if f['capsule_row']['source']==p['capsule_section']:expected[f['capsule_row']['site']:f['capsule_row']['site']+f['bytes']]=bytes.fromhex(f['after'])
    need(body==bytes(expected),'each selected body differs only by all actual typed fields')
gfo=next(p for p in piece_map if p['capsule_section']==2);tail=int.from_bytes(o.data[46][gfo['CODE46_offset']+300:gfo['CODE46_offset']+304],'big')
need(entry_target(gfo['preferred']+300,tail)==gfo['preferred']and tail==int.from_bytes(s.data[2][300:304],'big'),'untyped local GFO tail retained')
need(o.data[46][:len(n.data[46])]==n.data[46],'all old CODE46 literal prefix')
expected31=bytearray(n.data[31])
for p in patches:struct.pack_into('>I',expected31,p['site']-n.headers[31][3],p['after'])
need(o.data[31]==bytes(expected31),'only four original Native code words changed')
need(o.data[37]==n.data[37]+b''.join(patch_relas)and o.data[47]==n.data[47]+b''.join(rows),'all old/new RELA record bytes exact')
for i in range(51):
    if i not in(28,29,31,37,46,47,49,50):need(o.data[i]==n.data[i],'all other Native payload bytes unchanged '+str(i))
need(o.data[32]==old.data[32]and o.data[34]==old.data[34]and o.headers[6]==n.headers[6]and o.headers[33]==n.headers[33],'no DATA/owner/report/GFO/BSS allocation or clear changes')
need(o.data[25]==old.data[25]and o.data[13]==old.data[13],'genuine import payloads unchanged')
need(o.data[30]==old.data[30]and o.headers[30][3]==symend,'SHSTR moved only beyond actual grown SYM')
actual_rows=[x for x in o.rows if x['source']==46 and n.headers[46][3]+len(n.data[46])<=x['site']]
need(len(actual_rows)==80,'actual80 new runtime RELA records')
for x,y in zip(actual_rows,fields):need((x['site'],x['kind'],x['addend'],x['symbol'])==(y['preferred_site'],y['kind'],y['Native_addend'],y['Native_symbol']),'actual runtime target expression')
text=[(h[3],h[3]+len(o.data[i]))for i,h in enumerate(o.headers)if h[1]==1 and h[2]&6==6]
textend=max(x[1]for x in text);fi=o.data[50];textsize=struct.unpack_from('>I',fi,4)[0];loadsize=struct.unpack_from('>I',fi,20)[0];pad=struct.unpack_from('>I',fi,76)[0]
text_alignment=struct.unpack_from('>I',fi,8)[0];base_adjust=struct.unpack_from('>I',fi,32)[0]
need(pad==136 and struct.unpack_from('>I',fi,72)[0]==432 and textsize==aligned(textend-0x02000000,text_alignment)+base_adjust+old_reserve+384,'exact original trailing TEXT reserve plus384')
need(textsize-base_adjust-aligned(textend-0x02000000,16)>=4096+384,'retained minimum post-trampoline TEXT capacity')
mapped=[]
for i,h in enumerate(o.headers):
    if h[2]&2 and h[3]and h[5]:
        a,b=h[3],h[3]+(h[5]if h[1]==8 else len(o.data[i]));need(all(disjoint((a,b),(x['begin'],x['end']))for x in mapped),'all real/import/metadata mapped spans disjoint');mapped.append(dict(section=i,begin=a,end=b))
need(all(x['end']<=0xc0000000+loadsize-pad for x in mapped if x['begin']>=0xc0000000),'all loader metadata within usable FileInfo loadSize minus136')
changed_payloads=[i for i in range(51)if o.data[i]!=old.data[i]]
need(changed_payloads==[28,29,31,37,46,47,49,50],'complete exact decoded payload change set')
header_diffs=[dict(section=i,field=j,before=a,after=b)for i,(aa,bb)in enumerate(zip(old.headers,o.headers))for j,(a,b)in enumerate(zip(aa,bb))if a!=b]
def payload_pin(i,data):return dict(section=i,name=o.names[i],bytes=len(data),sha256=sha(data))
save('integration.json',dict(schema='Native178-minimal-initializer-NOT-FOR-INSTALL-static-integration/1',passed_within_static_serialization_scope=True,installable_release=False,artifact=pin(output),baseline=pin(R/'v178-current-features-test-r1/cking.rpx'),metadata_input=pin(M/'Native178-OSInitCond-metadata-only.NOT-FOR-INSTALL.bin'),capsule=pin(S/'minimal-init-source.o'),capsule_peer=pin(args.capsule_peer),practical_lifetime_and_order_report=pin(args.lifetime_report),CODE_pieces=piece_map,owned_global_FUNC_definitions=new_functions,private_local_gfo_match_not_fabricated_as_Native_FUNC=True,new_CODE_bytes=2740,CODE46_growth_including_alignment=len(code)-len(n.data[46]),duplicate_Source_owner_storage_import_payload_and_startup_all_omitted=True,borrowed_owner_mapping=owners['mapping'],borrowed916_and_report256_literal_bytes_preserved=True,JOS1_and_GFO32_and_frame_preserved=True,selected_typed_fields=fields,selected_typed_field_count=80,SDK_loader_REL24_fields=24,additional_Native_entry_REL24_records=4,Native_word_changes=patches,all_other_Native_gameplay_request_probe_gates_literal_exact=True,new_RELA47_bytes=len(b''.join(rows)),new_RELA37_bytes=len(b''.join(patch_relas)),local_GFO_tail_exact=True,header_changes=header_diffs,decoded_changed_sections=changed_payloads,all_decoded_sections=[payload_pin(i,x)for i,x in enumerate(o.data)],all_old_RELA_and_symbols_literal_prefix=True,FileInfo=dict(before_hex=old.data[50].hex(),after_hex=fi.hex(),text_declared=textsize,text_actual_end=textend,old_posttramp_reserve=old_reserve,added_SDK_reserve=384,encoder_4096_compensated=True,unexplained_extra_encoder_reserve=0,trampAddition_before=408,trampAddition_after=432,loadSize=loadsize,loadReservedPad=pad,loadAlign=struct.unpack_from('>I',fi,24)[0],tempSize=struct.unpack_from('>I',fi,28)[0]),all_mapped_spans=mapped,SDK_import_targets_are_metadata_not_runtime_implementation_VMAs=True,all_section_CRCs_and_FileInfo_bounds_checked=True,physical_review_layout_selected_only_in_noninstallable_artifact=True,cold_before_first_use_all_consumer_alias_lock_context_lifetime_Native_cache_readiness_and_activation_proved=False,no_target_SDK_PPC_CPU_emulator_runtime_or_stopped_composition_execution=True,no_input_currentSource_Native_repo_CP_console_network_or_version_package_mutation=True))
print(json.dumps(dict(artifact=pin(output),integration=pin(H/'integration.json'),new_CODE_bytes=2740,CODE46_growth=len(code)-len(n.data[46]),selected_fields=80,SDK_loader_edges=24,Native_words=4,terminal_exit=0),indent=2))
