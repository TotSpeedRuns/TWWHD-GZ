"""Add validated location warps to the exact accepted R755 runtime.

Existing warp IDs, route payloads and all restore code are preserved. Explicitly
audited bad entrance tuples are repaired without renumbering their records.
Only the warp-count bound and text-browser root count change in executable code.
"""
from pathlib import Path
import argparse,hashlib,json,re,struct,subprocess,sys
H=Path(__file__).resolve().parent;R=H.parents[1]
SDK=R/'work/aroma-sdk/opt/devkitpro/devkitPPC/bin'
sys.path.insert(0,str(R/'outputs/wwhdgz/WiiU/Patcher'))
from native_rpx import Elf
BASE=R/'work/quarter-heart-pieces-candidate-20261026'
SOURCE=BASE/'runtime-personal.s'
def pin(p):
    p=Path(p).resolve();b=p.read_bytes()
    return dict(path=str(p),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def main():
    p=argparse.ArgumentParser();p.add_argument('--decisions',required=True,type=Path);a=p.parse_args()
    assert not(H/'staging.json').exists()
    document=json.loads(a.decisions.read_text());decisions=document['rows']
    plan=json.loads((H/'reference-plan.json').read_text())['rows']
    assert len(plan)==len(decisions)==174
    assert [x['line']for x in plan]==[x['line']for x in decisions]
    old=SOURCE.read_text();s=old;seams=[]
    assert pin(SOURCE)['sha256']=='6664b1a9f6bca42dc80192600f1004fb6a5e6dc0c64da1edfa98e72cfa27f6b2'
    names={m[0]:json.loads(m[1])for m in re.findall(r'^(\w+):\n\.string (.+)$',s,re.M)}
    def table(symbol):
        start=s.index(symbol+':\n')+len(symbol)+2;stop=start;rows=[]
        for line in s[start:].splitlines(keepends=True):
            if not line.startswith('.long '):break
            rows.append(line[6:].strip().split(','));stop+=len(line)
        return start,stop,rows
    def replace(before,after,name):
        nonlocal s
        assert s.count(before)==1,(name,s.count(before))
        seams.append(dict(name=name,before=before,after=after));s=s.replace(before,after,1)
    def replace_table(symbol,rows):
        start,stop,_=table(symbol)
        replace(s[start:stop],''.join('.long '+','.join(map(str,row))+'\n'for row in rows),symbol)
    _,_,warps=table('_gz_warp_records');original_warps=[r.copy()for r in warps]
    _,_,folders=table('_gz_folder_records');original_folders=[r.copy()for r in folders]
    _,_,quadrants=table('_gz_map_quadrants');_,_,destinations=table('_gz_map_destinations')
    assert len(warps)==136 and len(folders)==62 and len(quadrants)==49 and len(destinations)==133
    def key(row):return tuple(int(x,0)for x in row[1:4])
    def pack(stage,room,spawn,layer):
        assert 1<=len(stage)<=8 and stage.isascii() and all(0<=x<=255 for x in (room,spawn,layer))
        return (*struct.unpack('>II',stage.encode().ljust(8,b'\0')),spawn<<16|room<<8|layer)
    strings=[]
    def label(text):
        assert text and len(text)<=70 and text.isascii()
        n=f'_gz_tw_label{len(strings)}';strings.append((n,text));return n
    repairs=document.get('current_repairs',[])
    assert len({r['warp']for r in repairs})==len(repairs)
    expected_old=[row.copy()for row in original_warps]
    for repair in repairs:
        index=repair['warp'];assert 0<=index<136 and repair.get('evidence')
        before=repair['before'];after=repair['after']
        assert key(warps[index])==pack(**before),(index,before,warps[index])
        assert before!=after
        warps[index][1:4]=[f'0x{x:08X}'for x in pack(**after)]
        if repair.get('label'):warps[index][0]=label(repair['label'])
        expected_old[index]=warps[index].copy()
    by_key={key(r):i for i,r in reversed(list(enumerate(warps)))}
    groups={i:list(range(int(row[1]),int(row[1])+int(row[2])))for i,row in enumerate(folders)if int(row[3])==0}
    maps=[[list(x)for x in destinations[int(q[1]):int(q[1])+int(q[2])]]for q in quadrants]
    for entries in maps:
        for row in entries:
            repaired=next((r for r in repairs if r['warp']==int(row[1])and r.get('label')),None)
            if repaired:row[0]=label(repaired['label'])
    folder_ids={}
    for i,row in enumerate(folders):folder_ids.setdefault(names[row[0]],[]).append(i)
    def target(folder):
        ids=folder_ids[folder]
        # FF has both the sea-island leaf and dungeon interiors; append to dungeon leaf.
        return ids[-1]
    changes=[];hyrule=[]
    short={'Dragon Roost Cavern':'DRC','Tower of the Gods':'TotG','Forbidden Woods':'FW','Earth Temple':'ET','Wind Temple':'WT',"Ganon's Tower":'Ganon Tower'}
    for planned,decision in zip(plan,decisions):
        line=planned['line'];status=decision['status']
        assert status in {'supported','existing','contextual','excluded','unresolved'},(line,status)
        if status not in {'supported','existing'}:
            changes.append(dict(line=line,status=status,label=planned['reference_label'],reason=decision.get('reason','')));continue
        assert planned['status']=='pending_owned_check'
        assert decision.get('evidence'),(line,'owned evidence required')
        stage=decision['stage'];room=decision['room'];spawn=decision['spawn'];layer=decision['layer']
        tup=pack(stage,room,spawn,layer);was=tup in by_key
        name=decision.get('label',planned['label']).strip()
        own_prefix={'Dragon Roost Cavern':'DRC','Forbidden Woods':'FW','Tower of the Gods':'TOTG','Earth Temple':'ET','Wind Temple':'WT'}.get(planned['folder'])
        if own_prefix:name=re.sub(r'^'+own_prefix+r'\s+','',name,flags=re.I)
        if tup in by_key:wid=by_key[tup]
        else:
            wid=len(warps);by_key[tup]=wid
            warps.append([label(name),*(f'0x{x:08X}'for x in tup),'0'])
        folder=planned['folder'];fid=target(folder)
        if wid not in groups[fid]:groups[fid].append(wid)
        sector=planned['sector'] if folder!='Ghost Ship' else None
        if sector is not None:
            entries=maps[sector-1]
            if not any(int(x[1])==wid for x in entries):
                title=name if folder not in short and folder!='Hyrule' else short.get(folder,'Hyrule')+' / '+name
                entries.append([label(title),str(wid)])
        if folder in {'Hyrule',"Ganon's Tower"} and wid not in hyrule:hyrule.append(wid)
        changes.append(dict(line=line,status='existing'if was else'added',label=planned['reference_label'],display=name,folder=folder,sector=sector,warp=wid,tuple=dict(stage=stage,room=room,spawn=spawn,layer=layer),evidence=decision['evidence']))
    assert not any(x['status']=='unresolved'for x in changes),'Resolve all reference lines before building.'
    def unique_ids(ids):
        seen=set();result=[]
        for wid in ids:
            k=key(warps[wid])
            if k not in seen:seen.add(k);result.append(wid)
        return result
    groups={fid:unique_ids(ids)for fid,ids in groups.items()}
    for sector,entries in enumerate(maps):
        seen=set();result=[]
        for entry in entries:
            k=key(warps[int(entry[1])])
            if k not in seen:seen.add(k);result.append(entry)
        maps[sector]=result
    hyrule=unique_ids(hyrule)
    # Keep original warp IDs stable; rebuild only changed leaf ranges at the end.
    for fid,ids in groups.items():
        previous=list(range(int(original_folders[fid][1]),int(original_folders[fid][1])+int(original_folders[fid][2])))
        if ids==previous:continue
        offset=len(warps);warps.extend(warps[i].copy()for i in ids)
        folders[fid][1:3]=[str(offset),str(len(ids))]
    # Hyrule is an explicit third browser root, containing castle and tower destinations.
    assert len(hyrule)>=20
    offset=len(warps);warps.extend(warps[i].copy()for i in hyrule)
    root=[label('Hyrule'),str(offset),str(len(hyrule)),'0']
    for row in folders:
        if int(row[3])==1 and int(row[1])>=2:row[1]=str(int(row[1])+1)
    folders.insert(2,root)
    newdest=[]
    for q,entries in zip(quadrants,maps):
        q[1:3]=[str(len(newdest)),str(len(entries))];newdest.extend(entries)
    assert warps[:136]==expected_old
    replace_table('_gz_warp_records',warps);replace_table('_gz_folder_records',folders)
    replace_table('_gz_map_quadrants',quadrants);replace_table('_gz_map_destinations',newdest)
    replace('cmplwi 8, 136\n','cmplwi 8, '+str(len(warps))+'\n','warp bound')
    before='_gz_setup_folders:\n';start=s.index(before);end=s.index('_gz_routes_back_root:',start)
    oldcode=s[start:end];newcode=oldcode.replace('li 10, 2\nstw 10, 0(9)','li 10, 3\nstw 10, 0(9)',1)
    assert newcode!=oldcode;replace(oldcode,newcode,'three browser roots')
    replace('_gz_list_count:\n.long 2\n','_gz_list_count:\n.long 3\n','initial root count')
    appendix='\n.section .data,"aw",@progbits\n'+''.join('.balign 4\n.global '+n+'\n'+n+':\n.string '+json.dumps(t)+'\n'for n,t in strings)
    s+=appendix
    reverse=s[:-len(appendix)]
    for seam in reversed(seams):assert reverse.count(seam['after'])==1;reverse=reverse.replace(seam['after'],seam['before'],1)
    assert reverse==old
    (H/'runtime-personal.s').write_text(s,newline='\n')
    visible='_gz_ccv_visible:\n.long 1\n';assert s.count(visible)==1
    (H/'runtime-public.s').write_text(s.replace(visible,'_gz_ccv_visible:\n.long 0\n',1),newline='\n')
    data=dict(schema='teotia-native-warps-r757-staging/1',staged=True,source=pin(SOURCE),decisions=pin(a.decisions),reference=pin(H/'aroma.yaml'),seams=seams,appended_data=appendix,old_warp_count=136,new_warp_count=len(warps),unique_tuple_count=len(by_key),old_map_count=133,new_map_count=len(newdest),old_folder_count=62,new_folder_count=len(folders),hyrule_count=len(hyrule),changes=changes,current_repairs=repairs,warps=warps,folders=folders,quadrants=quadrants,destinations=newdest,old_warp_IDs_preserved=True,only_authorized_old_tuple_repairs=True,route_payloads_preserved=True,new_heap_bytes=0,new_hooks=0,new_imports=0,only_executable_changes=['warp count immediate','root count immediate'],hardware_tested=False)
    (H/'staging.json').write_text(json.dumps(data,indent=2)+'\n')
    print(json.dumps({k:data[k]for k in ('new_warp_count','unique_tuple_count','new_map_count','new_folder_count','hyrule_count')}))
if __name__=='__main__':main()
