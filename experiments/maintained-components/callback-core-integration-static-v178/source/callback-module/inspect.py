"""Bounded static callback-family dependency inventory; never executes PPC."""
from pathlib import Path
import ast,collections,hashlib,json,struct,types,zlib
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
 b=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def save(name,obj):
 with(H/name).open('x',encoding='utf-8',newline='\n')as f:json.dump(obj,f,indent=2);f.write('\n')
S=R/'source714-native-cold-entry-join-r1/source-native-cold-entry-bound.o';N=R/'v178-current-features-test-r1/cking.rpx';I=R/'source714-minimal-native-init-candidate-r1/Native178-minimal-init-integration.NOT-FOR-INSTALL.bin';P=R/'ncb-kernel-scope-independent-r1/review.py'
for p,expected in((S,'ea1be3dfad78db219c6fb577012ff9931fc7bb4840d371422f822d1144ef8f57'),(N,'37ea3c790f607b95814ece1d5f176c2b5cb8981344ca5fb2747a0e2c426623ac'),(I,'2eadd54fc5f311266952da3ec8096f9c0fad9a6715d7c7dbe13b36581be9faeb'),(P,'515f9ba06da06d0136928f8e7a1e6e432ed91cc71cc9d83a9ce82e8fc6023bd1')):assert pin(p)['sha256']==expected
node=next(v for v in ast.parse(P.read_text()).body if isinstance(v,ast.ClassDef)and v.name=='Raw');m=types.ModuleType('context_family_Raw_only');m.__dict__.update(struct=struct,zlib=zlib);exec(compile(ast.Module(body=[node],type_ignores=[]),str(P),'exec'),m.__dict__)
s,n,i=m.Raw(S),m.Raw(N),m.Raw(I)
functions=[x for x in s.sy if x['info']&15==2 and x['size']and 0<x['section']<len(s.h)and s.h[x['section']][1]==1 and s.h[x['section']][2]&4]
def fid(x):return x['section'],x['value'],x['size']
def at(sec,value):return[x for x in functions if x['section']==sec and x['value']==value]
def sx(x,b):return x-(1<<b)if x&(1<<(b-1))else x
seeds=['_gz_jwa_enter','_gz_jwa_after','_gz_jwf_enter','_gz_jwf_after'];todo=[s.named(x)for x in seeds];seen={};rows=[];edges=[];data={};sdk=[];external=[];unhandled=[];indirect=[]
def follow(owner,r):
 rows.append(dict(owner=owner,row=r));f=r['symbol'];sec=f['section'];target=f['value']+r['addend']
 if not sec:external.append(dict(owner=owner,row=r));return
 if sec>=len(s.h):unhandled.append(dict(owner=owner,row=r,reason='reserved/ABS interface'));return
 h=s.h[sec]
 if h[1]==0x80000002:sdk.append(dict(owner=owner,row=r));return
 if h[2]&4:
  hits=at(sec,target)
  if hits:
   assert len({fid(x)for x in hits})==1;todo.append(hits[0]);edges.append(dict(owner=owner,row=r,target=hits[0],kind='typed defined Source function'))
  else:unhandled.append(dict(owner=owner,row=r,reason='CODE target not actual FUNC entry'))
 elif h[2]&2:
  if sec not in data:data[sec]=dict(section=sec,name=s.names[sec],header=list(h),bytes=h[5],aliases=[x for x in s.sy if x['section']==sec],typed_rows=[x for x in s.rows if x['source']==sec],incoming=[])
  data[sec]['incoming'].append(dict(owner=owner,row=r))
 else:unhandled.append(dict(owner=owner,row=r,reason='nonalloc dependency'))
while todo:
 f=todo.pop();key=fid(f)
 if key in seen:continue
 assert len(seen)<64,'callback core exceeded bounded64 function scope'
 seen[key]=dict(symbol=f,aliases=[x for x in functions if fid(x)==key],body_sha256=hashlib.sha256(s.body(f)).hexdigest(),section_name=s.names[f['section']])
 refs=s.refs(f)
 for r in refs:follow(f['name'],r)
 typed={r['site']for r in refs if r['kind']in(10,11,12,13)}
 for k,(word,)in enumerate(struct.iter_unpack('>I',s.body(f))):
  site=f['value']+k*4;op=word>>26
  if op in(16,18):
   if site in typed:continue
   if word&2:unhandled.append(dict(owner=f['name'],site=site,word=word,reason='untyped absolute branch'));continue
   target=site+sx(word&(0x03fffffc if op==18 else 0xfffc),26 if op==18 else 16)
   if f['value']<=target<f['value']+f['size']:continue
   hits=at(f['section'],target)
   if hits:
    assert len({fid(x)for x in hits})==1;todo.append(hits[0]);edges.append(dict(owner=f['name'],source_section=f['section'],source_site=site,word=word,target=hits[0],kind='untyped local direct function edge'))
   else:unhandled.append(dict(owner=f['name'],section=f['section'],site=site,word=word,target=target,reason='local target lacks exact FUNC'))
  elif op==19 and word>>1&1023==528:indirect.append(dict(owner=f['name'],site=site,word=word))
assert not any(x['typed_rows']for x in data.values()),'pointer-bearing owner needs explicit followup'
pieces=[]
for sec in sorted({x[0]for x in seen}):
 spans=sorted((x[1],x[1]+x[2])for x in seen if x[0]==sec);merged=[]
 for begin,end in spans:
  if merged and begin<=merged[-1][1]:merged[-1][1]=max(merged[-1][1],end)
  else:merged.append([begin,end])
 for a,z in merged:pieces.append(dict(Source_section=sec,Source_name=s.names[sec],offset=a,bytes=z-a,whole_original_section=a==0 and z==len(s.d[sec]),original_section_bytes=len(s.d[sec]),alignment=s.h[sec][8],sha256=hashlib.sha256(s.d[sec][a:z]).hexdigest()))
sdk_roles=[]
for name in sorted({r['row']['symbol']['name']for r in sdk}):
 ss=s.named(name);sdk_roles.append(dict(name=name,Source_symbol=ss,Source_import_section=s.names[ss['section']],Native_named_matches=[x for x in n.sy if x['name']==name],initializer_candidate_named_matches=[x for x in i.sy if x['name']==name],uses=[r for r in sdk if r['row']['symbol']['name']==name]))
all_consumer_names={}
for sec in data:
 consumers=[]
 for r in s.rows:
  if r['symbol']['section']!=sec:continue
  names=[x for x in functions if x['section']==r['source']and x['value']<=r['site']<x['value']+x['size']]
  consumers.append(dict(row=r,containing_functions=names))
 all_consumer_names[str(sec)]=consumers
result=dict(schema='current-ea1-callback-context-family-static-discovery/1',source=pin(S),Native=pin(N),initializer_candidate=pin(I),reader=pin(P),seeds=seeds,functions=list(seen.values()),function_count=len(seen),CODE_slices=pieces,CODE_bytes=sum(x['bytes']for x in pieces),typed_relocation_count=len(rows),kind_counts=dict(collections.Counter(str(x['row']['kind'])for x in rows)),all_typed_relocations=rows,Source_internal_edges=edges,DATA_pieces=list(data.values()),DATA_bytes=sum(x['bytes']for x in data.values()),SDK_roles=sdk_roles,external_UND=external,unhandled_direct_or_literal=unhandled,indirect_branch_frontier=indirect,all_full_Source_consumers_of_selected_owners=all_consumer_names,no_runtime_or_physical_activation=True)
save('discovery.json',result)
print(json.dumps(dict(discovery=pin(H/'discovery.json'),function_count=len(seen),functions=[dict(name=x['symbol']['name'],section=x['symbol']['section'],offset=x['symbol']['value'],bytes=x['symbol']['size'])for x in seen.values()],CODE_pieces=pieces,CODE_bytes=result['CODE_bytes'],DATA=[dict(section=x['section'],name=x['name'],bytes=x['bytes'])for x in data.values()],DATA_bytes=result['DATA_bytes'],relocations=len(rows),SDK=[dict(name=x['name'],Native_matches=len(x['Native_named_matches']))for x in sdk_roles],external=external,unhandled=unhandled,indirect=indirect),indent=2))
