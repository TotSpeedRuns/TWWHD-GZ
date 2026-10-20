"""Read-only Cemu adapter and WWHD EUR v0 route-checkpoint capture."""
import ctypes
from ctypes import wintypes as W
import hashlib, json, math, os, re, struct, time
from pathlib import Path

TITLE='0005000010143600'
SIGNATURE=bytes.fromhex('7c0802a69421fff093e1000c7c7f1b7993c1000890010014408200143860005c')
BANKS={'globalFlags':(0x624,16),'sceneFlags':(0x778,16),'seaFlags':(0x380,9),
       'ffFlags':(0x3c8,9),'drcFlags':(0x3ec,10),'fwFlags':(0x414,8),
       'totgFlags':(0x434,9),'etFlags':(0x458,10),'wtFlags':(0x480,8),
       'gtFlags':(0x4a0,9),'hyruleFlags':(0x4c4,9),'interiorFlags':(0x50c,9)}
INVENTORY=sorted(set(list(range(0x3c,0x51))+[0x0e,0x0f,0xb4,0xb5,0xb8,0xb6,0xbe,0xbf,0xbd]+
                      list(range(0x86,0x8e))+list(range(0x9c,0xa4))))
STATUS={'health':(3,1),'magic':(0x14,1),'rupees':(4,2),'arrows':(0x69,1),
        'bombs':(0x6a,1),'maxhealth':(0,2),'maxmagic':(0x13,1),
        'maxarrows':(0x6f,1),'maxbombs':(0x70,1)}

# Verified persistent fields from libwwhd's d_save.h. Keep the legacy fields
# intact, and add the missing state without copying unknown/runtime records.
_covered=set(INVENTORY)|set(range(9,12))
for _off,_n in STATUS.values():_covered.update(range(_off,_off+_n))
for _off,_n in BANKS.values():_covered.update(range(_off,_off+_n*4))
_persistent={0x10,0x12,0xb7,0x145}
for _start,_end in [(0x51,0x68),(0x76,0x86),(0x90,0x96),(0xa4,0xac),
                    (0xd4,0x135),(0x5c0,0x624),(0x624,0x724)]:
    _persistent.update(range(_start,_end))
for _stage in range(16):
    _persistent.update(range(0x380+_stage*0x24,0x380+_stage*0x24+0x22))
EXTRA_SAVE_OFFSETS=sorted(_persistent-_covered)
# Network captures compare every byte that will actually be serialized. Raw
# audit-only padding, unknown runtime fields and unused temporary-event bytes
# can change while Link is idle; they are not part of a restorable memfile.
SERIALIZED_SAVE_OFFSETS=tuple(sorted(_covered|set(EXTRA_SAVE_OFFSETS)))

def serialized_save_bytes(data):
    return bytes(data[i] for i in SERIALIZED_SAVE_OFFSETS)

def serialized_pose_bytes(data):
    return data[:12]+data[22:24]

class CaptureError(Exception):pass

def pointer(p):
    if not 0x10000000<=p<0xbfff0000 or p%4:raise CaptureError('Link/save memory is unavailable. Load a playable save first.')
    return p

def validate(j):
    if not re.fullmatch(r'[A-Za-z0-9_]{1,7}',j.get('stage','')):raise CaptureError('Invalid stage name.')
    for k,(off,n) in STATUS.items():
        v=j.get(k)
        if type(v) is not int or not 0<=v<(1<<(n*8)):raise CaptureError('Invalid '+k)
    for k,n in [('roomId',256),('layer',256),('spawnId',65536),('angle',65536)]:
        if type(j.get(k)) is not int or not 0<=j[k]<n:raise CaptureError('Invalid '+k)
    for k,(off,n) in BANKS.items():
        if len(j.get(k,[]))!=n or any(type(v) is not int or not 0<=v<=0xffffffff for v in j[k]):raise CaptureError('Invalid '+k)
    expected={str(0x1506b500+i) for i in INVENTORY}|{str(0x10976e6b+i) for i in range(3)}
    actual=set(j.get('inventory',{}))
    # Imported legacy files may also include the Wind Waker acquisition byte.
    if not expected<=actual<=expected|{str(0x1506b553)} or any(type(v) is not int or not 0<=v<=255 for v in j['inventory'].values()):raise CaptureError('Invalid inventory.')
    extra=j.get('extraSaveBytes',{})
    if (not isinstance(extra,dict) or not set(extra)<=set(map(str,EXTRA_SAVE_OFFSETS))
            or any(type(v) is not int or not 0<=v<=255 for v in extra.values())):
        raise CaptureError('Invalid extended save fields.')
    # Native Killer Bees acceptance/catches change the first two mTmp bytes.
    # Keep this separate from persistent save bytes; do not restore unknown
    # restart, actor or remaining session fields.
    session=j.get('sessionEventBytes',{})
    if (not isinstance(session,dict) or not set(session)<={'4440','4441'}
            or any(type(v) is not int or not 0<=v<=255 for v in session.values())):
        raise CaptureError('Invalid supported session event fields.')
    slots=j.get('selectedItemSlots')
    if not isinstance(slots,list) or len(slots)!=3 or any(type(v) is not int or v not in (*range(0x15),0xff) for v in slots):raise CaptureError('Invalid saved item selections.')
    if len(j.get('position',[]))!=3 or any(type(v) not in (int,float) or not math.isfinite(v) or abs(v)>1e7 for v in j['position']):raise CaptureError('Invalid position.')
    if type(j.get('usesPosition')) is not bool or type(j.get('requiresDaytime')) is not bool:raise CaptureError('Invalid capture options.')
    if type(j.get('requiresNighttime',False)) is not bool or (j['requiresDaytime'] and j.get('requiresNighttime',False)):
        raise CaptureError('Choose daytime or nighttime, not both.')
    return j

def capture(memory, uses_position=True, requires_daytime=False):
    """Local reads use one frame; network reads require repeated stable fields."""
    u32=lambda a:int.from_bytes(memory.read(a,4),'big')
    strict_frame=getattr(memory,'require_same_frame',True)
    for attempt in range(getattr(memory,'capture_attempts',12)):
        try:
            frame=u32(0x101ff558)
            if memory.read(0x104741fc,1)!=b'\0':raise CaptureError('The game is loading. Wait until Link is controllable.')
            actor=pointer(u32(0x10474be4));heap=pointer(u32(0x101f84f4));save=heap+0x20
            procedure=u32(actor+0x65f0)
            # EUR v0 dEvt_control_c::mMode (play + 0x3F30 + 0xC2).
            # An idle Link procedure can coexist with an active native demo.
            event_mode=memory.read(0x10474342,1)[0]
            if event_mode:
                raise CaptureError('A conversation or scripted event is active. Wait until Link is controllable, then capture again.')
            # Native demo procedures include the door exit (193), which can
            # replace Link's pose after an otherwise successful checkpoint load.
            if 169<=procedure<222:
                raise CaptureError('A scripted scene or door transition is active. Wait until Link is controllable, then capture again.')
            location=memory.read(0x104741e4,12);room=memory.read(0x1047e6c8,1)[0]
            stage=location[:8].split(b'\0',1)[0].decode('ascii')
            if not re.fullmatch(r'[A-Za-z0-9_]{1,7}',stage):raise CaptureError('No playable stage. Load a save first.')
            if stage in ('sea_T','Name'):raise CaptureError('Title/file-select scene. Load a playable save first.')
            data=memory.read(save,0x800);pose=memory.read(actor+0x314,0x18)
            session=memory.read(save+0x1158,256)
            equip=memory.read(0x10474c6b,3);clock=struct.unpack('>f',memory.read(0x10476a88,4))[0]
            # Reject actor replacements, room changes, loads and frame crossings.
            same=((not strict_frame or frame==u32(0x101ff558)) and actor==u32(0x10474be4) and heap==u32(0x101f84f4)
                  and procedure==u32(actor+0x65f0)
                  and event_mode==memory.read(0x10474342,1)[0]
                  and location==memory.read(0x104741e4,12) and room==memory.read(0x1047e6c8,1)[0]
                  and memory.read(0x104741fc,1)==b'\0'
                  and serialized_save_bytes(data)==serialized_save_bytes(memory.read(save,0x800))
                  and session[:2]==memory.read(save+0x1158,2)
                  and serialized_pose_bytes(pose)==serialized_pose_bytes(memory.read(actor+0x314,0x18))
                  and equip==memory.read(0x10474c6b,3))
            end_frame=u32(0x101ff558)
            same=same and (not strict_frame or frame==end_frame)
            if not same:time.sleep(.002);continue
            j={k:int.from_bytes(data[o:o+n],'big') for k,(o,n) in STATUS.items()}
            j['inventory']={str(0x1506b500+i):data[i] for i in INVENTORY}
            j['inventory'].update({str(0x10976e6b+i):v for i,v in enumerate(equip)})
            j['selectedItemSlots']=list(data[9:12])
            j['extraSaveBytes']={str(i):data[i] for i in EXTRA_SAVE_OFFSETS}
            j['sessionEventBytes']={str(0x1158+i):session[i] for i in range(2)}
            j.update(stage=stage,roomId=room,spawnId=int.from_bytes(location[8:10],'big'),layer=location[11])
            for k,(off,n) in BANKS.items():j[k]=list(struct.unpack('>'+str(n)+'I',data[off:off+4*n]))
            j.update(position=list(struct.unpack('>3f',pose[:12])),angle=int.from_bytes(pose[0x16:0x18],'big'),
                     usesPosition=bool(uses_position),requiresDaytime=bool(requires_daytime))
            validate(j)
            j['_capture']={'tool':'TWWHD-GZ Memfile Creator 0.6','titleId':TITLE,'version':0,
                           'utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'gameFrame':frame,'playerProcedure':procedure,'eventMode':event_mode,
                           'timeOfDay':clock if math.isfinite(clock) else None,'animationSet':data[0x651]&1,
                           'saveSnapshotHex':data.hex(),'saveSnapshotSha256':hashlib.sha256(data).hexdigest(),
                           'extendedSaveBytesCaptured':len(EXTRA_SAVE_OFFSETS),
                           'sessionSnapshotHex':session.hex()}
            j['_capture']['consistency']='single game frame' if strict_frame else 'repeated stable serialized fields across network reads; not frame-atomic'
            j['_capture']['endGameFrame']=end_frame
            j['_capture']['source']=getattr(memory,'source',{'device':'Cemu','pid':getattr(memory,'pid',None)})
            return j
        except (UnicodeError,struct.error,KeyError,ValueError) as e:raise CaptureError('Game memory could not be decoded: '+str(e)) from e
    raise CaptureError('The scene kept changing during capture. Stand still or pause the game, then try again.')

def save_json(path,j):
    validate(j)
    # Exclusive creation prevents silently replacing an existing checkpoint.
    with Path(path).open('x',encoding='utf-8') as f:json.dump(j,f,indent=2,allow_nan=False);f.write('\n')

def encode_checkpoint(j):
    """Encode a validated JSON into the native loader's existing patch format."""
    validate(j)
    patches=[(off,j[k],n) for k,(off,n) in STATUS.items()]
    for key,v in j['inventory'].items():
        a=int(key)
        offset=a-0x1506b500 if 0x1506b500<=a<0x1506b880 else 0x80000000+a-0x10976e6b
        patches.append((offset,v,1))
    patches.extend((9+i,v,1) for i,v in enumerate(j['selectedItemSlots']))
    for k,(off,n) in BANKS.items():patches.extend((off+i*4,v,4) for i,v in enumerate(j[k]))
    patches.extend((int(k),v,1) for k,v in j.get('extraSaveBytes',{}).items())
    patches.extend((int(k),v,1) for k,v in j.get('sessionEventBytes',{}).items())
    if j['requiresDaytime']:patches.extend([(0x24,0x43340000,4),(0x40000000,0x43340000,4)])
    if j.get('requiresNighttime',False):patches.extend([(0x24,0x43ac8000,4),(0x40000000,0x43ac8000,4)])
    return struct.pack('>6I',len(patches),*struct.unpack('>3I',struct.pack('>3f',*j['position'])),j['angle'],int(j['usesPosition']))+b''.join(struct.pack('>3I',*p) for p in patches)

if os.name=='nt':
    K=ctypes.WinDLL('kernel32',use_last_error=True)
    K.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];K.OpenProcess.restype=W.HANDLE
    K.ReadProcessMemory.argtypes=[W.HANDLE,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_size_t,ctypes.POINTER(ctypes.c_size_t)];K.ReadProcessMemory.restype=W.BOOL
    K.QueryFullProcessImageNameW.argtypes=[W.HANDLE,W.DWORD,W.LPWSTR,ctypes.POINTER(W.DWORD)];K.QueryFullProcessImageNameW.restype=W.BOOL
    K.CloseHandle.argtypes=[W.HANDLE];K.CloseHandle.restype=W.BOOL
    K.CreateToolhelp32Snapshot.argtypes=[W.DWORD,W.DWORD];K.CreateToolhelp32Snapshot.restype=W.HANDLE
    class PROCESSENTRY32W(ctypes.Structure):
        _fields_=[('dwSize',W.DWORD),('cntUsage',W.DWORD),('th32ProcessID',W.DWORD),('th32DefaultHeapID',ctypes.c_size_t),('th32ModuleID',W.DWORD),('cntThreads',W.DWORD),('th32ParentProcessID',W.DWORD),('pcPriClassBase',W.LONG),('dwFlags',W.DWORD),('szExeFile',W.WCHAR*260)]
    for name in ['Process32FirstW','Process32NextW']:
        getattr(K,name).argtypes=[W.HANDLE,ctypes.POINTER(PROCESSENTRY32W)];getattr(K,name).restype=W.BOOL

def processes():
    if os.name!='nt':return []
    result=[];snapshot=K.CreateToolhelp32Snapshot(2,0)
    if snapshot==ctypes.c_void_p(-1).value:raise CaptureError('Cannot list processes.')
    try:
        entry=PROCESSENTRY32W();entry.dwSize=ctypes.sizeof(entry)
        more=K.Process32FirstW(snapshot,ctypes.byref(entry))
        while more:
            if entry.szExeFile.lower()=='cemu.exe':
                handle=K.OpenProcess(0x1000,False,entry.th32ProcessID)
                if handle:
                    try:
                        image=ctypes.create_unicode_buffer(32768);n=W.DWORD(len(image))
                        if K.QueryFullProcessImageNameW(handle,0,image,ctypes.byref(n)):result.append((entry.th32ProcessID,image.value))
                    finally:K.CloseHandle(handle)
            more=K.Process32NextW(snapshot,ctypes.byref(entry))
    finally:K.CloseHandle(snapshot)
    return sorted(result,reverse=True)

class CemuMemory:
    def __init__(self,pid,log=None):
        self.handle=None;self.pid=pid;self.base=0
        if os.name!='nt' or struct.calcsize('P')!=8:raise CaptureError('Use 64-bit Windows.')
        # Deliberately omit PROCESS_VM_WRITE and PROCESS_VM_OPERATION.
        self.handle=K.OpenProcess(0x1010,False,pid)
        if not self.handle:raise CaptureError('Cannot read Cemu. Run both apps as the same Windows user/privilege level.')
        try:
            image=ctypes.create_unicode_buffer(32768);n=W.DWORD(len(image))
            if not K.QueryFullProcessImageNameW(self.handle,0,image,ctypes.byref(n)) or Path(image.value).name.lower()!='cemu.exe':raise CaptureError('Selected process is not Cemu.exe.')
            self.image=image.value;directory=Path(self.image).parent
            logs=[Path(log)] if log else [directory/'portable/log.txt',directory/'log.txt',Path(os.environ.get('APPDATA',''))/'Cemu/log.txt']
            reasons=[]
            for candidate in logs:
                if not candidate.is_file():continue
                try:
                    text=candidate.read_text(encoding='utf-8',errors='replace')
                    bases=re.findall(r'Init Wii U memory space \(base: (0x[0-9a-fA-F]+)\)',text)
                    titles=re.findall(r'TitleId: ([0-9a-fA-F]{8})-?([0-9a-fA-F]{8})',text)
                    versions=re.findall(r'TitleVersion: v(\d+)',text)
                    if not titles or ''.join(titles[-1]).lower()!=TITLE or not versions or versions[-1]!='0':raise CaptureError('Log is not WWHD EUR v0.')
                    if not bases:raise CaptureError('Log has no guest-memory base.')
                    self.base=int(bases[-1],16)
                    if self.read(0x02000020,len(SIGNATURE))!=SIGNATURE:raise CaptureError('Live executable signature does not match this profile/log.')
                    self.log=str(candidate);return
                except (OSError,CaptureError) as e:reasons.append(str(candidate)+': '+str(e))
            raise CaptureError('No matching live Cemu log. Use Browse log to select this instance\'s log.txt. '+('; '.join(reasons)))
        except Exception:self.close();raise

    def read(self,address,size):
        if not self.handle:raise CaptureError('Disconnected.')
        if not 0<=address<=0xffffffff or size<0 or address+size>0x100000000:raise CaptureError('Invalid guest read.')
        buffer=ctypes.create_string_buffer(size);done=ctypes.c_size_t()
        if not K.ReadProcessMemory(self.handle,self.base+address,buffer,size,ctypes.byref(done)) or done.value!=size:raise CaptureError('Cemu memory is unavailable or the process closed. Reconnect.')
        return buffer.raw

    def close(self):
        if self.handle:K.CloseHandle(self.handle);self.handle=None
