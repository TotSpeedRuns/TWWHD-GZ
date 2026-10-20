"""Append complete named capture data; never promote a checkpoint as verified."""
import json,hashlib,os,re,datetime
from pathlib import Path

def checkpoint_name(value):
 name=value.strip()
 if not name or len(name)>100:raise ValueError('Enter a checkpoint name (1 to 100 characters).')
 if re.search(r'[<>:"/\\|?*\x00-\x1f]',name) or name.endswith(('.', ' ')):raise ValueError('Use a filename-safe checkpoint name.')
 if name.split('.')[0].upper() in {'CON','PRN','AUX','NUL',*[f'COM{i}' for i in range(1,10)],*[f'LPT{i}' for i in range(1,10)]}:raise ValueError('That name is reserved by Windows.')
 return name

def named_snapshot(snapshot,name):
 result=dict(snapshot);result['_capture']=dict(snapshot.get('_capture',{}));result['_capture']['checkpointName']=checkpoint_name(name)
 return result

def append_capture(log_path,snapshot,saved_path,pid=None):
 path=Path(saved_path).resolve();record=dict(format='wwhdgz-named-capture-log-v1',utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),name=snapshot['_capture']['checkpointName'],memfile=str(path),memfileSha256=hashlib.sha256(path.read_bytes()).hexdigest(),cemuPid=pid,reloadVerified=False,installed=False,snapshot=snapshot)
 line=json.dumps(record,ensure_ascii=False,allow_nan=False,separators=(',',':'))+'\n'
 log_path=Path(log_path);log_path.parent.mkdir(parents=True,exist_ok=True)
 with log_path.open('a+',encoding='utf-8',newline='\n') as f:
  if os.name=='nt':
   import msvcrt
   f.seek(0);msvcrt.locking(f.fileno(),msvcrt.LK_LOCK,1)
  try:f.seek(0,2);f.write(line);f.flush();os.fsync(f.fileno())
  finally:
   if os.name=='nt':f.seek(0);msvcrt.locking(f.fileno(),msvcrt.LK_UNLCK,1)
 return record
