"""Rebuild the authored v171 renderer component, without a game image.

Only the pinned source snapshot, public ELF inspector and reviewed compiler /
assembler / linker / WUT-header boundary are build inputs. Imports stay UND.
"""
from pathlib import Path, PurePosixPath
import argparse, hashlib, json, os, re, struct, subprocess, sys
sys.dont_write_bytecode=True
REPO=Path(__file__).resolve().parents[1]
COMPONENT=REPO/'experiments/maintained-components/renderer-normalization-v171'
INSPECTOR=REPO/'patcher/native_rpx.py'
CC_SHA='518fadfdd99f8882980a2564fa385597dffcfe33b589b2844c5ce3eeaab7dda5'
INSPECTOR_SHA='52c566d12e8bb4b2182c2a1de93883e0eb40fa9db7ff2fba49c83cfe9b635cb5'
UNIT_PLAN_SHA='0fb26296fb71fbebf6fb14f6ff9d3f37da228637e02b7825725a022b95e16617'
EXTERNAL_INPUTS_SHA='523f51fa7f4408c21a2fa0a41a4aff6f0896d64361bef782fc0d0a4f855b3a2d'
AUTHORED=set(["interfaces/mapped-api/mapped_api.h", "interfaces/mapped-policy/mapped_storage_policy.h", "interfaces/native-v154/native_lifetime/forest_snapshot.h", "interfaces/native-v154/native_lifetime/heap_domain_cut.h", "interfaces/native-v154/native_lifetime/heap_graph.h", "interfaces/native-v154/native_lifetime/joint_restore.h", "interfaces/native-v154/native_lifetime/lifetime.h", "interfaces/native-v154/native_lifetime/object_binding.h", "interfaces/native-v154/native_lifetime/object_binding_try.h", "interfaces/native-v154/native_lifetime/process.h", "interfaces/native-v154/native_lifetime/rebuild.h", "interfaces/native-v154/ois/arena_snapshot.h", "interfaces/native-v154/ois/fpc_global_cut.h", "interfaces/native-v154/ois/native_context_cut.h", "interfaces/native-v154/ois/owned_heap_common_domain.h", "interfaces/native-v154/ois/owned_heap_cut_policy.h", "interfaces/native-v154/ois/owned_heap_dual_cut.h", "interfaces/native-v154/ois/owned_heap_held_view.h", "interfaces/native-v154/ois/owned_heap_image_version.h", "interfaces/native-v154/ois/owned_heap_native.h", "interfaces/native-v154/ois/owned_heap_process.h", "interfaces/native-v154/ois/owned_heap_save.h", "interfaces/native-v154/ois/owned_images.h", "interfaces/native-v154/ois/player_global_cut.h", "interfaces/native-v154/ois/world_global_cut.h", "interfaces/recovery/restore_workspace.h", "interfaces/recovery/workspace_overlay_private.h", "interfaces/workspace/mapped_ois_backend.h", "interfaces/workspace/split_workspace.h", "source/ghs-pre-hg-gpu-order-r2/ghs_pre_hg_tick.c", "source/render-capture-normalize-wrapper-r1/capture_normalize.c", "source/render-capture-normalize-wrapper-r1/capture_normalize.h", "source/render-current-graph-preflight-r1/render_graph.h", "source/render-current-graph-preflight-r2/lifetime_witness.h", "source/render-current-graph-preflight-r2/render_graph.c", "source/render-current-graph-preflight-r2/render_graph.h", "source/render-current-graph-preflight-r2/render_lifetime.c", "source/render-normalization-source-join-r1/graphics_alias.S", "source/render-normalized-private-closure-r1/compound_private.c", "source/render-normalized-slots-r1/normalized_finish.c", "source/render-normalized-slots-r1/normalized_slot_registry.c", "source/render-normalized-slots-r1/normalized_slot_registry.h", "source/render-normalized-slots-r1/render_trace.h", "source/v168-gpu-framework-serialization-r1/gfx_owner_query.c"])
EXTRAS={'README.md','scope.json','provenance.json','build-spec.json','.gitattributes','diagnostics/decode_trace.py','diagnostics/diagnostic_enum.json'}
OBJECTS={'render_normalization':('5f5357e97a794f2f5d456401d03779e02ed3cbecb1c4139622917c24e27ab378',29184,91200,79),
 'private':('c38824087da5b5c768481d8b23bb6f304954fd165547002ad053387af8d530e5',108,0,2)}

def need(value,message):
 if not value:raise ValueError(message)
def sha(raw):return hashlib.sha256(raw).hexdigest()
def canonical(value):return json.dumps(value,sort_keys=True,separators=(',',':')).encode()
def safe(root,relative,exists=True):
 need(type(relative)is str and '\\'not in relative,'POSIX relative input path required')
 q=PurePosixPath(relative);need(not q.is_absolute()and not any(p in('..','.')for p in q.parts),'Contained relative input path required')
 original=root/relative;p=original.resolve(strict=exists)
 need(p.is_relative_to(root),'Input path escaped its allowed root')
 need(not any(a.is_symlink()for a in [original,*original.parents]if a.is_relative_to(root)),'Symlink input path refused')
 return p
def identity(raw,Elf):
 e=Elf(raw);sy={(s['table'],s['index']):s for s in e.symbols()}
 active={i:e.name(i)for i,s in enumerate(e.sections)if s.header[2]&2 and s.header[5]}
 sections=[dict(name=active[i],kind=s.kind,flags=s.header[2],bytes=s.header[5],sha256=sha(bytes(s.header[5])if s.kind==8 else s.data))for i,s in enumerate(e.sections)if i in active]
 edges=[]
 for rel in e.sections:
  if rel.kind!=4 or rel.header[7]not in active:continue
  for at,info,ad in struct.iter_unpack('>IIi',rel.data):
   s=sy[rel.header[6],info>>8]
   if s['section']in active:target=['allocated',active[s['section']],s['value']+ad]
   else:need(not s['section'],'Only allocated or unresolved targets');target=['interface',s['name'],ad]
   edges.append([active[rel.header[7]],at,info&255,target])
 exports=sorted([s['name'],active[s['section']],s['value'],s['size'],s['info']]for s in sy.values()if s['name']and s['section']in active and s['info']>>4==1)
 und=sorted(s['name']for s in sy.values()if s['name']and not s['section']and s['info']>>4 in(1,2))
 description=dict(sections=sections,relocations=sorted(edges),exports=exports,undefined=und)
 return dict(description=description,sha256=sha(canonical(description)))
def deps(raw):
 # Make escaping is not shell syntax. Preserve Windows separators and drive
 # colons; unescape only Make whitespace/#/$ and explicit continuations.
 raw=raw.replace('\\\r\n','').replace('\\\n','')
 need(raw.startswith('RENDERER171_DEPS:'),'Exact compiler dependency target required')
 words=re.findall(r'(?:\\[ \t#\\]|[^\s])+',raw.split(':',1)[1])
 words=[re.sub(r'\\([ \t#\\])',r'\1',w).replace('$$','$')for w in words]
 return {Path(w).resolve(strict=True)if Path(w).is_absolute()else(COMPONENT/w).resolve(strict=True)for w in words}
def no_pch(roots):
 # GCC may use header.h.gch at -c while -M/-MM scan header.h. Forbid that
 # alternate input form throughout all declared header-search trees.
 for root in roots:
  if root.is_dir():
   for p in root.rglob('*'):
    need(p.suffix.lower()not in('.gch','.pch'),'Precompiled-header input refused: '+str(p))
def build(output,compiler,sdk):
 output=output.resolve();compiler=compiler.resolve(strict=True);sdk=sdk.resolve(strict=True)
 need(not output.exists()and output.parent.is_dir()and not output.is_relative_to(REPO),'Fresh build output outside repository with existing parent required')
 need(sha(compiler.read_bytes())==CC_SHA,'Exact reviewed compiler required before output')
 toolchain=compiler.parent.parent.resolve(strict=True)
 need(compiler==toolchain/'bin/powerpc-eabi-gcc.exe','Reviewed compiler layout required')
 need(sha(INSPECTOR.read_bytes())==INSPECTOR_SHA,'Pinned public ELF inspector required')
 manifest_raw=(COMPONENT/'manifest.json').read_bytes();manifest=json.loads(manifest_raw);spec=json.loads((COMPONENT/'build-spec.json').read_bytes())
 expected={(COMPONENT/p).relative_to(REPO).as_posix()for p in AUTHORED|EXTRAS}|{Path(__file__).resolve().relative_to(REPO).as_posix(),INSPECTOR.relative_to(REPO).as_posix()}
 need(len(manifest['files'])==len(expected)and{r['path']for r in manifest['files']}==expected,'Exact repository source/build dependency set required')
 need({p.relative_to(COMPONENT).as_posix()for p in COMPONENT.rglob('*')if p.is_file()}==AUTHORED|EXTRAS|{'manifest.json'},'Unexpected component input file')
 before={}
 for row in manifest['files']:
  p=safe(REPO,row['path']);raw=p.read_bytes();need(len(raw)==row['bytes']and sha(raw)==row['sha256'],'Pinned repository source input: '+row['path']);before[p]=sha(raw)
 need(sha(canonical(spec['units']))==UNIT_PLAN_SHA,'Exact reviewed unit/flags/includes/dependency plan required')
 external={k:spec[k]for k in('SDK_headers','toolchain_headers','toolchain_tools')}
 need(sha(canonical(external))==EXTERNAL_INPUTS_SHA,'Exact reviewed WUT/compiler-header/tool plan required')
 need(spec['compiler_sha256']==CC_SHA and set(spec['references'])==set(OBJECTS),'Exact object/compiler specification required')
 roots={'@COMPONENT@':COMPONENT,'@SDK@':sdk,'@TOOLCHAIN@':toolchain}
 def resolve(value,exists=True):
  key,sep,relative=value.partition('/');need(key in roots and(sep or key=='@SDK@'),'Known source/SDK/toolchain root required')
  return roots[key]if not sep else safe(roots[key],relative,exists)
 for key in external:
  for row in external[key]:
   p=resolve(row['path']);raw=p.read_bytes();need(len(raw)==row['bytes']and sha(raw)==row['sha256'],'Pinned external build input: '+row['path']);before[p]=sha(raw)
 need({r['path']for r in external['toolchain_tools']}=={'@TOOLCHAIN@/bin/powerpc-eabi-gcc.exe','@TOOLCHAIN@/powerpc-eabi/bin/as.exe','@TOOLCHAIN@/bin/powerpc-eabi-ld.exe','@TOOLCHAIN@/libexec/gcc/powerpc-eabi/15.2.0/cc1.exe'},'Exact reviewed compiler tool set')
 pch_roots={COMPONENT,sdk,toolchain/'lib/gcc/powerpc-eabi/15.2.0/include',toolchain/'powerpc-eabi/include'};no_pch(pch_roots)
 # Child tool discovery is confined to this compiler prefix and Windows system
 # path. CPATH/C_INCLUDE_PATH/GCC_EXEC_PREFIX/COMPILER_PATH and similar ambient
 # include/driver overrides are not inherited.
 env={k:v for k,v in os.environ.items()if k.upper()in{'SYSTEMROOT','WINDIR','COMSPEC','TEMP','TMP','PATHEXT','SYSTEMDRIVE'}}
 env['PATH']=str(compiler.parent)+os.pathsep+str(Path(env.get('SystemRoot',env.get('SYSTEMROOT','C:/Windows')))/'System32');env['LC_ALL']='C';env['LANG']='C'
 commands=[];scans={}
 def run(args):
  result=subprocess.run([str(a)for a in args],cwd=COMPONENT,env=env,capture_output=True,text=True)
  need(result.returncode==0,result.stdout+result.stderr);record=dict(command=[str(a)for a in args],stdout=result.stdout,stderr=result.stderr);commands.append(record);return result
 driver_queries={}
 for name,relative in [('as','powerpc-eabi/bin/as.exe'),('cc1','libexec/gcc/powerpc-eabi/15.2.0/cc1.exe')]:
  result=run([compiler,'-print-prog-name='+name]);reported=Path(result.stdout.strip()).resolve(strict=True)
  need(reported==safe(toolchain,relative),'Compiler helper resolution differs: '+name);driver_queries[name]=relative
 result=run([compiler,'-print-file-name=specs']);need(result.stdout.strip()=='specs','External compiler specs refused');driver_queries['specs']='no external specs file'
 authored_union=set();sdk_union=set();system_union=set()
 for unit in spec['units']:
  src=resolve(unit['source']);includes=[resolve(p,False)for p in unit['includes']];args=[compiler,*unit['flags'],*['-I'+str(p)for p in includes]]
  observed={};records={}
  for mode,field in(('-MM','MM_dependencies'),('-M','M_dependencies')):
   result=run([*args,mode,'-MT','RENDERER171_DEPS',src]);actual=deps(result.stdout);wanted={resolve(p)for p in unit[field]}
   need(actual==wanted,'Complete exact compiler dependency closure '+unit['name']+' '+mode)
   observed[field]=sorted(unit[field]);records[field]=dict(stdout_sha256=sha(result.stdout.encode()),entries=len(actual))
  for p in unit['M_dependencies']:
   key=p.partition('/')[0]
   if key=='@COMPONENT@':authored_union.add(p)
   elif key=='@SDK@':sdk_union.add(p)
   else:system_union.add(p)
  scans[unit['name']]=dict(dependencies=observed,scans=records)
 need(authored_union=={'@COMPONENT@/'+p for p in AUTHORED},'Exactly44 authored compiler dependencies required')
 need(sdk_union=={r['path']for r in external['SDK_headers']}and system_union=={r['path']for r in external['toolchain_headers']},'Complete declared external header closure')
 for p,digest in before.items():need(sha(p.read_bytes())==digest,'Input changed before output creation')
 # All identity/path/toolchain/dependency/PCH barriers run before output exists.
 output.mkdir();objects={}
 for unit in spec['units']:
  obj=output/(unit['name']+'.o');args=[compiler,*unit['flags'],*['-I'+str(resolve(p,False))for p in unit['includes']]]
  run([*args,'-c',resolve(unit['source']),'-o',obj]);objects[unit['name']]=obj
 ld=toolchain/'bin/powerpc-eabi-ld.exe';code=output/'code.ld';code.write_text('SECTIONS { .text 0 : { *(.text*) *(.rodata*) } /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) } }\n')
 def link(name,inputs,script=code,link_roots=()):
  obj=output/(name+'.o');run([ld,'-r',*(['--gc-sections']if link_roots else[]),'-T',script,*[v for root in link_roots for v in('-u',root)],*[objects[n]for n in inputs],'-o',obj]);objects[name]=obj
 link('ghs_core',['ghs_raw']);link('ghs',['ghs_core','graphics']);link('wrapper',['wrapper_raw'])
 joined=output/'components.ld';joined.write_text('SECTIONS { .text 0 : { *(.text*) *(.rodata*) } .data 0 : { *(.data*) } .bss : { *(.bss*) *(COMMON) } /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) } }\n')
 link('render_normalization',['ghs','wrapper','guard','registry','finish','alias'],joined,['_gz_pre_hg_tick','_gz_rcw_capture','_gz_rns_private_apart','_gz_rns_publish'])
 sys.path.insert(0,str(INSPECTOR.parent));from native_rpx import Elf
 results={};unresolved=set();defined=set()
 for name,(digest,code_bytes,mutable,und_count)in OBJECTS.items():
  raw=objects[name].read_bytes();expected=spec['references'][name]
  need(expected['object_sha256']==digest and sha(raw)==digest,'Whole reviewed ELF bytes/symbol ABI required: '+name)
  observed=identity(raw,Elf);need(observed['sha256']==expected['sha256'],'Reviewed allocated bytes/ABI/typed expressions required: '+name)
  e=Elf(raw);symbols=e.symbols();code_n=sum(s.header[5]for s in e.sections if s.header[2]&6==6);mutable_n=sum(s.header[5]for s in e.sections if s.header[2]&3==3)
  need((code_n,mutable_n,len(observed['description']['undefined']))==(code_bytes,mutable,und_count),'Reviewed CODE/owner/interface geometry required')
  unresolved.update(observed['description']['undefined']);defined.update(x[0]for x in observed['description']['exports'])
  results[name]=dict(path=str(objects[name]),object_sha256=sha(raw),CODE_bytes=code_n,mutable_bytes=mutable_n,canonical_identity=observed)
 need(len(unresolved-defined)==80,'Exactly80 combined genuine external interfaces remain unresolved')
 no_pch(pch_roots)
 need((COMPONENT/'manifest.json').read_bytes()==manifest_raw,'Manifest changed during build')
 for p,digest in before.items():need(sha(p.read_bytes())==digest,'Input changed during build')
 report=dict(schema='authored-renderer171-source-only-rebuild/1',passed=True,manifest_sha256=sha(manifest_raw),results=results,dependency_scans=scans,driver_queries=driver_queries,commands=commands,
  total_CODE_bytes=29292,main_CODE_bytes=29184,private_CODE_bytes=108,mutable_bytes=91200,new_mutable_vs170=0,CODE_delta_vs170=12,allocation_only_scan_limit=262144,metadata_read_budget=131072,
  compiler_units=8,authored_compiler_dependencies=44,WUT_header_dependencies=10,compiler_newlib_header_dependencies=16,combined_unresolved_interfaces=80,
  source_only=True,Game_native_or_recovery_binary_inputs=False,standalone_runtime=False,native_interfaces_resolved=False,Game_or_SDK_executed=False,hardware_SaveRestore_verified=False,
  limits=['Exact authored relocatable rebuild within the pinned compiler/WUT/header boundary, not complete current runtime reproduction.','Native SDK functions, GHS/OIS/HL owners and GPU/allocator/whole-world lifetime providers remain genuine unresolved integration requirements.','Allocation lookup limit expands to262144 only; metadata reads retain131072 and all other graph/node/log limits remain unchanged.','Trace decoder is advisory sequential metadata and supplies no freshness, admission or physical-memory permission.','No hardware Save/Restore, boot, elapsed-time, full-world replay or causal live-fix claim follows from this build.'])
 with(output/'report.json').open('xb')as f:f.write((json.dumps(report,indent=2)+'\n').encode());f.flush()
 return report
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--compiler',type=Path,required=True);p.add_argument('--sdk-include',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
 a=p.parse_args();r=build(a.output,a.compiler,a.sdk_include);print(json.dumps(dict(passed=r['passed'],CODE=r['total_CODE_bytes'],mutable=r['mutable_bytes'],source_only=True)))
if __name__=='__main__':main()
