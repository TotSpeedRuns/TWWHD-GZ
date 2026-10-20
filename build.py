"""Build the native runtime locally; never modify the input game."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess, sys
ROOT=Path(__file__).resolve().parent
sys.path[:0]=[str(ROOT/'tools'),str(ROOT/'patcher')]
from local_assets import materialize
from patch_engine import patch_rpx
FLAGS=['-mcpu=750','-O2','-ffreestanding','-fno-builtin','-msdata=none','-fno-common','-fno-section-anchors','-fno-stack-protector','-fno-asynchronous-unwind-tables','-fno-tree-vectorize','-fno-tree-loop-distribute-patterns','-msoft-float','-mregnames','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fstack-usage','-fno-jump-tables','-fno-tree-switch-conversion']
PLACEHOLDERS=('OSGetSystemTime','OSGetSystemInfo','OSGetCurrentThread','OSInitEvent','OSResetEvent','OSWaitEvent','OSSignalEvent','MEMAllocFromExpHeapEx','MEMAllocFromFrmHeapEx','MEMCreateExpHeapEx','MEMCreateFrmHeapEx','MEMDestroyExpHeap','MEMDestroyFrmHeap','MEMFreeToExpHeap','OSDynLoad_Acquire','OSDynLoad_FindExport','OSDynLoad_Release','OSDynLoad_SetAllocator')
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--original-rpx',type=Path,required=True)
    p.add_argument('--toolchain',type=Path,help='devkitPPC bin directory (or put tools on PATH)')
    p.add_argument('--assets',type=Path,default=ROOT/'local-assets');p.add_argument('--output',type=Path,default=ROOT/'build');a=p.parse_args()
    original=a.original_rpx.resolve();raw=original.read_bytes();digest=hashlib.sha256(raw).hexdigest()
    if digest!='f9f461738949a09481dc1a31c01ad27db813c4c6058fdd7d015624a4146bbf0b':raise ValueError('Supported input is EUR base game version 0')
    dest=a.output.resolve()
    if dest==original or original.is_relative_to(dest):raise ValueError('Output must be separate from the input game')
    dest.mkdir(parents=True,exist_ok=False)
    env=dict(os.environ,WWHD_RPX=str(original))
    def run(args):subprocess.run([str(v)for v in args],check=True,cwd=ROOT,env=env)
    def tool(name):
        executable='powerpc-eabi-'+name+('.exe'if os.name=='nt'else'')
        path=str(a.toolchain.resolve()/executable)if a.toolchain else shutil.which(executable)
        if not path:raise ValueError('Missing devkitPPC tool: '+executable)
        return path
    source=dest/'patch_wwhdgz.asm';source.write_text(materialize(ROOT/'runtime/patch_wwhdgz.asm.in',a.assets,ROOT/'runtime/local-assets.json'))
    run([sys.executable,ROOT/'tools/convert_native_assembly.py','--source',source,'--build-dir',dest,'--relocate-game-literals'])
    asm=dest/'runtime.s';text=asm.read_text()
    if text.count('andi. 8, 8, 503')!=1:raise ValueError('Unexpected legacy overlay mask')
    asm.write_text(text.replace('andi. 8, 8, 503','andi. 8, 8, 4599'))
    link=(ROOT/'runtime/runtime.ld').read_text().replace('*(.text)','*(.text*)').replace('*(.data)','*(.data*) *(.rodata*) *(.bss*) *(.sdata*) *(.sbss*) *(COMMON)')
    for name,address in json.loads((dest/'aliases.json').read_text()).items():
        if name+' ='not in link:link+=f'\n{name} = {address:#x};\n'
    for name in PLACEHOLDERS:
        if name+' ='not in link:link+=f'\n{name} = 0x02900000;\n'
    (dest/'runtime.ld').write_text(link)
    for name,path in [('runtime',asm),('imports',ROOT/'runtime/imports.s')]:run([tool('gcc'),'-mcpu=750','-c','-x','assembler',path,'-o',dest/(name+'.o')])
    for name in ('morth_trick','morth_setup','morth_setup_data'):run([tool('gcc'),*FLAGS,'-c',ROOT/'runtime'/(name+'.c'),'-o',dest/(name+'.o')])
    run([tool('ld'),'--emit-relocs','-T',dest/'runtime.ld',*[dest/(n+'.o')for n in ('runtime','morth_trick','morth_setup','morth_setup_data','imports')],'-o',dest/'runtime.elf'])
    run([sys.executable,ROOT/'tools/build_native_payload.py','--source',source,'--build-dir',dest,'--output',dest/'payload'])
    patched,proof=patch_rpx(raw,dest/'payload');(dest/'cking.rpx').write_bytes(patched)
    if original.read_bytes()!=raw:raise AssertionError('Input game changed')
    proof.update(original_sha256=digest,RPX_sha256=hashlib.sha256(patched).hexdigest(),ELF_sha256=hashlib.sha256((dest/'runtime.elf').read_bytes()).hexdigest(),input_unchanged=True)
    (dest/'build-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(proof,indent=2))
    # Opt-in is local to a maintainer checkout. Ordinary contributors' builds
    # never attempt to push to someone else's repository.
    git=shutil.which('git')
    enabled=subprocess.run([git,'config','--bool','--get','gz.autopublish'],cwd=ROOT,capture_output=True,text=True)if git else None
    if enabled and enabled.returncode==0 and enabled.stdout.strip()=='true':
        run([sys.executable,ROOT/'tools/publish.py','--push'])
if __name__=='__main__':main()
