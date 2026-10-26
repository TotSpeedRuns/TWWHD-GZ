"""Reproduce R755 and compile the bounded warp-table revision; no engine launch."""
from pathlib import Path
import hashlib,json,subprocess,sys
H=Path(__file__).resolve().parent;R=H.parents[1]
SDK=R/'work/aroma-sdk/opt/devkitpro/devkitPPC/bin'
sys.path.insert(0,str(R/'outputs/wwhdgz/WiiU/Patcher'))
from native_rpx import Elf
def pin(p):
    p=Path(p).resolve();b=p.read_bytes();return dict(path=str(p),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def main():
    assert not(H/'build-report.json').exists()
    out=H/'build';out.mkdir(exist_ok=False)
    parent=json.loads((R/'work/quarter-heart-pieces-candidate-20261026/build-report.json').read_text())
    stage=json.loads((H/'staging.json').read_text());assert stage['old_warp_IDs_preserved']and stage['only_authorized_old_tuple_repairs']and stage['route_payloads_preserved']
    modules=[parent['bottle_object'],parent['stick_module'],parent['module']]
    for row in modules:assert pin(row['path'])==row
    runs=[]
    def run(args):
        r=subprocess.run(list(map(str,args)),capture_output=True,text=True);runs.append(dict(args=list(map(str,args)),exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr))
        (out/'compiler-log.json').write_text(json.dumps(runs,indent=2)+'\n');assert r.returncode==0,r.stderr
    editions={}
    for name,source in [('baseline',Path(stage['source']['path'])),('personal',H/'runtime-personal.s'),('public',H/'runtime-public.s')]:
        home=out/name;home.mkdir();asm=home/'assembly.o';combined=home/'combined.o';final=home/'runtime.o'
        run([SDK/'powerpc-eabi-gcc.exe','-mcpu=750','-c','-x','assembler',source,'-o',asm])
        run([SDK/'powerpc-eabi-ld.exe','-r',*parent['aliases'],asm,*[m['path']for m in modules],'-o',combined])
        e=Elf(combined.read_bytes());symbols={s['name']for s in e.symbols()};anchors=[]
        for i,s in enumerate(e.sections):
            if s.kind==8 and s.header[5]:
                anchor=f'_gz_gameplay_anchor_runtime_{i}'
                if anchor not in symbols:anchors+=['--add-symbol',anchor+'='+e.name(i)+':0,local']
        run([SDK/'powerpc-eabi-objcopy.exe',*anchors,combined,final])
        editions[name]=dict(source=pin(source),assembly=pin(asm),combined=pin(combined),object=pin(final))
    assert editions['baseline']['object']['sha256']==parent['editions']['candidate']['object']['sha256']=='02f4e2b102b48ed3f233d250a21b7c197153cb58251622ad89d2ab634eafb02c'
    es={k:Elf(Path(v['object']['path']).read_bytes())for k,v in editions.items()}
    def section(e,name):return next(s for i,s in enumerate(e.sections)if e.name(i)==name)
    def undefined(e):return sorted(s['name']for s in e.symbols()if s['name']and not s['section'])
    assert undefined(es['baseline'])==undefined(es['personal'])==undefined(es['public'])
    for name in ('.bss','.gz_route_ro'):
        assert section(es['baseline'],name).data==section(es['personal'],name).data==section(es['public'],name).data
        assert section(es['baseline'],name).header[5]==section(es['personal'],name).header[5]
    assert section(es['personal'],'.text').data==section(es['public'],'.text').data
    assert section(es['baseline'],'.text').header[5]==section(es['personal'],'.text').header[5]
    b=section(es['baseline'],'.text').data;p=section(es['personal'],'.text').data
    differences=[dict(offset=i,before=b[i:i+4].hex(),after=p[i:i+4].hex())for i in range(0,len(b),4)if b[i:i+4]!=p[i:i+4]]
    assert len(differences)==2,differences
    report=dict(compiled=True,baseline_reproduced=True,editions=editions,modules=modules,aliases=parent['aliases'],staging=pin(H/'staging.json'),script=pin(Path(__file__)),text_word_differences=differences,new_hooks=0,new_imports=0,new_heap_bytes=0,route_RO_exact=True,BSS_exact=True,hardware_tested=False,native_tested=False)
    (H/'build-report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(dict(compiled=True,baseline_exact=True,text_differences=differences,objects={k:v['object']for k,v in editions.items()})))
if __name__=='__main__':main()
