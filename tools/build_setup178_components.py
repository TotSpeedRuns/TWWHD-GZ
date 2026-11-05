"""Reproduce the authored Setup178 component; no image, recipes or SDK input.

This builds ET_REL objects only. It does not patch an RPX/WUHB or access a
console, and it refuses an existing output directory or changed source inputs.
"""
from pathlib import Path,PurePosixPath
import argparse,collections,hashlib,json,os,re,struct,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
REL='experiments/maintained-components/setup-usb-v178'
COMP=ROOT/REL
SPEC_SHA='72ceef0f52070add72002ae97a51f9a73415b89b317ec9ed248fa11c89e9da06'

def need(v,message):
 if not v:raise ValueError(message)

def sha(b):return hashlib.sha256(b).hexdigest()

def safe(relative):
 need(isinstance(relative,str),'text source path');q=PurePosixPath(relative)
 need('\\'not in relative and not q.is_absolute()and q.parts and not any(x in('.','..')for x in q.parts),'contained POSIX source path')
 p=ROOT/relative;resolved=p.resolve(strict=True);need(resolved.is_relative_to(ROOT)and resolved.is_file(),'source path escaped root')
 need(not any(x.is_symlink()for x in[p,*p.parents]if x.is_relative_to(ROOT)),'source symlink refused');return resolved

def object_shape(p,catalog,bindings):
 b=p.read_bytes();need(len(b)>=52 and b[:7]==b'\x7fELF\x01\x02\x01','ELF32 big-endian object')
 e=struct.unpack_from('>16sHHIIIIIHHHHHH',b);need(e[1]==1 and e[2]==20 and e[11]==40 and 0<e[12]<1024,'PowerPC ET_REL section shape')
 need(e[6]+40*e[12]<=len(b),'bounded section table');hs=[struct.unpack_from('>10I',b,e[6]+40*i)for i in range(e[12])]
 for h in hs:
  if h[1]!=8:need(h[4]+h[5]<=len(b),'bounded section')
 data=[b[h[4]:h[4]+h[5]]if h[1]!=8 else b''for h in hs];syms=[];tables={}
 def text(blob,i):
  need(0<=i<len(blob),'bounded string');end=blob.find(b'\0',i);need(end>=i,'terminated string');return blob[i:end].decode('ascii')
 for i,h in enumerate(hs):
  if h[1]!=2:continue
  need(h[9]==16 and h[5]%16==0 and h[6]<len(hs),'symbol table shape');ss=[]
  for off in range(0,len(data[i]),16):
   name,value,size,info,other,section=struct.unpack_from('>IIIBBH',data[i],off);ss.append(dict(name=text(data[h[6]],name),value=value,size=size,info=info,other=other,section=section))
  tables[i]=ss;syms.extend(ss)
 alloc=[(i,h)for i,h in enumerate(hs)if h[2]&2 and h[5]];need(len(alloc)==1 and alloc[0][1][2]==6,'one immutable CODE/RO allocation')
 index,h=alloc[0];body=data[index];need(len(body)==5656,'selected merged extent')
 need(sum(h[5]for h in hs if h[2]&1)==0,'zero mutable bytes')
 und=sorted(s['name']for s in syms if s['name']and not s['section']);need(und==sorted(bindings),'all genuine interfaces remain unresolved')
 rows=[]
 for i,h in enumerate(hs):
  if h[1]!=4:continue
  need(h[9]==12 and h[5]%12==0 and h[7]==index and h[6]in tables,'merged relocation shape')
  for off,info,addend in struct.iter_unpack('>IIi',data[i]):
   symbol=info>>8;kind=info&255;need(symbol<len(tables[h[6]])and off<len(body),'bounded relocation');rows.append(dict(site=off,kind=kind,addend=addend,symbol=tables[h[6]][symbol]))
 counts=dict(sorted(collections.Counter(str(r['kind'])for r in rows).items()));need(counts=={'1':40,'10':14,'4':18,'6':19},'selected typed relocation kinds/counts')
 table=next(s for s in syms if s['name']=='gz_roles');need(table['section']==index and table['value']==2896 and table['size']==160,'catalog table shape')
 pointers=sorted((r for r in rows if r['kind']==1),key=lambda r:r['site']);expected=[p[k]for p in catalog for k in('rpx_sha256','title_sha256')]
 for i,(r,digest)in enumerate(zip(pointers,expected)):
  need(r['site']==table['value']+4*i and r['symbol']['section']==index,'CODE-owned catalog pointer row')
  at=r['symbol']['value']+r['addend'];need(0<=at<=len(body)-65 and body[at:at+65]==digest.encode()+b'\0','actual catalog string')
 return dict(ELF='PowerPC ELF32 big-endian ET_REL',immutable_CODE_RO_bytes=len(body),mutable_bytes=0,imports=0,undefined_symbols=und,relocation_counts=counts,catalog_pointer_rows=len(pointers))

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--compiler',type=Path,required=True,help='powerpc-eabi-gcc from devkitPPC (reviewed r49)');ap.add_argument('--output',type=Path,required=True,help='fresh directory outside this source tree, with existing parent');a=ap.parse_args()
 cc=a.compiler.resolve(strict=True);need(cc.is_file(),'compiler file');ld=cc.parent/('powerpc-eabi-ld.exe'if cc.suffix.lower()=='.exe'else'powerpc-eabi-ld');need(ld.is_file(),'matching sibling linker')
 out=a.output.resolve();need(not out.exists()and out.parent.is_dir()and not out.is_relative_to(ROOT),'fresh output outside source tree required')
 raw=(COMP/'build-spec.json').read_bytes();need(sha(raw)==SPEC_SHA,'changed selected build spec');spec=json.loads(raw)
 need(spec['standard_headers']==['stdint.h'],'declared standard C header set')
 manifest=json.loads((COMP/'source-manifest.json').read_text());need(manifest['schema']=='authored-Setup178-source-manifest/1','source manifest schema');paths=[r['path']for r in manifest['files']];need(len(paths)==len(set(paths))and set(paths)==set(manifest['declared_paths']),'exact manifest allowlist');inputs=[]
 for row in manifest['files']:
  p=safe(row['path']);b=p.read_bytes();need(len(b)==row['bytes']and sha(b)==row['sha256'],'changed source '+row['path']);inputs.append(dict(path=row['path'],bytes=len(b),sha256=sha(b)))
 wanted={p for p in paths if p.startswith(REL+'/')}|{REL+'/source-manifest.json'};actual={p.relative_to(ROOT).as_posix()for p in COMP.rglob('*')if p.is_file()};need(actual==wanted,'unexpected component file')
 need(Path(__file__).resolve().relative_to(ROOT).as_posix()in paths,'builder is manifest-covered')
 for p in COMP.rglob('*'):need(not p.is_symlink()and not p.name.endswith(('.gch','.pch')),'source symlink/precompiled header refused')
 catalog=json.loads((COMP/'catalog.json').read_text())['records'];need(len(catalog)==20 and all(re.fullmatch('[0-9a-f]{64}',p[k])for p in catalog for k in('rpx_sha256','title_sha256')),'20 exact hash records')
 header=(COMP/'roles/verified_roles.h').read_text();need(re.findall(r'\{"([0-9a-f]{64})","([0-9a-f]{64})"\}',header)==[(p['rpx_sha256'],p['title_sha256'])for p in catalog],'catalog/header identity')
 bindings=json.loads((COMP/'bindings.json').read_text())['unresolved_bindings'];need(sorted(bindings)==spec['expected_merged']['undefined_symbols'],'same binding set')
 env={k:v for k,v in os.environ.items()if k.upper()in{'SYSTEMROOT','WINDIR','COMSPEC','TEMP','TMP','PATHEXT','SYSTEMDRIVE'}};env['PATH']=str(cc.parent)+os.pathsep+os.defpath;env['LC_ALL']='C';env['LANG']='C'
 commands=[]
 def normalized(x):
  return str(x).replace(str(ROOT),'@SOURCE@').replace(str(out),'@OUTPUT@').replace(str(cc.parent.parent),'@TOOLCHAIN@')
 def run(cmd):
  p=subprocess.run([str(x)for x in cmd],cwd=COMP,env=env,capture_output=True,text=True);need(p.returncode==0,p.stdout+p.stderr);commands.append(dict(argv=[normalized(x)for x in cmd],stdout=p.stdout,stderr=p.stderr))
 out.mkdir()
 for source,target,flags in[('usb/practice_selection.c','practice_selection.o',spec['C_flags_O2']),('usb/volume_choice.c','volume_choice.o',spec['C_flags_O2']),('usb/contexts.S','usb-contexts.o',spec['assembler_flags']),('roles/role_match.c','role_match.o',spec['C_flags_Os']),('roles/contexts.S','role-contexts.o',spec['assembler_flags'])]:run([cc,*flags,'-c',COMP/source,'-o',out/target])
 run([ld,'-r','-T',COMP/'usb/bundle.ld',out/'usb-contexts.o',out/'volume_choice.o',out/'practice_selection.o','-o',out/'usb-adapter.o'])
 run([ld,'-r','--emit-relocs',out/'role_match.o',out/'role-contexts.o','-o',out/'role-adapter.o'])
 run([ld,'-r','-T',COMP/'merge.ld','-o',out/'usb-upgrade-linked.o',out/'usb-adapter.o',out/'role-adapter.o'])
 objects=[]
 for name,digest in spec['expected_objects'].items():
  b=(out/name).read_bytes();need(sha(b)==digest,'object differs from reviewed selection: '+name);objects.append(dict(name=name,bytes=len(b),sha256=sha(b)))
 shape=object_shape(out/'usb-upgrade-linked.o',catalog,bindings)
 for r in inputs:need(sha(safe(r['path']).read_bytes())==r['sha256'],'source changed during build')
 report=dict(schema='authored-Setup178-reproduced-components/1',passed=True,component_only=True,no_game_SDK_recipe_or_private_object_input=True,compiled_from_included_source=True,compiler_sha256=sha(cc.read_bytes()),linker_sha256=sha(ld.read_bytes()),reference_toolchain_exact=sha(cc.read_bytes())==spec['compiler_reference_sha256']and sha(ld.read_bytes())==spec['linker_reference_sha256'],source_files=inputs,commands=commands,objects=objects,merged_shape=shape,RPX_WUHB_or_console_mutated=False)
 encoded=(json.dumps(report,indent=2)+'\n').encode()
 with(out/'build-report.json').open('xb')as f:f.write(encoded);f.flush();os.fsync(f.fileno())
 print(json.dumps(dict(passed=True,objects=len(objects),merged_sha256=objects[-1]['sha256'],immutable_bytes=5656,mutable_bytes=0,report_sha256=sha(encoded))))
if __name__=='__main__':main()
