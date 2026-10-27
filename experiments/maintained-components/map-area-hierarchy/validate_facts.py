"""Portable source-fact checks; no compiler, game or process access."""
from pathlib import Path
import hashlib,json,re
H=Path(__file__).resolve().parent
scope=json.loads((H/'scope.json').read_text())
for name,expected in scope['exported_source_sha256'].items():
    assert hashlib.sha256((H/name).read_bytes()).hexdigest()==expected,name
text=(H/'hierarchy.s').read_text()
def table(symbol):
    block=text.split(symbol+':',1)[1]
    result=[]
    for line in block.splitlines():
        line=line.strip()
        if not line or line.startswith('#'):
            continue
        if not line.startswith('.long '):
            break
        result.append([x.strip() for x in line[6:].split(',')])
    return result
roots=table('_gz_mah_roots');folders=table('_gz_mah_folders');rows=table('_gz_mah_rows')
assert (len(roots),len(folders),len(rows))==(49,12,230)
facts=json.loads((H/'hierarchy-facts.json').read_text())
folder_ids=[];leaves=0
for root,fact in zip(roots,facts['sectors']):
    first,count=map(int,root[1:]);assert 0<=first<=len(rows) and first+count<=len(rows)
    actual=[]
    for row in rows[first:first+count]:
        value=int(row[1],0)
        if value&0x80000000:
            group=value&0x7fffffff;assert group<12;folder_ids.append(group)
            actual.append(('folder',group))
        else:
            actual.append(('leaf',value));leaves+=1
    expected=[(r['kind'],r['folder'] if r['kind']=='folder' else r['warp']) for r in fact['root_rows']]
    assert actual==expected
    for folder in fact['folders']:
        first,count=map(int,folders[folder['id']][1:]);assert first+count<=len(rows)
        ids=[int(r[1],0) for r in rows[first:first+count]]
        assert all(not value&0x80000000 for value in ids)
        assert ids==[r['warp'] for r in folder['rows']];leaves+=len(ids)
assert sorted(folder_ids)==list(range(12)) and leaves==218
seams=json.loads((H/'runtime-seams.json').read_text())
assert seams['seam_count']==len(seams['seams'])==8 and seams['module_linked_separately']
assert all(s['before']!=s['after'] for s in seams['seams'])
print('Passed: 49 sectors, 12 folders, 218 original leaf IDs, eight separate runtime seams.')
