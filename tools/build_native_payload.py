import sys,json,struct,hashlib,re,argparse,os
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'patcher'))
from native_rpx import Elf,COMPRESSED,relative_branch
from native_relocations import collect_address_relocations
parser=argparse.ArgumentParser()
parser.add_argument('--build-dir',default='build')
parser.add_argument('--output',default='build/payload')
parser.add_argument('--source',default='build/patch_wwhdgz.asm')
parser.add_argument('--objects',nargs='+',default=['runtime.o','imports.o'],help='Every linked input object that contributes payload code/data')
args=parser.parse_args()
root=Path(args.build_dir);out=Path(args.output);out.mkdir(parents=True,exist_ok=True)
source_path=Path(args.source)
linked=Elf((root/'runtime.elf').read_bytes());linked_symbols={s['name']:s for s in linked.symbols()}
text=next(s for i,s in enumerate(linked.sections) if linked.name(i)=='.text');data=next(s for i,s in enumerate(linked.sections) if linked.name(i)=='.data')
original=Elf(Path(os.environ['WWHD_RPX']).read_bytes())
original.verify_crcs();symbols={s['name']:s for s in original.symbols()}
# The native CPU protects TEXT against writes; Cemu may allow a wrongly
# classified mutable table there. Validate the emitted object placement.
layout_path=root/'section-layout-audit.json'
layout_checked=0
if layout_path.exists():
 for block in json.loads(layout_path.read_text()):
  if block['code']:continue
  for name in block['names']:
   symbol=linked_symbols[name];section=linked.sections[symbol['section']]
   if not section.header[2]&1 or section.header[2]&4:
    raise ValueError('Native data symbol is not writable DATA: '+name)
   layout_checked+=1
for name in ('_gz_native_action_copy_control','_gz_native_action_copy_storage','_gz_native_action_calculate_context'):
 if name not in linked_symbols:continue  # Older builds did not have this renderer.
 symbol=linked_symbols[name];section=linked.sections[symbol['section']]
 if section.header[2]&3!=3 or section.header[2]&4:raise ValueError('Font state targets protected TEXT: '+name)
imports=[]
for obj in args.objects:
 elf=Elf((root/obj).read_bytes());syms=elf.symbols();by_index={s['index']:s for s in syms}
 for reloc_section in elf.sections:
  if reloc_section.kind!=4:continue
  section_index=reloc_section.header[7]
  anchors=[s for s in syms if s['section']==section_index and s['name'] in linked_symbols and s['name']]
  if not anchors:continue
  base=linked_symbols[anchors[0]['name']]['value']-anchors[0]['value']
  assert all(linked_symbols[s['name']]['value']-s['value']==base for s in anchors),(obj,'section moved')
  for offset,info,addend in struct.iter_unpack('>IIi',reloc_section.data):
   name=by_index[info>>8]['name'];kind=info&255
   if name in symbols and symbols[name]['section']>=14 and symbols[name]['section']<=27:
    assert kind==10,(name,kind)
    address=base+offset
    assert text.address<=address<text.address+len(text.data)
    imports.append(dict(address=address,symbol_index=symbols[name]['index'],name=name,addend=addend))
hooks=[]
for hook in json.loads((root/'hooks.json').read_text()):
 address=hook['address'];expression=hook['expression'];instruction,target=expression.split()
 _,section,off=original.section_at(address)
 before=int.from_bytes(section.data[off:off+4],'big');destination=linked_symbols[target]['value']
 after=destination if instruction=='.int' else relative_branch(address,destination,instruction in ('bl','bla'))
 hooks.append(dict(address=address,before=before,after=after,destination=target))
for name,s in [('text',text),('data',data)]:(out/(name+'.bin')).write_bytes(s.data)
address_relocations=collect_address_relocations(linked,[Elf((root/name).read_bytes()) for name in args.objects],text,data)
expected_addresses=set()
for section in linked.sections:
 if section.kind!=4 or not linked.sections[section.header[7]].header[2]&2:continue
 for offset,info,addend in struct.iter_unpack('>IIi',section.data):
  if info&255 in (1,4,6):expected_addresses.add((offset,info&255))
actual_addresses={(r['address'],r['kind'])for r in address_relocations}
if expected_addresses!=actual_addresses:raise ValueError('Linked address relocations are missing: pass every linked input via --objects')
source_assembly=source_path.read_text()
record_region=source_assembly.split('_gz_route_records:',1)[1].split('_gz_checkpoint_name0:',1)[0]
checkpoint_count=len(re.findall(r'^\.int ',record_region,re.M))
manifest=dict(format='wwhdgz-native-rpx-patch',version=2,title_id='0005000010143600',title_version=0,region='EUR',
 source_sha256='f9f461738949a09481dc1a31c01ad27db813c4c6058fdd7d015624a4146bbf0b',
 text_address=text.address,data_address=data.address,text_sha256=hashlib.sha256(text.data).hexdigest(),data_sha256=hashlib.sha256(data.data).hexdigest(),
 imports=imports,address_relocations=address_relocations,hooks=hooks,hardware_tested=False,menu_checkpoints=checkpoint_count,
 writable_data_symbols_checked=layout_checked,
 source_cemu_sha256=hashlib.sha256(source_path.read_bytes()).hexdigest(),
 native_changes=['Separate writable state and executable code','Real RPX import relocations','Native address fixups for state, embedded pointers and game aliases','Expanded symbol table moved into free loader space','Mapped section overlap rejection','Plain new relocation tables','Native dynamic GX2 export thunks','Three fenced CPU canvases; no OS screen-buffer calls','1279-wide linear texture with 1280-pixel pitch for Cemu refresh compatibility','GPU retirement guards reuse without per-overlay DrawDone waits','Native menu reduced about 20 percent','Relocated game counter for notifications and controller ownership','Cache-block canvas clear'],
 symbols={s['name']:s['value'] for s in linked.symbols() if s['name'].startswith(('_gz_','_native_'))})
(out/'manifest.json').write_text(json.dumps(manifest,indent=2))
print(json.dumps(dict(text_size=len(text.data),data_size=len(data.data),hooks=len(hooks),native_import_sites=len(imports),native_import_names=sorted({e['name'] for e in imports}))))
