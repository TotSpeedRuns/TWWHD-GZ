"""Recreate the bounded Setup175 profile from exact owned inputs.

This is not a full Setup/game source build. No binary inputs are distributed.
"""
from pathlib import Path
import argparse,copy,hashlib,json,os,struct,sys,types,zlib
if not __debug__:raise RuntimeError('Run without optimized Python.')
sys.dont_write_bytecode=True
SOURCE=Path(__file__).resolve().parents[1]
REL='experiments/maintained-components/setup-profile-v175'
COMP=SOURCE/REL
PROFILE_SHA='196e2e25bf80623a359777a4bddfdb8131ef6b49176e0c310daf24b05a508c81'
PROFILE=json.loads((COMP/'profile.json').read_bytes())
PARENT_RPX=PROFILE['parent']['root_rpx_sha256']
PARENT_WUHB=PROFILE['parent']['WUHB_sha256']
PRIOR_RPX=PROFILE['parent']['metadata']['target_game_rpx_sha256']
PRIOR_TITLE=PROFILE['parent']['metadata']['target_game_title_sha256']
OLD_PRIOR_RPX='f69ed34ef90d5a20d067f8b0c3303afb21218f5e2a7662af9969f26eaa207a6a'
OLD_PRIOR_TITLE='a51df84be19f4aba4f9f4fd6fede3b41d257a7adeb495d3f11f90e4ffdc6d630'
REVISION=PROFILE['revision'].encode('ascii')

def sha(b):return hashlib.sha256(b).hexdigest()
def need(v,m):
 if not v:raise ValueError(m)
def enc(v):return(json.dumps(v,indent=2)+'\n').encode()
def save(p,b):
 with p.open('xb')as f:f.write(b);f.flush();os.fsync(f.fileno())
 need(p.read_bytes()==b,'Saved readback mismatch')

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('--repository',type=Path,required=True)
 ap.add_argument('--setup174',type=Path,required=True)
 ap.add_argument('--game-rpx',type=Path,required=True);ap.add_argument('--game-title',type=Path,required=True)
 ap.add_argument('--original-rpx',type=Path,required=True);ap.add_argument('--original-title',type=Path,required=True)
 ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 REPO=a.repository.resolve(strict=True);PARENT=a.setup174.resolve(strict=True);out=a.output.resolve()
 need(not out.exists()and out.parent.is_dir()and not any(out.is_relative_to(p)for p in(SOURCE,REPO,PARENT)),'Fresh output outside sources and input trees required')
 need(sha((COMP/'profile.json').read_bytes())==PROFILE_SHA,'Changed selected profile')
 manifest=json.loads((COMP/'source-manifest.json').read_bytes())
 for r in manifest['files']:
  p=SOURCE/r['path'];need(p.resolve().is_relative_to(SOURCE)and not p.is_symlink(),'Contained source required')
  data=p.read_bytes();need(len(data)==r['bytes']and sha(data)==r['sha256'],'Changed authored source')
 actual={p.relative_to(SOURCE).as_posix()for p in COMP.rglob('*')if p.is_file()}
 expected={r['path']for r in manifest['files']if r['path'].startswith(REL+'/')}|{REL+'/source-manifest.json'}
 need(actual==expected,'Unexpected component file')
 inputs={}
 def read(p,pin=None):
  need(not p.is_symlink(),'Symlink input refused');p=p.resolve(strict=True)
  need(p.is_file(),'Regular owned input required')
  raw=p.read_bytes();need(pin is None or sha(raw)==pin,'Changed pinned input: '+str(p))
  inputs[p]=dict(name=p.name,bytes=len(raw),sha256=sha(raw));return raw
 modules={}
 for relative,record in PROFILE['published_dependencies']['files'].items():
  data=read(REPO/relative,record['sha256']);need(len(data)==record['bytes'],'Published helper size')
  modules[Path(relative).stem]=(REPO/relative,data)
 for name in('setup175_bundle','setup175_recipes'):
  p=COMP/'helpers'/(name+'.py');modules[name]=(p,p.read_bytes())
 # Compile verified source bytes directly, avoiding any local .pyc cache.
 for name in('native_rpx','title_engine','setup175_bundle','setup175_recipes'):
  p,data=modules[name];m=types.ModuleType(name);m.__file__=str(p);sys.modules[name]=m
  exec(compile(data,str(p),'exec'),m.__dict__)
 from native_rpx import Elf,COMPRESSED
 from setup175_bundle import bundle
 from setup175_recipes import make_rpx_recipe,make_title_recipe,apply_rpx_recipe,apply_title_recipe,SOURCE_RPX,SOURCE_TITLE
 from title_engine import unpack
 old=PROFILE['parent']['metadata']
 need(old['passed_structural_checks']and old['setup_version']==174 and old['game_version']==173 and old['hook_count']==5,'Checked Setup174 parent shape')
 wuhb=read(PARENT/'GZSetup.wuhb',PARENT_WUHB);header,resources=bundle(wuhb)
 raw=next(f['data']for f in resources if f['name']=='root.rpx');need(sha(raw)==PARENT_RPX,'Exact embedded Setup174 root')
 target=read(a.game_rpx,PROFILE['target']['game_rpx_sha256']);title=read(a.game_title,PROFILE['target']['game_title_sha256'])
 trpx=sha(target);ttitle=sha(title)
 game=Elf(target);game.verify_crcs();need(len(game.sections)<=256,'Native recipe section cap')
 need(sum(s.data.count(b'GZ Menu\0')for s in game.sections)==1,'Preserved actual GZ Menu heading')
 title_raw=unpack(title);need(title_raw.count('v175'.encode('utf-16be'))==1 and 'v173'.encode('utf-16be')not in title_raw,'Actual Game175 title label')
 original=read(a.original_rpx,SOURCE_RPX)
 original_title=read(a.original_title,SOURCE_TITLE)
 pp={r['name']:r for r in old['payloads']}
 previous_recipe=read(PARENT/'practice.gzd',pp['practice.gzd']['sha256']);previous_title_recipe=read(PARENT/'title.gzt',pp['title.gzt']['sha256'])
 need(sha(apply_rpx_recipe(original,previous_recipe))==PRIOR_RPX and sha(apply_title_recipe(original_title,previous_title_recipe))==PRIOR_TITLE,'Parent recipes reconstruct Game173')
 recipe,stats=make_rpx_recipe(original,target);title_recipe,title_stats=make_title_recipe(original_title,title)
 need(apply_rpx_recipe(original,recipe)==target and apply_title_recipe(original_title,title_recipe)==title,'Exact original-EUR to Game175 recipe round trip')
 clean=dict(cleanup_seam=PROFILE['cleanup_seam'])
 image=Elf(raw);image.verify_crcs();final=copy.deepcopy(image);patches=[]
 def patch(address,before,after,label):
  need(len(before)==len(after),'Same-width profile patch required')
  i,s,o=image.section_at(address,len(before));need(s.data[o:o+len(before)]==before,'Exact parent profile: '+label)
  need(not any(r['section']==i and max(o,r['offset'])<min(o+len(before),r['offset']+len(bytes.fromhex(r['before'])))for r in patches),'Overlapping profile patch')
  b=bytearray(final.sections[i].data);b[o:o+len(before)]=after;final.sections[i].data=bytes(b)
  patches.append(dict(address=address,section=i,offset=o,label=label,before=before.hex(),after=after.hex()))
 need(len(REVISION)==25 and all(32<=x<127 for x in REVISION),'Exact25-printable-byte Native label')
 for address,before,after,label in [
  (0x10000cb8,b'v174 USB Test / Game v173',REVISION,'Setup175 and Game175 revision'),
  (0x10000ce8,PRIOR_RPX.encode(),trpx.encode(),'selected Game175 RPX'),
  (0x10000d70,PRIOR_TITLE.encode(),ttitle.encode(),'selected Game175 title'),
  (0x100031d4,sha(previous_title_recipe).encode(),sha(title_recipe).encode(),'selected title recipe'),
  (0x10001b48,OLD_PRIOR_RPX.encode(),PRIOR_RPX.encode(),'Native known-prior173 RPX'),
  (0x1000100c,OLD_PRIOR_TITLE.encode(),PRIOR_TITLE.encode(),'Native known-prior173 title'),
  (0x10000464,('Title_00-v173-'+PRIOR_TITLE[:12]+'.upload').encode(),('Title_00-v175-'+ttitle[:12]+'.upload').encode(),'title upload stage'),
  (0x10000488,b'Title_00-before-v173-',b'Title_00-before-v175-','title backup'),
  (0x10003248,b'/Title_00-v173-',b'/Title_00-v175-','title candidate'),
  (0x10004788,b'/code/cking-v173-',b'/code/cking-v175-','RPX stage'),
  (0x100047a4,b'/code/cking-before-v173-',b'/code/cking-before-v175-','RPX backup'),
 ]:patch(address,before,after,label)
 for name,before,after in [('gz_cleanup_current_rpx_sha',PRIOR_RPX,trpx),('gz_cleanup_current_title_sha',PRIOR_TITLE,ttitle)]:
  obj=PROFILE['cleanup_current_objects'][name];need(obj['bytes']==32 and obj['domain']=='CODE','Qualified current-digest object metadata')
  patch(obj['address'],bytes.fromhex(before),bytes.fromhex(after),name)
 for before,after in [(b'Auto-cleanup173:',b'Auto-cleanup175:'),(b'Game173',b'Game175')]:
  code=image.sections[1].data;at=0;count=0
  while True:
   at=code.find(before,at)
   if at<0:break
   address=image.sections[1].address+at;need(address>=clean['cleanup_seam'],'Owned cleanup log literal only')
   patch(address,before,after,'cleanup current log label');at+=len(before);count+=1
  need(count>0,'Existing cleanup log literal required')
 crc=next(i for i,s in enumerate(final.sections)if s.kind==0x80000003)
 final.sections[crc].data=b''.join(struct.pack('>I',0 if i==crc or s.kind in(0,8)else zlib.crc32(s.data)&0xffffffff)for i,s in enumerate(final.sections))
 first=min(s.header[4]for s in image.sections if s.kind not in(0,8)and s.header[5]);result=bytearray(raw[:first]);table=struct.unpack_from('>I',raw,32)[0]
 for i in sorted((i for i,s in enumerate(image.sections)if s.kind not in(0,8)and s.header[5]),key=lambda i:image.sections[i].header[4]):
  s=final.sections[i];result.extend(bytes((-len(result))%64));at=len(result)
  physical=raw[image.sections[i].header[4]:image.sections[i].header[4]+image.sections[i].header[5]]if s.data==image.sections[i].data else struct.pack('>I',len(s.data))+zlib.compress(s.data,9)if s.header[2]&COMPRESSED else s.data
  result.extend(physical);struct.pack_into('>II',result,table+40*i+16,at,len(physical))
 result.extend(bytes((-len(result))%64));packed=bytes(result);checked=Elf(packed);checked.verify_crcs()
 need([i for i,(x,y)in enumerate(zip(image.sections,checked.sections))if x.data!=y.data]==[1,3,crc],'Only profile/cleanup identities+labels/CRC changed')
 for i,(x,y)in enumerate(zip(final.sections,checked.sections)):
  need(x.data==y.data and x.header[:4]+x.header[6:]==y.header[:4]+y.header[6:],'Logical section/header changed')
  if i!=crc:
   restored=bytearray(y.data)
   for r in patches:
    if r['section']==i:
     o=r['offset'];before=bytes.fromhex(r['before']);restored[o:o+len(before)]=before
   need(bytes(restored)==image.sections[i].data,'Unplanned logical change')
 need(image.header==checked.header,'ELF header unchanged')
 need(all(x.data==y.data for x,y in zip(image.sections,checked.sections)if x.kind==4),'Every existing typed loader row unchanged')
 need(all(x.data==y.data for x,y in zip(image.sections,checked.sections)if x.kind in(0x80000001,0x80000002,0x80000004)),'Imports and FileInfo unchanged')
 for h in old['hooks']:
  _,before,bo=image.section_at(h['site'],4);_,after,ao=checked.section_at(h['site'],4)
  need(after.data[ao:ao+4]==before.data[bo:bo+4],'Existing USB/role hook unchanged')
 part=header[-1];rebuilt=bytearray(wuhb[:part]);offsets=[]
 for f in resources:rebuilt.extend(bytes((-len(rebuilt))%16));offsets.append(len(rebuilt)-part);rebuilt.extend(packed if f['name']=='root.rpx'else f['data'])
 rebuilt.extend(bytes((-len(rebuilt))%4));shift=len(rebuilt)-header[2];rebuilt.extend(wuhb[header[2]:])
 for at in(8,24,40,56):struct.pack_into('>Q',rebuilt,at,struct.unpack_from('>Q',wuhb,at)[0]+shift)
 for f,offset in zip(resources,offsets):struct.pack_into('>QQ',rebuilt,f['table_offset']+shift+8,offset,len(packed)if f['name']=='root.rpx'else f['size'])
 _,roundtrip=bundle(rebuilt);need(all(x['name']==y['name']and y['data']==(packed if x['name']=='root.rpx'else x['data'])for x,y in zip(resources,roundtrip)),'WUHB resource roundtrip')
 need(sha(packed)==PROFILE['target']['root_rpx_sha256']and sha(rebuilt)==PROFILE['target']['WUHB_sha256'],'Exact selected Setup175 outputs')
 need(sha(recipe)==PROFILE['target']['recipe_sha256']and sha(title_recipe)==PROFILE['target']['title_recipe_sha256'],'Exact selected recipe outputs')
 identity=json.loads(read(PARENT/'app-identity.json',pp['app-identity.json']['sha256']))
 identity.update(setup_version=175,game_version=175,setup_revision=REVISION.decode(),practice_revision='v175 Personal Save-State Test',
  previous_practice_sha256=PRIOR_RPX,previous_title_sha256=PRIOR_TITLE,plain_rpx_sha256=trpx,source_cemu_sha256=trpx,native_rpx_sha256=trpx,
  installer_profile='Setup175 checked USB/17 historical-role recognition; Game175 full-kit, AD807 and indexed release readers',
  game_payload_unchanged=False,hardware_tested=False,USB_hardware_tested=False,cleanup_native_execution_observed=False)
 read(Path(__file__))
 for p,r in inputs.items():need(sha(p.read_bytes())==r['sha256'],'Input changed during build')
 out.mkdir()
 for name,data in [('practice.gzd',recipe),('title.gzt',title_recipe),('root.rpx',packed),('GZSetup.wuhb',bytes(rebuilt)),
  ('build.txt',('TWWHD-GZ Setup v175 / Game v175\n'+trpx+'\n').encode()),('app-identity.json',enc(identity))]:save(out/name,data)
 report=dict(schema='reproduced-bounded-Setup175-profile/1',passed=True,passed_structural_checks=True,setup_version=175,game_version=175,
  source_root_rpx_sha256=PARENT_RPX,source_wuhb_sha256=PARENT_WUHB,target_root_rpx_sha256=sha(packed),target_wuhb_sha256=sha(rebuilt),
  target_game_rpx_sha256=trpx,target_game_title_sha256=ttitle,known_prior_rpx_sha256=PRIOR_RPX,known_prior_title_sha256=PRIOR_TITLE,
  inputs=list(inputs.values()),patches=patches,recipe_stats=stats,title_stats=title_stats,exact_recipe_reconstruction=True,
  five_USB_role_hooks_unchanged=True,all_existing_loader_rows_unchanged=True,historical17_role_catalogue_unchanged=True,
  cleanup165_catalogue_and_controlflow_unchanged=True,cleanup_current_identity_arrays_and_log_labels_only_updated=True,
  added_CODE_bytes=0,added_mutable_bytes=0,added_imports=0,added_per_frame_work=False,resources_unchanged=True,
  payloads=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p.read_bytes()))for p in sorted(out.iterdir())],
  packed_CPU_checks_pending=True,independent_review_pending=True,hardware_tested=False,transferred=False,release_ready=False)
 save(out/'build.json',enc(report));print(json.dumps(dict(passed=True,build_sha256=sha((out/'build.json').read_bytes()),root_rpx_sha256=sha(packed),WUHB_sha256=sha(rebuilt),game_rpx_sha256=trpx)))
if __name__=='__main__':main()
