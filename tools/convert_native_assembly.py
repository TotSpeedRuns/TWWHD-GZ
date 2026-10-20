"""Convert our Cemu assembly to sectioned PPC ELF, retaining native API relocs."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import re,json,argparse
from native_menu_font import shrink_menu_font
from native_overlay_buffers import helpers
parser=argparse.ArgumentParser()
parser.add_argument('--source',default='build/patch_wwhdgz.asm')
parser.add_argument('--build-dir',default='build')
parser.add_argument('--relocate-game-literals',action='store_true')
args=parser.parse_args()
source=Path(args.source).read_text()
source=shrink_menu_font(source)
assert 'WWHDGZ_REAL_GAMEPLAY_TEST' not in source
hooks=[];aliases=[];lines=[];fixed_origin=None
def strip_comments(line):
 # Both comment styles occur in the maintained source. A comment after a
 # data table must never make that table look like executable instructions.
 quoted=False;escaped=False
 for i,ch in enumerate(line):
  if escaped:escaped=False;continue
  if ch=='\\':escaped=True;continue
  if ch=='"':quoted=not quoted
  elif not quoted and ch in (';','#'):return line[:i].strip()
 return line.strip()
for line in source.splitlines():
 line=strip_comments(line)
 if line.startswith('.origin'):
  value=line.split('=')[1].strip();fixed_origin=None if value=='codecave' else int(value,16)
  continue
 if not line or line.startswith('[') or line.startswith('moduleMatches'):continue
 if fixed_origin is not None:
  assert line.startswith('.int '),line
  hooks.append(dict(address=fixed_origin,expression=line));fixed_origin+=4
  continue
 match=re.fullmatch(r'(0x[\da-fA-F]+) = (.*)',line)
 if match:
  address=int(match[1],16);rhs=match[2]
  if rhs.endswith(':'):aliases.append(f'.equ {rhs[:-1]}, 0x{address:08x}')
  else:hooks.append(dict(address=address,expression=rhs))
  continue
 # Native canvas belongs to us; do not change Cafe OS screen buffers.
 line=line.replace('bl import.coreinit.OSScreenInit','nop').replace('bl import.coreinit.OSScreenSetBufferEx','nop')
 line=line.replace('bl import.coreinit.OSScreenGetBufferSizeEx','lis r3, 0xA8\nori r3, r3, 0xC000')
 line=line.replace('bl import.coreinit.OSScreenClearBufferEx','bl _native_clear_canvas')
 line=line.replace('bl import.coreinit.OSGetSystemTick','mftb r3')
 # Seal the draw and fence the canvas; no synchronous GPU wait per overlay.
 line=line.replace('bl import.gx2.GX2DrawEx','bl import.gx2.GX2DrawEx\nbl _native_submit_canvas')
 lines.extend(line.splitlines())
# Retain a 1279-pixel visible width with the API-calculated 1280-pixel pitch.
# This is a valid linear GX2 surface and keeps Cemu's upload hash refreshing.
text='\n'.join(lines)
# Upcharge alone is a gameplay tool and should not create a blank overlay.
visible='lis r8, _gz_tools_flags@ha\naddi r8, r8, _gz_tools_flags@l\nlwz r8, 0(r8)\ncmpwi r8, 0'
start=text.index('_gz_copy:');end=text.index('_gz_overlay_visible:',start)
block=text[start:end]
assert block.count(visible)==1
block=block.replace(visible,visible.replace('cmpwi r8, 0','andi. r8, r8, 503\ncmpwi r8, 0'))
# Draw flags1/2/4/16/32/64/128/256, excluding input-only Upcharge bit8.
# Spin and Chain Roll must remain visible after closing the practice menu.
text=text[:start]+block+text[end:]
assert text.count('_gz_draw:\n')==1
text=text.replace('_gz_draw:\n','_gz_draw:\nbl _native_acquire_canvas\ncmpwi r3, 0\nbeq _gz_original_copy\n',1)
# Native DATA moves. These literal reads bypassed the retained object
# relocations and froze toast/repeat/controller-ownership timers.
text,counter_reads=re.subn(r'lis r(\d+), 0x1020\nlwz r(\d+), -0xAA8\(r\1\)',
 lambda m:f'lis r{m[1]}, _gz_game_counter@ha\nlwz r{m[2]}, _gz_game_counter@l(r{m[1]})',text)
assert counter_reads>=14,counter_reads
assert not re.search(r'lis r\d+, 0x1020\nlwz r\d+, -0xAA8\(r\d+\)',text)
# Recreate the overwritten original counter high-half with a relocation.
# The original continuation's low-half is already relocated by Cafe OS.
assert text.count('lis r10, 0x1020\nb _gz_counter_rest')==1
text=text.replace('lis r10, 0x1020\nb _gz_counter_rest','lis r10, _gz_game_counter@ha\nb _gz_counter_rest')
aliases.append('.equ _gz_game_counter, 0x101ff558')
literal_records=[]
if args.relocate_game_literals:
 from native_literal_address_fixups import fix_game_literals
 text,literal_aliases,literal_records=fix_game_literals(text)
 aliases.extend(literal_aliases)
imports=sorted(set(re.findall(r'import\.\w+\.(\w+)',text))|{'GX2Flush','GX2GetRetiredTimeStamp','GX2GetLastSubmittedTimeStamp'})
for module,name in set(re.findall(r'import\.(\w+)\.(\w+)',text)):text=text.replace(f'import.{module}.{name}',name)
blocks=re.split(r'(?m)^(?=\w+:)',text)
result=[]
branch_targets=set(re.findall(r'(?m)^\s*b\w*[+-]?\s+(?:(?:cr[0-7]|[0-9]+),\s*)?(_gz_[\w.]+)\s*$',text))
types=[];next_code=False;layout=[]
for block in reversed(blocks):
 tokens=[l.strip() for l in block.strip().splitlines()]
 names=[l[:-1] for l in tokens if l.endswith(':')]
 instructions=any(l and not l.endswith(':') and not l.startswith('.') for l in tokens)
 data=any(l.startswith(('.int','.short','.float','.string','.byte','.long','.double','.space','.skip','.zero')) for l in tokens)
 is_code=instructions or bool(set(names)&branch_targets) or (not data and next_code)
 types.append(is_code);next_code=is_code
 layout.append(dict(names=names,code=is_code,instructions=instructions,data=data))
types.reverse()
layout.reverse()
for block,is_code in zip(blocks,types):
 b=block.strip()
 if not b:continue
 tokens=[l.strip() for l in b.splitlines()]
 result.append('.section .text,"ax",@progbits' if is_code else '.section .data,"aw",@progbits')
 result.append('.balign 4')
 # PPC GNU/LLVM register syntax and byte-alignment directives.
 for line in tokens:
  if line.endswith(':'):result.append('.global '+line[:-1])
  line=re.sub(r'\b[rf](\d+)\b',r'\1',line)
  line=line.replace('.align','.balign').replace('.int ','.long ')
  m=re.fullmatch(r'li (\d+), (\d+)',line)
  if m and 32767<int(m[2])<=65535:line=f'li {m[1]}, {int(m[2])-65536}'
  result.append(line)
# Dedicated clear uses volatile registers only, native no screen API side effects.
result+=['.section .text,"ax",@progbits','.balign 4','.global _native_clear_canvas','_native_clear_canvas:',
 'lis 8, _gz_state@ha','addi 8, 8, _gz_state@l','lwz 8, 12(8)',
 # Cache blocks are 32 bytes; the heap canvas is 256-byte aligned and its
 # 0x384000-byte size is a multiple of 32. GX2Invalidate stores it for GPU use.
 'lis 9, 0x1','ori 9, 9, 0xC200','mtctr 9',
 '_native_clear_loop:','dcbz 0, 8','addi 8, 8, 32','bdnz _native_clear_loop','blr']
result.append(helpers())
root=Path(args.build_dir);root.mkdir(parents=True,exist_ok=True)
(root/'runtime.s').write_text('\n'.join(result)+'\n')
(root/'hooks.json').write_text(json.dumps(hooks,indent=2))
(root/'imports.json').write_text(json.dumps(imports,indent=2))
(root/'aliases.json').write_text(json.dumps({l.split()[1].rstrip(','):int(l.split()[-1],16) for l in aliases},indent=2))
(root/'literal-address-audit.json').write_text(json.dumps(literal_records,indent=2))
(root/'section-layout-audit.json').write_text(json.dumps(layout,indent=2))
print('Converted native assembly:',len(hooks),'hooks,',len(imports),'native API calls')
