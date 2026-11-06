"""Verbatim Raw class only; legacy enclosing runtime review is excluded."""
import struct,zlib

class Raw:
 def __init__(self,p):
  raw=p.read_bytes();assert raw[:7]==b'\x7fELF\x01\x02\x01'and struct.unpack_from('>H',raw,18)[0]==20;off=struct.unpack_from('>I',raw,32)[0];stride,n,ni=struct.unpack_from('>HHH',raw,46);assert stride==40 and 0<n<=1024 and off+40*n<=len(raw);self.h=[];self.d=[]
  for i in range(n):
   h=struct.unpack_from('>10I',raw,off+i*40);self.h.append(h)
   if h[1]in(0,8):data=b''
   else:
    assert h[4]+h[5]<=len(raw);data=raw[h[4]:h[4]+h[5]]
    if h[2]&0x08000000:
     z=struct.unpack_from('>I',data)[0];assert z<=64*1024*1024;q=zlib.decompressobj();decoded=q.decompress(data[4:],z+1);assert len(decoded)==z and q.eof and not q.unused_data;data=decoded
   self.d.append(data)
  self.names=[self.d[ni][h[0]:].split(b'\0',1)[0].decode('ascii')for h in self.h];self.sy=[];lookup={};self.rows=[]
  for i,h in enumerate(self.h):
   if h[1]!=2:continue
   assert h[9]==16 and len(self.d[i])%16==0;strings=self.d[h[6]]
   for j,(no,v,z,info,other,sec)in enumerate(struct.iter_unpack('>IIIBBH',self.d[i])):
    s=dict(name=strings[no:].split(b'\0',1)[0].decode('ascii'),value=v,size=z,info=info,other=other,section=sec,table=i,index=j);self.sy.append(s);lookup[i,j]=s
  for i,h in enumerate(self.h):
   if h[1]==4:
    assert h[9]==12 and len(self.d[i])%12==0
    for site,info,a in struct.iter_unpack('>IIi',self.d[i]):self.rows.append(dict(source=h[7],site=site,kind=info&255,addend=a,symbol=lookup[h[6],info>>8]))
 def named(self,name):
  r=[s for s in self.sy if s['name']==name];assert len(r)==1,(name,len(r));return r[0]
 def body(self,s):return self.d[s['section']][s['value']:s['value']+s['size']]
 def span(self,p,z):
  r=[self.d[i][p-h[3]:p-h[3]+z]for i,h in enumerate(self.h)if h[1]!=8 and h[3]and h[3]<=p and p+z<=h[3]+len(self.d[i])];assert len(r)==1;return r[0]
 def refs(self,s):return[r for r in self.rows if r['source']==s['section']and s['value']<=r['site']<s['value']+s['size']]
 def sig(self,s):return(s['name'],self.names[s['section']]if 0<s['section']<len(self.h)else s['section'],s['value'],s['size'],s['info'],s['other'])
 def expr(self,r):
  s=r['symbol'];return(self.names[r['source']],r['site'],r['kind'],s['name'],r['addend'],*self.sig(s)[1:])
