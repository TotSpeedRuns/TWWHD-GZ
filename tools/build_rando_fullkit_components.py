"""Rebuild the selected one-shot Rando inventory components from authored source.

Builds seven ET_REL objects only. No SDK, game, memory capture, image patch,
console connection or game/Setup deployment is performed.
"""
from pathlib import Path,PurePosixPath
import argparse,hashlib,json,os,struct,subprocess,sys
if not __debug__:raise RuntimeError('Run without optimized Python.')
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
REL='experiments/maintained-components/rando-fullkit'
COMP=ROOT/REL
SPEC_SHA='afb7ce2aac559088145539754e96477cf53141d7abacfc456f72fc70b533d531'
def need(value,message):
    if not value:raise ValueError(message)
def sha(raw):return hashlib.sha256(raw).hexdigest()
def contained(relative):
    need(isinstance(relative,str)and '\\'not in relative,'POSIX source path')
    q=PurePosixPath(relative);need(not q.is_absolute()and q.parts and not any(x in('.','..')for x in q.parts),'contained source path')
    p=ROOT/relative;need(not p.is_symlink(),'source symlink refused');resolved=p.resolve(strict=True)
    need(resolved.is_file()and resolved.is_relative_to(ROOT),'source escaped tree')
    need(not any(x.is_symlink()for x in[p,*p.parents]if x.is_relative_to(ROOT)),'parent symlink refused');return resolved
def shape(path):
    raw=path.read_bytes();need(len(raw)>=52 and raw[:7]==b'\x7fELF\x01\x02\x01','ELF32 BE required')
    h=struct.unpack_from('>16sHHIIIIIHHHHHH',raw);need(h[1]==1 and h[2]==20 and h[11]==40 and 0<h[12]<1024,'PPC ET_REL required')
    need(h[6]+40*h[12]<=len(raw),'section bounds');ss=[struct.unpack_from('>10I',raw,h[6]+40*i)for i in range(h[12])]
    for s in ss:
        if s[1]!=8:need(s[4]+s[5]<=len(raw),'body bounds')
    def body(i):
        s=ss[i];return raw[s[4]:s[4]+s[5]]
    code=readonly=mutable=0;exports=[];undefined=[]
    for s in ss:
        if s[2]&2 and s[5]:
            if s[2]&1:mutable+=s[5]
            elif s[2]&4:code+=s[5]
            else:readonly+=s[5]
        if s[1]!=2:continue
        need(s[9]==16 and s[5]%16==0 and s[6]<len(ss),'symbol table')
        strings=body(s[6])
        for off in range(s[4],s[4]+s[5],16):
            name,value,size,info,other,index=struct.unpack_from('>IIIBBH',raw,off)
            need(name<len(strings),'string offset');end=strings.find(b'\0',name);need(end>=name,'symbol termination')
            text=strings[name:end].decode('ascii')
            if text and info>>4 in(1,2):
                if not index:undefined.append(dict(name=text,info=info))
                else:exports.append(text)
    need(mutable==readonly==0 and code>0,'CODE-only selected component')
    return dict(CODE=code,mutable=mutable,readonly=readonly,exports=sorted(exports),undefined=sorted(undefined,key=lambda x:x['name']))
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--compiler',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
    cc=a.compiler.resolve(strict=True);need(cc.is_file(),'compiler file');out=a.output.resolve()
    need(not out.exists()and out.parent.is_dir()and not out.is_relative_to(ROOT),'fresh output outside source tree')
    specraw=(COMP/'build-spec.json').read_bytes();need(sha(specraw)==SPEC_SHA,'changed selected build spec');spec=json.loads(specraw)
    manifest=json.loads((COMP/'source-manifest.json').read_bytes());paths=[r['path']for r in manifest['files']]
    need(len(paths)==len(set(paths))and set(paths)==set(manifest['declared_paths']),'manifest allowlist')
    inputs=[]
    for r in manifest['files']:
        p=contained(r['path']);b=p.read_bytes();need(len(b)==r['bytes']and sha(b)==r['sha256'],'changed source '+r['path'])
        inputs.append(dict(path=r['path'],bytes=len(b),sha256=sha(b)))
    need(Path(__file__).resolve().relative_to(ROOT).as_posix()in paths,'builder manifest-covered')
    actual={p.relative_to(ROOT).as_posix()for p in COMP.rglob('*')if p.is_file()}
    need(actual=={p for p in paths if p.startswith(REL+'/')}|{REL+'/source-manifest.json'},'unexpected component file')
    for p in COMP.rglob('*'):need(not p.is_symlink()and p.suffix.lower()not in('.gch','.pch'),'source symlink/precompiled file refused')
    env={k:v for k,v in os.environ.items()if k.upper()in{'SYSTEMROOT','WINDIR','COMSPEC','TEMP','TMP','PATHEXT','SYSTEMDRIVE'}}
    env['PATH']=str(cc.parent)+os.pathsep+os.defpath;env['LANG']='C';env['LC_ALL']='C'
    objects=[];commands=[];exported=set();unresolved=set();out.mkdir()
    for record in spec['objects']:
        source=contained(REL+'/source/'+record['source']);target=out/record['output']
        argv=[str(cc),*spec['assembler_flags'],'-c',str(source),'-o',str(target)]
        result=subprocess.run(argv,cwd=COMP,env=env,capture_output=True,text=True)
        need(result.returncode==0,result.stdout+result.stderr);raw=target.read_bytes();need(sha(raw)==record['sha256'],'object differs: '+record['output'])
        identity=shape(target);need(identity['CODE']==record['CODE'],'CODE size differs')
        exported.update(identity['exports']);unresolved.update(x['name']for x in identity['undefined'])
        objects.append(dict(name=target.name,bytes=len(raw),sha256=sha(raw),**identity))
        commands.append(dict(argv=[x.replace(str(ROOT),'@SOURCE@').replace(str(out),'@OUTPUT@').replace(str(cc.parent.parent),'@TOOLCHAIN@')for x in argv],stdout=result.stdout,stderr=result.stderr))
    native=sorted(unresolved-exported);bindings=json.loads((COMP/'bindings.json').read_bytes())['native_bindings']
    need(native==sorted(bindings),'exact named Native interface closure');need(sum(x['CODE']for x in objects)==1668,'complete transport/kit extent')
    for r in inputs:need(sha(contained(r['path']).read_bytes())==r['sha256'],'source changed during build')
    report=dict(schema='reproduced-authored-rando-fullkit-components/1',passed=True,component_only=True,
      objects=objects,commands=commands,source_files=inputs,compiler_sha256=sha(cc.read_bytes()),
      reference_compiler_exact=sha(cc.read_bytes())==spec['compiler_reference_sha256'],
      total_CODE=1668,mutable=0,readonly=0,net_GAME_CODE_after_replacing_old204B_transport=1464,
      unresolved_Native_interfaces=native,SDK_game_recipe_or_private_object_input=False,
      image_patch_or_console_or_network_action=False)
    p=out/'build-report.json';encoded=(json.dumps(report,indent=2)+'\n').encode()
    with p.open('xb')as f:f.write(encoded);f.flush();os.fsync(f.fileno())
    print(json.dumps(dict(passed=True,objects=7,CODE=1668,mutable=0,report_sha256=sha(encoded))))
if __name__=='__main__':main()
