"""Replace literal game pointer materialization with ELF relocation symbols."""
import re,sys,json,os
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'patcher'))
from native_rpx import Elf

def fix_game_literals(text):
    original=Elf(Path(os.environ['WWHD_RPX']).read_bytes())
    mapped=[(s.address,s.address+(s.header[5] if s.kind==8 else len(s.data))) for s in original.sections if s.header[2]&2 and s.address<0xc0000000]
    def valid(address):return any(a<=address<b for a,b in mapped)
    records=[];aliases={}
    def label(address):
        name='_native_game_literal_'+format(address,'08x');aliases[name]=address;return name
    # Convert both halves; @ha/addi correctly accounts for low-half carry after
    # Cafe OS moves DATA/TEXT. Keep constants outside actual game sections.
    pattern=r'(?m)^lis r(\d+), (0x[0-9a-fA-F]+)\n(ori|addi) r(\d+), r\1, (0x[0-9a-fA-F]+|-?\d+)$'
    def pair(m):
        hi=int(m[2],0);lo=int(m[5],0)
        address=((hi<<16)|lo) if m[3]=='ori' else ((hi<<16)+(lo if lo<32768 else lo-65536))&0xffffffff
        if not valid(address):return m[0]
        name=label(address);records.append(dict(address=address,instruction=m[0],kind='materialized pointer'))
        return f'lis r{m[1]}, {name}@ha\naddi r{m[4]}, r{m[1]}, {name}@l'
    text=re.sub(pattern,pair,text)
    pattern=r'(?m)^lis r(\d+), (0x[0-9a-fA-F]+)\n(lwz|lbz|lhz|lha|lfs|lfd|stw|stb|sth|stfs|stfd) ([rf]\d+), (-?0x[0-9a-fA-F]+|-?\d+)\(r\1\)$'
    def access(m):
        hi=int(m[2],0);lo=int(m[5],0);lo=lo if lo<32768 else lo-65536;address=((hi<<16)+lo)&0xffffffff
        if not valid(address):return m[0]
        name=label(address);records.append(dict(address=address,instruction=m[0],kind='direct game memory'))
        return f'lis r{m[1]}, {name}@ha\n{m[3]} {m[4]}, {name}@l(r{m[1]})'
    text=re.sub(pattern,access,text)
    assert any(r['address']==0x026f9550 for r in records),'Known native crash return was not covered'
    return text,[f'.equ {name}, 0x{address:08x}' for name,address in aliases.items()],records
