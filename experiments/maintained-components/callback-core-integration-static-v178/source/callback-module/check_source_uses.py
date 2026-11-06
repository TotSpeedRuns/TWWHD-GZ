"""Check exact scalar pointer construction in this fixed Source snapshot only."""
from pathlib import Path
import ast,hashlib,json,struct,types,zlib
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
 b=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def save(name,v):
 with(H/name).open('x',encoding='utf-8',newline='\n')as f:json.dump(v,f,indent=2);f.write('\n')
d=json.loads((H/'discovery.json').read_text());p=B/d['reader']['path'];assert pin(p)==d['reader'];source=B/d['source']['path'];assert pin(source)==d['source']
node=next(x for x in ast.parse(p.read_text()).body if isinstance(x,ast.ClassDef)and x.name=='Raw');m=types.ModuleType('context_split_Raw_only');m.__dict__.update(struct=struct,zlib=zlib);exec(compile(ast.Module(body=[node],type_ignores=[]),str(p),'exec'),m.__dict__);s=m.Raw(source)
hub=R/'source714-native-invocation-cold-family-r2/report.json';hubq=R/'source714-native-invocation-cold-family-r2/qualification.json'
assert pin(hub)['sha256']=='7c52bdb66f7b2868638f42cfb2e1c934aaca1a9c904444d6a9e0008a266f2462'and pin(hubq)['sha256']=='c92a443726a51350b849786390e2531f882af196c3e73979edd152030f3939cb'
native=json.loads(hub.read_text());typed={(r['source'],r['site']):r for r in s.rows};W=lambda sec,off:int.from_bytes(s.d[sec][off:off+4],'big')
def pair(sec,hi,lo,offset,reg):
 a,b=typed[sec,hi],typed[sec,lo];assert(a['kind'],b['kind'])==(6,4)and a['symbol']==b['symbol']and a['symbol']['section']==478 and a['symbol']['value']+a['addend']==b['symbol']['value']+b['addend']==offset
 hw,lw=W(sec,hi&~3),W(sec,lo&~3);assert hw==0x3c000000|(reg<<21)and lw==0x38000000|(reg<<21)|(reg<<16)
 return dict(section=sec,HA_site=hi,LO_site=lo,Source_offset=offset,destination_GPR=reg,HA_word=hw,LO_word=lw)
def words(sec,items):
 out=[]
 for off,w,meaning in items:assert W(sec,off)==w,(sec,off,hex(W(sec,off)),hex(w));out.append(dict(section=sec,offset=off,word=w,meaning=meaning))
 return out
checks=[]
checks.append(dict(method='_gz_jwf_enter',record_base=pair(17,634,638,4,31),record_loop_words=words(17,[(648,0x3b9f1c00,'end=r31+7168 in r28'),(668,0x3bff0038,'advance one56-byte record'),(676,0x7f9cf840,'compare cursor with independent record end')]),serial_base=pair(17,706,714,0,3),serial_use='r3 goes directly to atomic_issue at720; only the maximum scalar argument is completed in between'))
checks.append(dict(method='_gz_jwf_after',record_base=pair(17,1338,1346,4,31),record_loop_words=words(17,[(1416,0x3bdf1c00,'end=r31+7168 in r30'),(1484,0x3bff0038,'advance one56-byte record'),(1488,0x7c1ff040,'compare cursor against end')]),serial_base=None))
checks.append(dict(method='_gz_jwf_cold_jwa_enter',record_base=pair(17,2090,2094,4,31),explicit_one_past=pair(17,2134,2146,7172,9),record_loop_words=words(17,[(2140,0x3bff0038,'advance one56-byte record'),(2148,0x7e9f4840,'r9 is only the one-past comparator here')]),serial_base=pair(17,2178,2186,0,3),serial_use='r3 goes directly to atomic_issue at2192; no serial-base+4 record derivation'))
checks.append(dict(method='_gz_jwf_cold_jwa_conflict',record_base=pair(17,2586,2590,4,31),record_loop_words=words(17,[(2596,0x3bdf1c00,'end=r31+7168 in r30'),(2612,0x3bff0038,'advance one56-byte record'),(2620,0x7f9ff040,'compare cursor against end')]),serial_base=None))
checks.append(dict(method='_gz_jwf_cold_jwa_after',record_base=pair(17,2802,2806,4,31),record_loop_words=words(17,[(2860,0x3bdf1c00,'end=r31+7168 in r30'),(2932,0x3bff0038,'advance one56-byte record'),(2936,0x7c1ff040,'compare cursor against end')]),serial_base=None))
for site in(720,2192):assert typed[17,site]['kind']==10 and typed[17,site]['symbol']['name']=='_gz_jwf_atomic_issue'
core_rows=[r for r in d['all_typed_relocations']if r['row']['symbol']['section']==478];assert len(core_rows)==16 and{r['row']['symbol']['value']+r['row']['addend']for r in core_rows}=={0,4,7172}
assert {r['owner']for r in core_rows}=={x['method']for x in checks}
ghs_record=pair(37,1786,1798,4,5);ghs_serial=pair(37,1822,1834,0,5)
ghs_words=words(37,[(1792,0x38c01c00,'GHS exclusion record range bytes7168'),(1828,0x38c00004,'separate GHS exclusion serial range bytes4')])
apart=s.named('ghsj_apart')
for site in(1804,1840):
 row=typed[37,site];assert row['kind']==10 and row['symbol']['section']==apart['section']and row['symbol']['value']+row['addend']==apart['value']
full478=[r for r in s.rows if r['symbol']['section']==478];assert len(full478)==21
assert {(r['source'],r['site'])for r in full478}=={(r['row']['source'],r['row']['site'])for r in core_rows}|{(37,x)for x in(1786,1798,1822,1834)}|{(387,224)}
startup=typed[387,224];assert startup['kind']==1 and startup['symbol']['value']+startup['addend']==0 and int.from_bytes(s.d[387][228:232],'big')==1793
for name,sec,value,z in(('jwf_serial',478,0,4),('jwf_invocation',478,4,7168),('jwa_serial',483,0,4),('jwa_invocation',485,0,7168),('_gz_jwa_first',480,0,288),('_gz_jwf_report',409,0,256)):
 f=s.named(name);assert(f['section'],f['value'],f['size'])==(sec,value,z)
save('source-use-proof.json',dict(schema='current-ea1-context-owner-split-address-use-static-evidence/1',passed_within_fixed_Source_word_and_declared_method_scope=True,source=d['source'],Native_family_report=pin(hub),Native_family_qualification=pin(hubq),all_five_callback_methods=checks,all16_core_Source478_relocations=core_rows,Source478_representation=dict(serial=[0,4],records=[4,7172],record_count=128,record_stride=56),required_Native_projection=dict(serial='Native4980+7168..7172',records='Source4..7172 -> Native4980+0..7168',explicit_one_past='Source7172 -> Native4980+7168, comparison sentinel only; do not dereference or classify as Source serial'),no_selected_record_base_is_derived_from_serial_base=True,record_ends_derive_from_record_base_or_explicit_sentinel_only=True,normal_ABI_preservation_and_authored_record_helpers_required=True,one_extra_GHS_reader=dict(method='_gz_jwa_ghs_private_apart',record_range_base=ghs_record,serial_range_base=ghs_serial,range_size_words=ghs_words,separate_pure_scalar_calls=[1804,1840],not_a_coalesced_whole7172_range=True,method_not_in_callback_ET_REL=True),all_full_Source478_typed_refs_classified=21,startup_descriptor=dict(index=28,pointer_site=224,count_site=228,word_count=1793,bytes=7172,must_omit_when_borrowing=True,not_in_extracted_module=True),Native_mappings_copied_from_pinned_role_report=native['mappings'],layout=dict(Invocation_bytes=56,control=0,serial=4,thread=8,incoming_SP=12,caller_LR=16,kind=20,depth=24,admission=28,token_offset=32,SharedToken_bytes=24,token_fields=['Gate','thread','slot','generation','depth','active'],token_field_offsets=[0,4,8,12,16,20],first_rows=9,first_row_bytes=32,serial_max=0x1fffffff,phase_bits=3,phases=dict(FREE=0,RESERVED=1,LIVE=2,RETIRING=3,QUARANTINED=4)),actual_machine_words_and_target_roles_checked=True,Native_Source_body_equivalence_or_physical_binding_claim=False,no_target_runtime_emulator_compiler_SDK_stopped_suite_or_frozen_input_changes=True))
print(json.dumps(dict(evidence=pin(H/'source-use-proof.json'),Source478_core_refs=16,all_full_Source478_refs=21,record_methods=5,GHS_independent_ranges=2,Native_projection_not_applied=True),indent=2))
