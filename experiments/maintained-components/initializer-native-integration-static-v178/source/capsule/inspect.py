"""Inventory only the current714 initializer's actual static dependency closure."""
from pathlib import Path
import ast,hashlib,json,struct,types,zlib
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
SOURCE=R/'source714-native-cold-entry-join-r1/source-native-cold-entry-bound.o';NATIVE=R/'v178-current-features-test-r1/cking.rpx'
def pin(p):
 b=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
assert pin(SOURCE)['sha256']=='ea1be3dfad78db219c6fb577012ff9931fc7bb4840d371422f822d1144ef8f57'
assert pin(NATIVE)['sha256']=='37ea3c790f607b95814ece1d5f176c2b5cb8981344ca5fb2747a0e2c426623ac'
reader=R/'ncb-kernel-scope-independent-r1/review.py';assert pin(reader)['sha256']=='515f9ba06da06d0136928f8e7a1e6e432ed91cc71cc9d83a9ce82e8fc6023bd1'
node=next(v for v in ast.parse(reader.read_text()).body if isinstance(v,ast.ClassDef)and v.name=='Raw')
m=types.ModuleType('init_slice_Raw_only');m.__dict__.update(struct=struct,zlib=zlib);exec(compile(ast.Module(body=[node],type_ignores=[]),str(reader),'exec'),m.__dict__)
s,n=m.Raw(SOURCE),m.Raw(NATIVE)
seeds=['_gz_jwa_common_after_native_frame','_gz_jwa_init_for_native_jwt','_gz_jwa_init_after_native_frame','_gz_jwa_init_open','_gz_jwa_init_cold_for_native_entry']
funcs=[f for f in s.sy if f['info']&15==2 and f['size']and 0<f['section']<len(s.h)and s.h[f['section']][1]==1 and s.h[f['section']][2]&4]
def fid(f):return(f['section'],f['value'],f['size'])
def at(section,value):return[f for f in funcs if f['section']==section and f['value']==value]
def signed(v,b):return v-(1<<b)if v&(1<<(b-1))else v
todo=[s.named(v)for v in seeds];seen={};edges=[];rows=[];external=[];sdk=[];data={};unhandled=[];indirect=[]
def typed(owner,row):
 symbol=row['symbol'];sec=symbol['section'];target=symbol['value']+row['addend'];record=dict(owner=owner,row=row)
 rows.append(record)
 if sec==0:external.append(record);return
 if sec>=len(s.h):unhandled.append(record|dict(reason='ABS/reserved Source section role needs explicit literal treatment'));return
 if s.h[sec][1]==0x80000002:
  sdk.append(record);return
 if s.h[sec][2]&4:
  targets=at(sec,target)
  if targets:
   assert len({fid(f)for f in targets})==1
   todo.append(targets[0]);edges.append(record|dict(target=targets[0],edge='typed Source function expression'))
  else:unhandled.append(record|dict(reason='code target is not a defined FUNC entry'))
 elif s.h[sec][2]&2:
  if sec not in data:
   data[sec]=dict(section=sec,name=s.names[sec],header=list(s.h[sec]),bytes=s.h[sec][5],aliases=[v for v in s.sy if v['section']==sec],typed_rows=[v for v in s.rows if v['source']==sec])
  data[sec].setdefault('incoming',[]).append(record)
 else:unhandled.append(record|dict(reason='nonallocated target requires explicit treatment'))
while todo:
 f=todo.pop();key=fid(f)
 if key in seen:continue
 assert len(seen)<32,'Bounded initializer inventory unexpectedly expanded'
 aliases=[v for v in funcs if fid(v)==key];seen[key]=dict(symbol=f,aliases=aliases,section_name=s.names[f['section']],body_sha256=hashlib.sha256(s.body(f)).hexdigest())
 refs=s.refs(f)
 for row in refs:typed(f['name'],row)
 typed_sites={r['site']for r in refs if r['kind']in(10,11,12,13)}
 for offset,w in enumerate(struct.iter_unpack('>I',s.body(f))):
  offset*=4;w=w[0];op=w>>26;site=f['value']+offset
  if op==18 or op==16:
   if site in typed_sites:continue
   if w&2:unhandled.append(dict(owner=f['name'],site=site,word=w,reason='untyped absolute branch'));continue
   dest=site+signed(w&(0x03fffffc if op==18 else 0xfffc),26 if op==18 else 16)
   if f['value']<=dest<f['value']+f['size']:continue
   targets=at(f['section'],dest)
   if targets:
    assert len({fid(v)for v in targets})==1
    todo.append(targets[0]);edges.append(dict(owner=f['name'],source_section=f['section'],source_site=site,target=targets[0],word=w,edge='local untyped direct branch'))
   else:unhandled.append(dict(owner=f['name'],source_section=f['section'],source_site=site,target_offset=dest,word=w,reason='local direct target not a defined FUNC entry'))
  elif op==19 and w>>1&1023==528:indirect.append(dict(owner=f['name'],section=f['section'],site=site,word=w))
# Initializer-owned data must be pointer-free or its typed dependencies followed.
assert not any(v['typed_rows']for v in data.values()),'Data pointer closure requires explicit continuation'
code_slices=[]
for sec in sorted({k[0]for k in seen}):
 intervals=sorted((k[1],k[1]+k[2])for k in seen if k[0]==sec);merged=[]
 for begin,end in intervals:
  if merged and begin<=merged[-1][1]:merged[-1][1]=max(merged[-1][1],end)
  else:merged.append([begin,end])
 for begin,end in merged:
  code_slices.append(dict(Source_section=sec,Source_name=s.names[sec],offset=begin,bytes=end-begin,whole_original_section=begin==0 and end==len(s.d[sec]),original_section_bytes=len(s.d[sec]),alignment=s.h[sec][8],sha256=hashlib.sha256(s.d[sec][begin:end]).hexdigest()))
sdk_roles=[]
for name in sorted({r['row']['symbol']['name']for r in sdk}):
 source=s.named(name);matches=[v for v in n.sy if v['name']==name]
 sdk_roles.append(dict(name=name,Source_symbol=source,Source_import_section=s.names[source['section']],Native_exact_name_matches=matches,uses=[v for v in sdk if v['row']['symbol']['name']==name],binding_qualified=False))
report=dict(schema='Current714-ea1-initializer-static-dependency-slice-discovery/1',source=pin(SOURCE),Native=pin(NATIVE),reader=pin(reader),seeds=seeds,functions=list(seen.values()),function_count=len(seen),CODE_slices=code_slices,CODE_bytes=sum(v['bytes']for v in code_slices),Source_internal_edges=edges,all_typed_relocations=rows,typed_relocation_count=len(rows),SDK_roles=sdk_roles,external_UND=external,DATA_pieces=list(data.values()),DATA_bytes=sum(v['bytes']for v in data.values()),unhandled_direct_or_literal=unhandled,indirect_branch_frontier=indirect,no_binary_output_mapping_binding_or_activation=True)
(H/'discovery.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(dict(discovery=pin(H/'discovery.json'),functions=[dict(name=v['symbol']['name'],section=v['symbol']['section'],value=v['symbol']['value'],bytes=v['symbol']['size'])for v in seen.values()],CODE_slices=code_slices,CODE_bytes=report['CODE_bytes'],DATA_bytes=report['DATA_bytes'],SDK=[dict(name=v['name'],Native_matches=len(v['Native_exact_name_matches']))for v in sdk_roles],external=sorted({v['row']['symbol']['name']for v in external}),unhandled=unhandled,indirect=indirect),indent=2))
