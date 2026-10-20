"""Read-only TCPGecko transport. Aroma peekmultiple and legacy command 0x04 only."""
import ipaddress,socket,struct,threading
from memfile_core import CaptureError,SIGNATURE

class WiiUMemory:
    require_same_frame=False
    capture_attempts=3
    pid=None
    def __init__(self,ip,port=None,timeout=3,profile_address=None):
        self.ip=str(ipaddress.IPv4Address(ip.strip()));self.port=int(port) if port is not None else 7332
        if not 1<=self.port<=65535:raise CaptureError('Invalid Wii U port.')
        self.sock=None;self.lock=threading.Lock();self.log=None;self.text_delta=0;self.data_delta=0
        self.profile_address=int(str(profile_address),0) if profile_address else None;self.profile_map=[]
        self.source={'device':'Wii U','ip':self.ip,'port':self.port,'transport':'TCPGecko read-only',
                     'memoryProfile':'WWHD EUR base v0','hardwareReloadVerified':False}
        try:
            if port is None:
                try:self.sock=socket.create_connection((self.ip,7332),timeout=timeout)
                except OSError:self.port=7331;self.sock=socket.create_connection((self.ip,7331),timeout=timeout)
            else:self.sock=socket.create_connection((self.ip,self.port),timeout=timeout)
            self.aroma=self.port==7332
            self.source.update(port=self.port,transport='TCPGecko Aroma read-only peekmultiple' if self.aroma else 'Legacy TCPGecko read-only 0x04')
            self.sock.settimeout(timeout)
            if self.profile_address is not None:
                self._detect_gz_beacon()
            elif self.aroma:
                self._detect_aroma_profile()
            else:
                matched=False
                for base in (0x02000000,0x0e000000):
                    if self._read_raw(base+0x20,len(SIGNATURE))==SIGNATURE:
                        self.text_delta=base-0x02000000;matched=True;break
                if not matched:
                    raise CaptureError('Live executable does not match WWHD EUR base v0. Open the supported game before connecting.')
                # Two retained original lis/lwz pairs locate the moved game DATA
                # segment. Validate opcodes before trusting either immediate.
                def data_reference(high,low,high_opcode,low_opcode):
                    h=int.from_bytes(self._read_raw(high+self.text_delta,4),'big')
                    l=int.from_bytes(self._read_raw(low+self.text_delta,4),'big')
                    if h&0xffff0000!=high_opcode or l&0xffff0000!=low_opcode:
                        raise CaptureError('Running game does not match the supported DATA-address probes.')
                    immediate=l&0xffff;immediate=immediate if immediate<0x8000 else immediate-0x10000
                    return (((h&0xffff)<<16)+immediate)&0xffffffff
                first=data_reference(0x0202437c,0x02024380,0x3d800000,0x818c0000)
                second=data_reference(0x02026a9c,0x02026ab0,0x3f200000,0x80f90000)
                if first!=second or not 0x10000000<=first<0x30000000:
                    raise CaptureError('Wii U DATA-address probes disagree; refusing capture.')
                self.data_delta=first-0x101f84f4
                self.source.update(textDelta=self.text_delta,dataDelta=self.data_delta)
        except (OSError,CaptureError) as e:
            self.close()
            raise CaptureError('Wii U connection failed: '+str(e)+'. TCPGecko must be running alongside WWHD-GZ (Aroma 7332 or legacy 7331). No PC hacker is required; FTPiiU alone cannot read live memory.') from e
    def _detect_aroma_profile(self):
        if self.profile_address is not None:
            self._detect_gz_beacon();return
        self._detect_legacy_aroma_profile()

    def _detect_legacy_aroma_profile(self):
        # Aroma's peek implementation directly dereferences the address.
        # Do NOT use legacy low TEXT probes (0x02000000 / 0x0e000000).
        # Limit discovery to the two documented WWHD DATA layouts, reject
        # invalid pointers before following them, and verify native Link
        # identity plus independently located stage/save data.
        import re,math
        matches=[]
        for delta in (0,0x27f20):
            stage=self._read_raw(0x104741e4+delta,8).split(b'\0')[0]
            if not re.fullmatch(rb'[A-Za-z0-9_]{1,7}',stage):continue
            heap=int.from_bytes(self._read_raw(0x101f84f4+delta,4),'big')
            actor=int.from_bytes(self._read_raw(0x10474be4+delta,4),'big')
            if not (0x10000000<=heap<0xbfff0000 and heap%4==0):continue
            if not (0x10000000<=actor<0xbfff0000 and actor%4==0):continue
            if int.from_bytes(self._read_raw(actor+8,2),'big')!=168:continue
            status=self._read_raw(heap+0x20,5);maximum=int.from_bytes(status[:2],'big')
            if not 1<=maximum<=80 or status[3]>maximum:continue
            pose=struct.unpack('>3f',self._read_raw(actor+0x314,12))
            if not all(math.isfinite(v) and abs(v)<10000000 for v in pose):continue
            matches.append((delta,stage.decode()))
        if len(matches)!=1:
            raise CaptureError('Aroma game DATA profile is not uniquely validated. Load a playable WWHD-GZ EUR v0 save first; no code-address probes were sent.')
        self.data_delta=matches[0][0]
        self.source.update(dataDelta=self.data_delta,profileValidation='mapped stage + save format + native Link process168 + finite pose',lowCodeAddressProbes=False)

    def _detect_gz_beacon(self):
        import re,math
        address=self.profile_address
        if not 0x10000000<=address<0x50000000-64 or address%4:
            raise CaptureError('GZ profile must be an aligned game DATA address from the GZ v75 menu.')
        raw=self._read_raw(address,64)
        if raw[:20]!=b'GZCAPTUREBEACON1\0'+bytes(3):
            raise CaptureError('GZ profile signature missing. Copy the number from the v75 main menu.')
        version,region,*fields=struct.unpack('>11I',raw[20:])
        if version!=1 or region!=0x45555230 or fields[-1]!=0x43504d31:
            raise CaptureError('Unsupported GZ capture profile schema or region.')
        addresses=fields[:-1]
        if any(not 0x10000000<=a<0x50000000-32 for a in addresses):
            raise CaptureError('GZ profile references are outside game DATA.')
        stage,actor_slot,heap_slot,clock,equip,event,frame,room=addresses
        if any(a%4 for a in (stage,actor_slot,heap_slot,clock,frame)):
            raise CaptureError('Unaligned GZ profile references.')
        stage_name=self._read_raw(stage,8).split(b'\0')[0]
        if not re.fullmatch(rb'[A-Za-z0-9_]{1,7}',stage_name) or stage_name in (b'Name',b'sea_T'):
            raise CaptureError('Load a playable save before connecting.')
        actor=int.from_bytes(self._read_raw(actor_slot,4),'big');heap=int.from_bytes(self._read_raw(heap_slot,4),'big')
        if any(not 0x10000000<=p<0x50000000-0x8000 or p%4 for p in (actor,heap)):
            raise CaptureError('GZ save/Link pointers are unavailable.')
        if int.from_bytes(self._read_raw(actor+8,2),'big')!=168:
            raise CaptureError('GZ profile does not reference native Link.')
        status=self._read_raw(heap+0x20,5);maximum=int.from_bytes(status[:2],'big')
        pose=struct.unpack('>3f',self._read_raw(actor+0x314,12))
        if not 1<=maximum<=80 or status[3]>maximum or not all(math.isfinite(v) and abs(v)<10000000 for v in pose):
            raise CaptureError('GZ profile save or pose validation failed.')
        originals=[0x104741e4,0x10474be4,0x101f84f4,0x10476a88,0x10474c6b,0x10474342,0x101ff558,0x1047e6c8]
        sizes=[32,4,4,4,3,1,4,1]
        self.profile_map=list(zip(originals,addresses,sizes))
        self.source.update(memoryProfile='GZ v75 relocated runtime profile',profileAddress=address,profileValidation='versioned EUR0 beacon + mapped stage + native Link 168 + save and finite pose',lowCodeAddressProbes=False,profileReferences=dict(zip(map(hex,originals),addresses)))

    def _receive(self,size):
        out=bytearray()
        while len(out)<size:
            chunk=self.sock.recv(size-len(out))
            if not chunk:raise CaptureError('Wii U disconnected during memory read. Reconnect.')
            out.extend(chunk)
        return bytes(out)
    def read(self,address,size):
        # Static game globals use the known profile. Actor/save pointers read
        # from the game already contain live addresses and remain unchanged.
        if self.profile_map:
            for original,actual,length in self.profile_map:
                if original<=address and address+size<=original+length:
                    address=actual+address-original;break
            return self._read_raw(address,size)
        if 0x02000000<=address<0x03000000:address+=self.text_delta
        elif 0x10000000<=address<0x104da2c8:address+=self.data_delta
        return self._read_raw(address,size)
    def _read_raw(self,address,size):
        if type(address) is not int or type(size) is not int or not 0<size<=65536:
            raise CaptureError('Invalid Wii U memory read.')
        if not (0x02000000<=address and address+size<=0x10000000 or
                0x10000000<=address and address+size<=0xc0000000):
            raise CaptureError('Wii U read is outside the supported game memory ranges.')
        with self.lock:
            if self.sock is None:raise CaptureError('Disconnected.')
            result=bytearray()
            try:
                if self.aroma:return self._read_aroma(address,size)
                for offset in range(0,size,0x400):
                    count=min(0x400,size-offset);start=address+offset
                    self.sock.sendall(b'\x04'+struct.pack('>II',start,start+count))
                    response=self._receive(1)
                    if response==b'\xbd':result.extend(self._receive(count))
                    elif response==b'\xb0':result.extend(bytes(count))
                    else:raise CaptureError('Unexpected TCPGecko memory response; reconnect.')
            except (OSError,CaptureError) as e:
                self.close();raise CaptureError('Wii U memory read failed: '+str(e)) from e
            return bytes(result)
    def _read_aroma(self,address,size):
        # Server replies to peekmultiple with unsigned decimal words, each
        # terminated by '|'. This permits exact receive framing even when
        # TCP splits/coalesces packets; never guess completion from one recv.
        begin=address&~3;end=(address+size+3)&~3;data=bytearray()
        for start in range(begin,end,32*4):
            addresses=list(range(start,min(start+32*4,end),4))
            command=('peekmultiple -t u32'+''.join(' -a 0x%08X'%a for a in addresses)).encode('ascii')
            assert len(command)<1024
            self.sock.sendall(command);reply=bytearray()
            while reply.count(b'|')<len(addresses):
                piece=self.sock.recv(512)
                if not piece:raise CaptureError('Wii U disconnected during Aroma memory read.')
                reply.extend(piece)
                if len(reply)>12*len(addresses)+100:raise CaptureError('Oversized Aroma response.')
                if b'Invalid' in reply or b'missing' in reply:raise CaptureError('Aroma rejected read-only request: '+reply.decode(errors='replace'))
            parts=bytes(reply).split(b'|')
            if len(parts)!=len(addresses)+1 or parts[-1]:raise CaptureError('Unexpected Aroma response framing; reconnect.')
            for part in parts[:-1]:
                if not part.isdigit():raise CaptureError('Unexpected Aroma word value; reconnect.')
                value=int(part)
                if not 0<=value<=0xffffffff:raise CaptureError('Aroma word is out of range.')
                data.extend(struct.pack('>I',value))
        return bytes(data[address-begin:address-begin+size])

    def close(self):
        sock=self.sock;self.sock=None
        if sock:
            try:sock.shutdown(socket.SHUT_RDWR)
            except OSError:pass
            sock.close()
