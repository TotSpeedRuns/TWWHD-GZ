"""Strict authored-component rebuild for the partial experimental v173 test.

No game, recovery object/image, captured memory, substitute SDK or native
implementation is an input. Optional generated capacity text is separately
identified and remains private. All genuine runtime interfaces stay unresolved.
"""
from pathlib import Path, PurePosixPath
import argparse,hashlib,importlib.util,json,os,struct,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
REL='experiments/maintained-components/rando-mode-v173'
COMP=ROOT/REL
SPEC_SHA='52b891117e143a6352c981dba58872de5db562f1f617d1673d9a7d03fee0a5a2'
def need(v,m):
 if not v:raise ValueError(m)
def sha(b):return hashlib.sha256(b).hexdigest()
def save(p,j):
 b=(json.dumps(j,indent=2)+'\n').encode()
 with p.open('xb')as f:f.write(b);f.flush();os.fsync(f.fileno())
 need(p.read_bytes()==b,'saved report readback')
def safe(root,relative):
 q=PurePosixPath(relative);need(type(relative)is str and '\\'not in relative and not q.is_absolute()and not any(x in('.','..')for x in q.parts),'contained POSIX input path')
 original=root/relative;p=original.resolve(strict=True);need(p.is_relative_to(root),'input escaped declared root')
 need(not any(x.is_symlink()for x in[original,*original.parents]if x.is_relative_to(root)),'symlink input refused');return p
def load_module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);sys.modules[name]=m;s.loader.exec_module(m);return m
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('--compiler',type=Path,required=True);ap.add_argument('--sdk-include',type=Path,required=True);ap.add_argument('--output',type=Path,required=True)
 ap.add_argument('--reference-repository',type=Path,default=ROOT,help='Existing pinned renderer171 snapshot/public parser; defaults to this repository')
 ap.add_argument('--capacity-source',type=Path,help='Private output of generate_practice173_capacity.py; optional28th object')
 a=ap.parse_args();reference=a.reference_repository.resolve(strict=True);cc=a.compiler.resolve(strict=True);sdk=a.sdk_include.resolve(strict=True);out=a.output.resolve()
 need(not out.exists()and out.parent.is_dir()and not out.is_relative_to(ROOT)and not out.is_relative_to(reference),'fresh output outside source repositories with existing parent')
 before={}
 def pinned(p,digest,n=None):
  b=p.read_bytes();need(sha(b)==digest and(n is None or len(b)==n),'pinned input '+str(p));before[p]=digest;return b
 spec_raw=pinned(COMP/'build-spec.json',SPEC_SHA);spec=json.loads(spec_raw)
 manifest_raw=(COMP/'manifest.json').read_bytes();manifest=json.loads(manifest_raw)
 need(manifest['schema']=='authored-practice173-source-manifest/1','source manifest schema')
 paths=[r['path']for r in manifest['files']];need(len(paths)==len(set(paths))and set(paths)==set(manifest['declared_paths']),'unique exact manifest allowlist')
 for row in manifest['files']:pinned(safe(ROOT,row['path']),row['sha256'],row['bytes'])
 for rel in(REL,spec['renderer_overlay']):
  actual={(p.relative_to(ROOT)).as_posix()for p in(ROOT/rel).rglob('*')if p.is_file()};wanted={p for p in paths if p.startswith(rel+'/')}
  if rel==REL:wanted.add(REL+'/manifest.json')
  need(actual==wanted,'unexpected or missing component file '+rel)
 need(str(Path(__file__).resolve().relative_to(ROOT).as_posix())in paths,'builder covered by manifest')
 need(len(spec['objects'])==28 and sum(not x.get('owned_local_asset_required')for x in spec['objects'])==27,'selected28/asset-free27 object scope')
 for row in spec['references']:pinned(safe(reference,row['path']),row['sha256'],row['bytes'])
 helper=load_module('practice173_source_helpers',reference/'tools/build_renderer_normalization171_source.py')
 parser=load_module('practice173_public_native_rpx',reference/'patcher/native_rpx.py')
 base=safe(reference,spec['renderer_base']);base_manifest=json.loads((base/'manifest.json').read_bytes());bs=json.loads((base/'build-spec.json').read_bytes())
 for row in base_manifest['files']:pinned(safe(reference,row['path']),row['sha256'],row['bytes'])
 need(sha(cc.read_bytes())==spec['compiler_sha256']==bs['compiler_sha256'],'exact reviewed compiler')
 tc=cc.parent.parent.resolve(strict=True);need(cc==tc/'bin/powerpc-eabi-gcc.exe','reviewed compiler layout')
 roots={'@SDK@':sdk,'@TOOLCHAIN@':tc}
 def external(path):
  key,sep,rel=path.partition('/');need(key in roots and sep,'known external root');return safe(roots[key],rel)
 for key in('SDK_headers','toolchain_headers','toolchain_tools'):
  for row in bs[key]:pinned(external(row['path']),row['sha256'],row['bytes'])
 need({r['path']for r in bs['toolchain_tools']}=={'@TOOLCHAIN@/bin/powerpc-eabi-gcc.exe','@TOOLCHAIN@/powerpc-eabi/bin/as.exe','@TOOLCHAIN@/bin/powerpc-eabi-ld.exe','@TOOLCHAIN@/libexec/gcc/powerpc-eabi/15.2.0/cc1.exe'},'complete declared tool set')
 pch_roots={COMP,ROOT/spec['renderer_overlay'],base,sdk,tc/'lib/gcc/powerpc-eabi/15.2.0/include',tc/'powerpc-eabi/include'};helper.no_pch(pch_roots)
 env={k:v for k,v in os.environ.items()if k.upper()in{'SYSTEMROOT','WINDIR','COMSPEC','TEMP','TMP','PATHEXT','SYSTEMDRIVE'}}
 env['PATH']=str(cc.parent)+os.pathsep+str(Path(env.get('SystemRoot',env.get('SYSTEMROOT','C:/Windows')))/'System32');env['LC_ALL']='C';env['LANG']='C'
 commands=[]
 def run(args,cwd=COMP):
  p=subprocess.run([str(x)for x in args],cwd=cwd,env=env,capture_output=True,text=True);need(p.returncode==0,p.stdout+p.stderr);commands.append(dict(command=[str(x)for x in args],stdout=p.stdout,stderr=p.stderr));return p
 for name,rel in(('as','powerpc-eabi/bin/as.exe'),('cc1','libexec/gcc/powerpc-eabi/15.2.0/cc1.exe')):need(Path(run([cc,'-print-prog-name='+name]).stdout.strip()).resolve(strict=True)==safe(tc,rel),'compiler helper resolution '+name)
 need(run([cc,'-print-file-name=specs']).stdout.strip()=='specs','external compiler specs refused')
 capacity=None
 if a.capacity_source:
  capacity=a.capacity_source.resolve(strict=True);need(not capacity.is_relative_to(ROOT)and not capacity.is_relative_to(reference),'game-derived capacity input must remain private')
  generated=json.loads((capacity/'generation.json').read_bytes());need(generated['schema']=='owned-EUR173-capacity-generation/1'and generated['passed']and generated['contains_game_text'],'explicit owned-text provenance')
  need(generated['source_pack_pins']==spec['capacity']['owned_pack_pins'],'exact owned language-pack identities');pinned(capacity/'capacity_policy.c',spec['capacity']['generated_policy_sha256'])
 for p,d in before.items():need(sha(p.read_bytes())==d,'input changed before output')
 out.mkdir();merged=out/'renderer-sources';merged.mkdir()
 # Private merge only. The historical171 component itself is never edited.
 used=set()
 renderer_units=[u for u in bs['units']if u['name']!='private']
 for u in renderer_units:
  for dep in u['M_dependencies']:
   if dep.startswith('@COMPONENT@/'):used.add(dep.partition('/')[2])
 used.add('source/render-current-graph-preflight-r2/hx_index_layout.h')
 overlay=set(spec['renderer_overlay_files'])
 for rel in sorted(used):
  source=safe(ROOT/spec['renderer_overlay'],rel)if rel in overlay else safe(base,rel);target=merged/rel;target.parent.mkdir(parents=True,exist_ok=True)
  with target.open('xb')as f:f.write(source.read_bytes())
 helper.COMPONENT=merged
 def resolve(v,exists=True):
  key,sep,rel=v.partition('/');need(sep or key=='@SDK@','declared include root')
  root=merged if key=='@COMPONENT@'else roots.get(key);need(root is not None,'known dependency root')
  if not sep:return root
  return safe(root,rel)if exists else root/rel
 scans={};renderer_objects={}
 for u in renderer_units:
  src=resolve(u['source']);args=[cc,*u['flags'],*['-I'+str(resolve(p,False))for p in u['includes']]]
  scans[u['name']]={}
  for mode,field in(('-MM','MM_dependencies'),('-M','M_dependencies')):
   wanted={resolve(p)for p in u[field]}
   if u['name']=='guard':wanted.add(merged/'source/render-current-graph-preflight-r2/hx_index_layout.h')
   if u['name']=='registry':wanted.add(merged/'interfaces/native-v154/ois/owned_heap_held_view.h')
   raw=run([*args,mode,'-MT','RENDERER171_DEPS',src],merged).stdout;actual=helper.deps(raw);need(actual==wanted,'renderer exact full dependency closure '+u['name']+' '+mode)
   scans[u['name']][field]=dict(entries=len(actual),stdout_sha256=sha(raw.encode()))
  obj=out/(u['name']+'.o');run([*args,'-c',src,'-o',obj],merged);renderer_objects[u['name']]=obj
 ld=tc/'bin/powerpc-eabi-ld.exe';code=out/'code.ld';code.write_text('SECTIONS { .text 0 : { *(.text*) *(.rodata*) } /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) } }\n')
 def link(name,inputs,script=code,link_roots=()):
  obj=out/(name+'.o');run([ld,'-r',*(['--gc-sections']if link_roots else[]),'-T',script,*[x for root in link_roots for x in('-u',root)],*[renderer_objects[x]for x in inputs],'-o',obj]);renderer_objects[name]=obj
 link('ghs_core',['ghs_raw']);link('ghs',['ghs_core','graphics']);link('wrapper',['wrapper_raw'])
 joined=out/'components.ld';joined.write_text('SECTIONS { .text 0 : { *(.text*) *(.rodata*) } .data 0 : { *(.data*) } .bss : { *(.bss*) *(COMMON) } /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) } }\n')
 link('render_normalization',['ghs','wrapper','guard','registry','finish','alias'],joined,spec['renderer_main_link_roots'])
 built={'render_normalization':renderer_objects['render_normalization']}
 for obj in spec['objects']:
  if obj.get('renderer'):continue
  if obj.get('owned_local_asset_required')and capacity is None:continue
  units={}
  for u in obj['units']:
   src=capacity/u['generated']if'generated'in u else safe(ROOT,u['source']);wanted={safe(ROOT,p)for p in u['authored_dependencies']}or{src};args=[cc,*u['flags']]
   scans[src.stem]={}
   for mode in('-MM','-M'):
    raw=run([*args,mode,'-MT','RENDERER171_DEPS',src]).stdout;need(helper.deps(raw)==wanted,'authored exact dependency closure '+src.name+' '+mode);scans[src.stem][mode]=dict(entries=len(wanted),stdout_sha256=sha(raw.encode()))
   target=out/(src.stem+'.o');run([*args,'-c',src,'-o',target]);units[src.stem]=target
  if'link'in obj:
   l=obj['link'];target=out/(obj['name']+'.o');run([ld if l['tool']=='ld'else cc,*l['flags'],*[units[x]for x in l['unit_order']],'-o',target])
  else:need(len(units)==1,'single-unit direct object');target=next(iter(units.values()))
  built[obj['name']]=target
 results={};unresolved=set();defined=set()
 for expected in spec['objects']:
  name=expected['name']
  if name not in built:continue
  raw=built[name].read_bytes();need(sha(raw)==expected['object_sha256'],'exact whole selected ELF required '+name)
  identity=helper.identity(raw,parser.Elf);need(identity==expected['identity'],'exact allocated bytes, exports, UND and typed relocations '+name)
  e=parser.Elf(raw);symbols=e.symbols();code_bytes=sum(s.header[5]for s in e.sections if s.header[2]&6==6);mutable=sum(s.header[5]for s in e.sections if s.header[2]&3==3)
  unresolved.update(identity['description']['undefined']);defined.update(x[0]for x in identity['description']['exports'])
  results[name]=dict(object_sha256=sha(raw),canonical_sha256=identity['sha256'],CODE_bytes=code_bytes,mutable_bytes=mutable,unresolved=len(identity['description']['undefined']))
 need(len(results)==(28 if capacity else 27),'all selected source objects qualified')
 helper.no_pch(pch_roots);need((COMP/'manifest.json').read_bytes()==manifest_raw,'manifest unchanged')
 for p,d in before.items():need(sha(p.read_bytes())==d,'input changed during build')
 report=dict(schema='strict-partial-practice173-authored-rebuild/1',passed=True,spec_sha256=SPEC_SHA,manifest_sha256=sha(manifest_raw),selected_objects=len(results),asset_free_objects=27,capacity_owned_local_assets_used=bool(capacity),objects=results,dependency_scans=scans,commands=commands,combined_unresolved_interfaces=sorted(unresolved-defined),game_or_native_images_read=False,private_recovery_objects_read=False,SDK_or_game_executed=False,source715_promoted=False,hardware_verified=False,limits=['Exact authored relocatable objects only; complete native integration and physical ownership remain external.','Compiler/WUT/header/tool boundary is pinned; Windows compiler runtime closure is not claimed.','Capacity generation is private, contains owned original game text, and must never be included in a source export.','Experimental173 has unresolved menu opening and no proven hardware boot, SaveRestore, capacity or timing result.'])
 save(out/'report.json',report);print(json.dumps(dict(passed=True,selected_objects=len(results),asset_free_objects=27,capacity_local=bool(capacity),report_sha256=sha((out/'report.json').read_bytes()))))
if __name__=='__main__':main()
