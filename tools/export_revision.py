"""Export a validated local runtime build, omitting embedded game artwork."""
from pathlib import Path
import argparse, hashlib, json, re, shutil, struct, sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'patcher'))
from native_rpx import Elf
sha=lambda b:hashlib.sha256(b).hexdigest()
def export(candidate,revision):
    candidate=Path(candidate).resolve();build=candidate/'native-build'
    if not re.fullmatch(r'v\d+(?:[.-][A-Za-z0-9]+)*',revision):raise ValueError('Invalid revision')
    proof=json.loads((build/'private-native-build-proof.json').read_text())
    source=candidate/'patch_wwhdgz.asm';rpx=build/'cking.rpx';linked=build/'runtime.elf'
    if not proof['passed'] or proof['RPX_sha256']!=sha(rpx.read_bytes()) or proof['ELF_sha256']!=sha(linked.read_bytes()):raise ValueError('Build proof does not match candidate')
    if proof['source_assembly_sha256']!=sha(source.read_bytes()):raise ValueError('Source differs from validated build')
    elf=Elf(linked.read_bytes());symbols={s['name']:s['value']for s in elf.symbols()if s['name']}
    text=source.read_text();labels=list(re.finditer(r'(?m)^([A-Za-z_][A-Za-z_0-9]*):[ \t]*$',text));assets=[];edits=[]
    for i,m in enumerate(labels):
        name=m[1]
        if not(re.fullmatch(r'_gz_(?:font_glyph|outline_glyph)\d+',name)or name.startswith('_gz_inv_icon_')or name in('_gz_cursor_pixels','_gz_cursor_pixels21')):continue
        end=labels[i+1].start()if i+1<len(labels)else len(text);body=text[m.end():end]
        rows=list(re.finditer(r'(?m)^\.int[ \t]+([^\n]+)\n?',body))
        if not rows:raise ValueError('Unexpected asset format: '+name)
        first,last=rows[0].start(),rows[-1].end()
        if re.sub(r'(?m)^\.int[ \t]+[^\n]+\n?','',body[first:last]).strip():raise ValueError('Mixed asset code/data: '+name)
        words=[]
        for row in rows:
            if not re.fullmatch(r'(?:0x[\da-fA-F]+|-?\d+)(?:\s*,\s*(?:0x[\da-fA-F]+|-?\d+))*\s*',row[1]):raise ValueError('Nonliteral asset: '+name)
            words.extend(int(x.strip(),0)&0xffffffff for x in row[1].split(','))
        data=struct.pack('>'+str(len(words))+'I',*words);_,section,offset=elf.section_at(symbols[name],len(data))
        if section.data[offset:offset+len(data)]!=data:raise ValueError('Asset differs from linked output: '+name)
        assets.append(dict(name=name,address=symbols[name],bytes=len(data),sha256=sha(data),kind='game-derived pixel data; not included'))
        edits.append((m.end()+first,m.end()+last,'.asset '+name+'\n'))
    if not assets:raise ValueError('No local artwork blocks found; review exporter before publishing')
    for start,end,replacement in reversed(edits):text=text[:start]+replacement+text[end:]
    # Compiler line markers are comments to the assembly converter; discard
    # their local paths without changing emitted instructions or data.
    text=re.sub(r'(?m)^# \d+ "[^"\n]*"[^\n]*\n?','',text)
    # No workspace paths or diagnostic metadata are copied into this export.
    target=ROOT/'runtime';(target/'patch_wwhdgz.asm.in').write_text(text)
    for name in ('trainer.c','trainer.asm','songs.asm','morth_trick.c','morth_setup.c','morth_setup.h','morth_setup_data.c'):shutil.copy2(candidate/name,target/name)
    accepted=[sha(rpx.read_bytes())]
    prior=target/'local-assets.json'
    if prior.exists():
        old=json.loads(prior.read_text())
        if old.get('assets')==assets:accepted=list(dict.fromkeys(accepted+old['accepted_practice_RPX_sha256']))
    meta=dict(format='TWWHD-GZ-local-assets/1',source_revision=revision,accepted_practice_RPX_sha256=accepted,assets=assets)
    (target/'local-assets.json').write_text(json.dumps(meta,indent=2)+'\n')
    public=dict(revision=revision,source_sha256=sha(source.read_bytes()),ELF_sha256=sha(linked.read_bytes()),RPX_sha256=sha(rpx.read_bytes()),asset_blocks_omitted=len(assets),asset_bytes_omitted=sum(a['bytes']for a in assets),hardware_acceptance='See release notes; compiled checks do not establish full-game correctness.')
    (ROOT/'docs/revision.json').write_text(json.dumps(public,indent=2)+'\n')
    return public
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidate',required=True,type=Path);p.add_argument('--revision',required=True);a=p.parse_args()
    print(json.dumps(export(a.candidate,a.revision),indent=2))
