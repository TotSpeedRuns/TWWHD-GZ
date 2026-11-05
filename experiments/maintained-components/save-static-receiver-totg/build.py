"""Rebuild owned relocatables; no game or SDK binary input."""
from pathlib import Path
import argparse,hashlib,json,os,subprocess
HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--compiler',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
 cc=a.compiler.resolve(strict=True);ld=cc.with_name('powerpc-eabi-ld.exe');out=a.output.resolve();s=json.loads((HERE/'build-spec.json').read_bytes())
 assert not out.exists() and out.parent.is_dir() and not out.is_relative_to(HERE)
 assert sha(cc)==s['compiler_sha256'] and sha(ld)==s['linker_sha256']
 manifest=json.loads((HERE/'manifest.json').read_bytes())
 assert {x.relative_to(HERE).as_posix()for x in HERE.rglob('*')if x.is_file()}==set(manifest['files'])|{'manifest.json'}
 for name,h in manifest['files'].items():assert sha(HERE/name)==h
 env={k:v for k,v in os.environ.items()if k.upper()in {'SYSTEMROOT','WINDIR','COMSPEC','TEMP','TMP'}}
 env['PATH']=str(cc.parent)+os.pathsep+str(Path(env.get('SYSTEMROOT','C:/Windows'))/'System32')
 out.mkdir()
 def run(args):subprocess.run([str(x)for x in args],check=True,capture_output=True,timeout=45,env=env)
 raw=out/'preflight.o';provider=out/'preflight_provider.o';door=out/'rando_totg_hub_doors.o';script=out/'provider.ld'
 run([cc,*s['flags'],'-c',HERE/'source/render-current-graph-preflight-r2/render_graph.c','-o',raw])
 script.write_text('SECTIONS { .text 0 : { *(.text*) *(.rodata*) } /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) } }\n',newline='\n')
 run([ld,'-r','-T',script,raw,'-o',provider]);run([cc,'-mcpu=750','-meabi','-m32','-c',HERE/'source/rando_totg_hub_doors.S','-o',door])
 assert sha(provider)==s['expected_preflight_object'] and sha(door)==s['expected_ToTG_object']
 result=dict(passed=True,source_only=True,preflight_sha256=sha(provider),ToTG_sha256=sha(door),full_game_or_hardware_proved=False)
 (out/'report.json').write_text(json.dumps(result,indent=2)+'\n',newline='\n');print(json.dumps(result))
if __name__=='__main__':main()
