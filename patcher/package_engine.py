import os,re,json,subprocess,tempfile,shutil,uuid,hashlib,struct
from pathlib import Path
from folder_engine import TEST_TITLE,file_hash

def read_common_key(path):
 data=Path(path).read_bytes()
 if len(data)==16:return data.hex()
 try:text=data.decode('ascii').strip()
 except UnicodeError:raise ValueError('The common-key file must contain 16 binary bytes or 32 hexadecimal characters.')
 if not re.fullmatch('[0-9a-fA-F]{32}',text):raise ValueError('The common-key file must contain 16 binary bytes or 32 hexadecimal characters.')
 return text.lower()

def package_install(game,destination,resources,key_file,progress=lambda message,fraction:None):
 game=Path(game).resolve(strict=True);destination=Path(destination).absolute();parent=destination.parent.resolve(strict=True)
 destination=parent/destination.name;resources=Path(resources).resolve(strict=True)
 if destination.exists() or destination.is_symlink() or destination.is_relative_to(game) or game.is_relative_to(destination):raise ValueError('Choose a new install-output folder outside the game copy.')
 key=read_common_key(key_file)
 java=resources/'tools/jre/bin/java.exe';jar=resources/'tools/NUSPacker.jar'
 if not java.is_file() or not jar.is_file():raise ValueError('The bundled package tools are missing. Keep the tools folder beside the patcher.')
 manifest=json.loads((resources/'tools/tools-manifest.json').read_text())
 if file_hash(jar)!=manifest['nuspacker_sha256']:raise ValueError('Package tool verification failed.')
 total=sum(p.stat().st_size for p in game.rglob('*') if p.is_file())
 if shutil.disk_usage(parent).free<2*total+256*1024*1024:raise ValueError('Not enough space for the Wii U install package.')
 stage=parent/('.wwhdgz-install-'+uuid.uuid4().hex);stage.mkdir();work=stage/'packing';work.mkdir();output=stage/'install'
 key_path=work/'encryptKeyWith';key_path.write_text(key)
 try:
  package_input=stage/'game';package_input.mkdir()
  for folder in ('code','content','meta'):
   shutil.copytree(game/folder,package_input/folder)
  progress('Building the separate Wii U install package',0)
  result=subprocess.run([str(java),'-Xmx768m','-jar',str(jar),'-in',str(package_input),'-out',str(output)],cwd=work,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=3600,creationflags=subprocess.CREATE_NO_WINDOW if os.name=='nt' else 0)
  text=result.stdout.decode('utf-8',errors='replace').replace(key,'[common key omitted]').replace(key.upper(),'[common key omitted]')
  text=re.sub(r'(?i)\b[0-9a-f]{32}\b','[key/hash omitted]',text)
  if result.returncode:raise ValueError('Install packing failed: '+text[-1500:])
  for name in ['title.tmd','title.tik','title.cert']:
   if not (output/name).is_file():raise ValueError('Package builder did not produce '+name+': '+text[-2000:])
  tmd=(output/'title.tmd').read_bytes();ticket=(output/'title.tik').read_bytes()
  expected=int(TEST_TITLE,16)
  if len(tmd)<0x194 or int.from_bytes(tmd[0x18c:0x194],'big')!=expected:raise ValueError('Install package has the wrong title ID.')
  if len(ticket)<0x1e4 or int.from_bytes(ticket[0x1dc:0x1e4],'big')!=expected:raise ValueError('Install ticket has the wrong title ID.')
  if not list(output.glob('*.app')):raise ValueError('Install package has no game content.')
  files={p.name:file_hash(p) for p in output.iterdir() if p.is_file()}
  report=dict(passed=True,title_id=TEST_TITLE,files=files,native_hardware_tested=False,common_key_in_report=False)
  (output/'WWHDGZ-install-report.json').write_text(json.dumps(report,indent=2))
  output.rename(destination);progress('Created Wii U install package',1)
  return report
 finally:
  if key_path.exists():key_path.unlink()
  if stage.exists() and stage.parent.resolve()==parent and stage.name.startswith('.wwhdgz-install-') and stage!=game:shutil.rmtree(stage)
