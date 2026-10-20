"""Create and verify a separate native game copy; never open source for writing."""
import os,hashlib,json,shutil,stat,uuid,struct,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image
from patch_engine import patch_rpx
from title_engine import patch_title

TEST_TITLE='0005000010575a00'

def resolve_game_folder(selected):
 """Accept root/code/file or one unambiguous wrapper; never alter the dump."""
 selected=Path(selected).expanduser().resolve(strict=True)
 if selected.is_file():
  if selected.name.lower()=='cking.rpx':selected=selected.parent.parent
  else:raise ValueError('Select an extracted WWHD folder containing code, content and meta. WUD/WUX/WUA and encrypted install files must be extracted first.')
 if selected.name.lower()=='code' and (selected/'cking.rpx').is_file():selected=selected.parent
 if (selected/'code/cking.rpx').is_file():return selected
 candidates=[p for p in selected.iterdir() if p.is_dir() and not p.is_symlink() and (p/'code/cking.rpx').is_file()]
 if len(candidates)==1:return candidates[0]
 if len(candidates)>1:raise ValueError('This folder contains multiple games. Select the WWHD folder containing code, content and meta.')
 if (selected/'title.tmd').is_file() or any(p.is_file() and p.suffix.lower() in ('.app','.h3') for p in selected.iterdir()):
  raise ValueError('This is an encrypted Wii U install package. In WiiUDownloader choose Tools > Decrypt Contents, then select the resulting folder containing code, content and meta. This release supports the EUR base game v0, not an update.')
 raise ValueError('Cannot find code/cking.rpx. Select the parent folder containing code, content and meta, not the code folder or the patcher folder. WUD/WUX/WUA and encrypted install-files dumps need extraction first; this release supports EUR v0.')
def sha(data):return hashlib.sha256(data).hexdigest()
def file_hash(path):
 h=hashlib.sha256()
 with Path(path).open('rb') as f:
  for chunk in iter(lambda:f.read(4*1024*1024),b''):h.update(chunk)
 return h.hexdigest()
def metadata(data):
 tree=ET.fromstring(data)
 for node in tree.iter():
  if node.tag=='title_id':node.text=TEST_TITLE
  elif node.tag=='group_id':node.text='0000575a'
  elif node.tag.startswith('longname_'):node.text='TWWHD-GZ'
  elif node.tag.startswith('shortname_'):node.text='TWWHD-GZ'
  elif node.tag.startswith('publisher_'):node.text='TWWHD-GZ Practice Tools'
 return ET.tostring(tree,encoding='utf-8',xml_declaration=True)
def make_icon(original,image_path):
 if len(original)<18:raise ValueError('Invalid original menu icon.')
 width,height=struct.unpack_from('<HH',original,12);depth=original[16]
 if original[2]!=2 or original[1] or (width,height)!=(128,128) or depth!=32:raise ValueError('Unsupported native icon format.')
 image=Image.open(image_path).convert('RGBA').resize((width,height),Image.Resampling.LANCZOS)
 if not original[17]&0x20:image=image.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
 start=18+original[0];end=start+width*height*4
 if len(original)<end:raise ValueError('Truncated original icon.')
 return original[:start]+image.tobytes('raw','BGRA')+original[end:]

def create_copy(source,destination,resources,progress=lambda message,fraction:None,cancel=lambda:False):
 source=resolve_game_folder(source);destination=Path(destination).absolute();resources=Path(resources)
 parent=destination.parent.resolve(strict=True);destination=parent/destination.name
 if not source.is_dir():raise ValueError('Select the extracted game folder containing code, content and meta.')
 if source==destination or destination.is_relative_to(source) or source.is_relative_to(destination):raise ValueError('Output must be separate from the original folder.')
 if destination.exists() or destination.is_symlink():raise ValueError('Choose a new output folder; existing folders are never replaced.')
 for required in ['code/cking.rpx','code/app.xml','code/cos.xml','meta/meta.xml','meta/iconTex.tga','content/Common/Layout/Title_00.szs']:
  if not (source/required).is_file():raise ValueError('Missing game file: '+required)
 if not (source/'content').is_dir() or not any((source/'content').iterdir()):raise ValueError('The full content folder is required.')
 manifest=json.loads((resources/'payload/manifest.json').read_text())
 app=ET.fromstring((source/'code/app.xml').read_bytes())
 if app.findtext('title_id','').lower()!=manifest['title_id'] or app.findtext('title_version')!='0000':raise ValueError('This native release supports WWHD EUR v0 only.')
 progress('Checking the original executable',0)
 raw=(source/'code/cking.rpx').read_bytes();patched,rpx_report=patch_rpx(raw,resources/'payload')
 if sha(raw)!=file_hash(source/'code/cking.rpx'):raise ValueError('The source executable changed during validation.')
 files=[]
 for directory,dirs,names in os.walk(source,followlinks=False):
  for name in dirs+names:
   path=Path(directory)/name
   if path.is_symlink() or not path.resolve().is_relative_to(source):raise ValueError('Links outside the game folder are unsupported.')
  for name in names:
   path=Path(directory)/name;s=path.stat()
   if not stat.S_ISREG(s.st_mode):raise ValueError('Unsupported source file type.')
   files.append((path,path.relative_to(source),s.st_size))
 total=sum(size for _,_,size in files)
 if shutil.disk_usage(parent).free<total+256*1024*1024:raise ValueError('Not enough free space for a verified separate game copy.')
 stage=parent/('.wwhdgz-'+uuid.uuid4().hex);stage.mkdir()
 source_hashes={};output_hashes={};copied=0
 try:
  for src,relative,size in files:
   if cancel():raise InterruptedError('Cancelled. The original was not changed.')
   dst=stage/relative;dst.parent.mkdir(parents=True,exist_ok=True);before=src.stat();h=hashlib.sha256()
   with src.open('rb') as reader,dst.open('xb') as writer:
    for chunk in iter(lambda:reader.read(4*1024*1024),b''):
     writer.write(chunk);h.update(chunk)
   after=src.stat()
   if (before.st_size,before.st_mtime_ns)!=(after.st_size,after.st_mtime_ns):raise ValueError('A source file changed while copying: '+str(relative))
   digest=h.hexdigest()
   if file_hash(dst)!=digest:raise ValueError('Copied file failed read-back verification: '+str(relative))
   source_hashes[relative.as_posix()]=digest;output_hashes[relative.as_posix()]=digest;copied+=size
   progress('Copying and verifying '+str(relative),.05+.85*copied/max(total,1))
  changes={'code/cking.rpx':patched,
   'code/app.xml':metadata((stage/'code/app.xml').read_bytes()),
   'meta/meta.xml':metadata((stage/'meta/meta.xml').read_bytes()),
   'meta/iconTex.tga':make_icon((stage/'meta/iconTex.tga').read_bytes(),resources/'home-menu-icon-original.jpg')}
  changes['content/Common/Layout/Title_00.szs']=patch_title((stage/'content/Common/Layout/Title_00.szs').read_bytes(),resources)
  for name,data in changes.items():
   (stage/name).write_bytes(data)
   if file_hash(stage/name)!=sha(data):raise ValueError('Modified file failed verification: '+name)
   output_hashes[name]=sha(data)
  if file_hash(source/'code/cking.rpx')!=manifest['source_sha256']:raise ValueError('The original executable changed during the build.')
  for name in changes:
   if file_hash(source/name)!=source_hashes[name]:raise ValueError('An original patch-target file changed during the build.')
  report=dict(format='wwhdgz-native-build',version='0.1.0',source=str(source),output=str(destination),title_id=TEST_TITLE,
   original_untouched=True,original_patch_targets_reverified=True,files_copied=len(files),bytes_copied=total,
   modified_files=list(changes),source_file_sha256=source_hashes,output_file_sha256=output_hashes,rpx=rpx_report,
   native_hardware_tested=False,launch_method='Separate homebrew-enabled Wii U title after WUP packaging, or compatible decrypted-folder loader.')
  (stage/'WWHDGZ-build-report.json').write_text(json.dumps(report,indent=2))
  progress('Publishing the verified modified game',.98)
  if destination.exists():raise ValueError('Output folder appeared during the build; refusing to replace it.')
  stage.rename(destination);progress('Created separate native test copy',1)
  return report
 except BaseException:
  # Only remove the uniquely created staging directory under the verified parent.
  if stage.exists() and stage.parent.resolve()==parent and stage.name.startswith('.wwhdgz-') and stage!=source and not source.is_relative_to(stage):shutil.rmtree(stage)
  raise
