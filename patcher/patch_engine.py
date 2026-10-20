import hashlib,json,struct
from pathlib import Path
from native_rpx import Elf,COMPRESSED
from native_relocations import add_address_relocations

def patch_rpx(raw,payload_dir):
 p=Path(payload_dir);m=json.loads((p/'manifest.json').read_text())
 if m.get('version')!=2 or 'address_relocations' not in m:raise ValueError('Native payload is missing address relocation support. Rebuild it before native testing.')
 if hashlib.sha256(raw).hexdigest()!=m['source_sha256']:raise ValueError('Unsupported or modified game. This native test requires the original WWHD EUR v0 RPX.')
 e=Elf(raw);e.verify_crcs();text=(p/'text.bin').read_bytes();data=(p/'data.bin').read_bytes()
 for name,blob in [('text',text),('data',data)]:
  if hashlib.sha256(blob).hexdigest()!=m[name+'_sha256']:raise ValueError('Patch payload failed verification.')
 for address,blob in [(m['text_address'],text),(m['data_address'],data)]:
  for s in e.sections:
   if s.address and s.header[2]&2 and address<s.address+(s.header[5] if s.kind==8 else len(s.data)) and address+len(blob)>s.address:raise ValueError('Native payload would overlap a game section.')
 for h in m['hooks']:
  _,s,o=e.section_at(h['address'])
  if int.from_bytes(s.data[o:o+4],'big')!=h['before']:raise ValueError('Hook signature mismatch.')
  e.write_word(h['address'],h['after'])
 addresses={h['address'] for h in m['hooks']}
 removed=0
 for s in e.sections:
  if s.kind!=4:continue
  entries=list(struct.iter_unpack('>III',s.data));kept=[v for v in entries if v[0]&~3 not in addresses]
  removed+=len(entries)-len(kept);s.data=b''.join(struct.pack('>III',*v) for v in kept)
 ti=e.add(text,m['text_address'],6|COMPRESSED)
 e.add(data,m['data_address'],3|COMPRESSED)
 rela=b''.join(struct.pack('>IIi',r['address'],r['symbol_index']<<8|10,r['addend']) for r in m['imports'])
 # Keep the new relocation table plain: native sExecReloc rejected the
 # compressed two-entry diagnostic with zlib_ret != Z_STREAM_END (-470055).
 e.add(rela,0,0,kind=4,link=28,info=ti,entrysize=12)
 refs=list(m['address_relocations'])
 # Function-pointer hooks in original tables must move with native TEXT too.
 for h in m['hooks']:
  if h['after']==m['symbols'][h['destination']]:refs.append(dict(address=h['address'],target=h['after'],kind=1,addend=0))
 relocation_report=add_address_relocations(e,refs)
 result=e.encode_rpx();check=Elf(result);check.verify_crcs()
 for h in m['hooks']:
  _,s,o=check.section_at(h['address']);assert int.from_bytes(s.data[o:o+4],'big')==h['after']
 assert check.section_at(m['text_address'],len(text))[1].data==text
 assert check.section_at(m['data_address'],len(data))[1].data==data
 return result,dict(passed=True,source_sha256=m['source_sha256'],modified_sha256=hashlib.sha256(result).hexdigest(),hooks=len(m['hooks']),removed_original_relocations=removed,import_relocations=len(m['imports']),native_hardware_tested=False,sections=len(check.sections),**relocation_report)
