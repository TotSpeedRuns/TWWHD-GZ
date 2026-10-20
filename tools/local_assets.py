"""Keep game-derived pixels outside the source repository."""
from pathlib import Path
import argparse, hashlib, json, re, struct, sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'patcher'))
from native_rpx import Elf
sha=lambda b:hashlib.sha256(b).hexdigest()

def metadata(path):
    meta=json.loads(Path(path).read_text())
    if meta['format']!='TWWHD-GZ-local-assets/1':raise ValueError('Unsupported asset manifest')
    names=set()
    for a in meta['assets']:
        if not re.fullmatch(r'_gz_[A-Za-z0-9_]+',a['name']) or a['name'] in names:raise ValueError('Invalid/duplicate asset name')
        if not 0<a['bytes']<=4*1024*1024 or a['bytes']%4:raise ValueError('Invalid asset size')
        names.add(a['name'])
    return meta

def extract(rpx, destination, manifest):
    raw=Path(rpx).read_bytes();meta=metadata(manifest)
    if sha(raw) not in meta['accepted_practice_RPX_sha256']:raise ValueError('Use the practice RPX matching this source revision')
    elf=Elf(raw);elf.verify_crcs();chunks=[]
    for a in meta['assets']:
        _,section,offset=elf.section_at(a['address'],a['bytes'])
        data=section.data[offset:offset+a['bytes']]
        if sha(data)!=a['sha256']:raise ValueError('Asset hash mismatch: '+a['name'])
        chunks.append((a['name'],data))
    dest=Path(destination);dest.mkdir(parents=True,exist_ok=True)
    for name,data in chunks:(dest/(name+'.bin')).write_bytes(data)
    return len(chunks)

def materialize(template, assets, manifest):
    meta=metadata(manifest);known={a['name']:a for a in meta['assets']};seen=set()
    def replace(m):
        name=m[1]
        if name not in known or name in seen:raise ValueError('Unknown/duplicate asset directive: '+name)
        seen.add(name);a=known[name];data=(Path(assets)/(name+'.bin')).read_bytes()
        if len(data)!=a['bytes'] or sha(data)!=a['sha256']:raise ValueError('Local asset mismatch: '+name)
        words=struct.unpack('>'+str(len(data)//4)+'I',data)
        return ''.join('.int '+','.join('0x%08X'%v for v in words[i:i+8])+'\n' for i in range(0,len(words),8))
    text=re.sub(r'(?m)^\.asset ([A-Za-z_][A-Za-z0-9_]*)\s*$',replace,Path(template).read_text())
    if seen!=set(known):raise ValueError('Manifest and template assets differ')
    return text

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--practice-rpx',required=True)
    p.add_argument('--destination',type=Path,default=ROOT/'local-assets')
    p.add_argument('--manifest',type=Path,default=ROOT/'runtime/local-assets.json');a=p.parse_args()
    print('Extracted',extract(a.practice_rpx,a.destination,a.manifest),'local-only asset blocks')
