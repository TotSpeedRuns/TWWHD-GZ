"""Generate private capacity text from exact owned EUR v0 language packs.

The exported source contains an authored transform and chooser template only.
Complete original/changed messages are generated solely in a new private output
directory, never in the repository. No runtime or game image is generated.
"""
from pathlib import Path
import argparse,hashlib,importlib.util,json,os,struct,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1];REL='experiments/maintained-components/rando-mode-v173';COMP=ROOT/REL
SPEC_SHA='52b891117e143a6352c981dba58872de5db562f1f617d1673d9a7d03fee0a5a2'
def need(v,m):
 if not v:raise ValueError(m)
def sha(b):return hashlib.sha256(b).hexdigest()
def put(p,b):
 with p.open('xb')as f:f.write(b);f.flush();os.fsync(f.fileno())
 need(p.read_bytes()==b,'saved private output readback')
def messages(b):
 need(b[:8]==b'MsgStdBn'and b[8:10]==b'\xfe\xff'and len(b)>=32,'MSBT header')
 need(struct.unpack_from('>I',b,18)[0]==len(b),'MSBT complete extent');n=struct.unpack_from('>H',b,14)[0];at=32;sections={}
 for unused in range(n):
  need(at+16<=len(b),'MSBT section header');name=b[at:at+4];size=struct.unpack_from('>I',b,at+4)[0];need(at+16+size<=len(b)and name not in sections,'MSBT section extent/uniqueness');sections[name]=b[at+16:at+16+size];at=(at+16+size+15)&~15
 need(b'LBL1'in sections and b'TXT2'in sections,'MSBT labels/text');labels=sections[b'LBL1'];text=sections[b'TXT2'];need(len(labels)>=4 and len(text)>=4,'MSBT tables')
 n=struct.unpack_from('>I',labels)[0];need(4+8*n<=len(labels),'label bucket table');selected={}
 for j in range(n):
  count,pos=struct.unpack_from('>II',labels,4+j*8)
  for unused in range(count):
   need(pos<len(labels),'label start');z=labels[pos];need(pos+1+z+4<=len(labels),'label extent');label=labels[pos+1:pos+1+z].decode('ascii');index=struct.unpack_from('>I',labels,pos+1+z)[0];pos+=1+z+4
   if label in('00140','00150'):need(label not in selected,'unique selected label');selected[label]=index
 total=struct.unpack_from('>I',text)[0];need(4+4*total<=len(text),'text offset table');offsets=list(struct.unpack_from('>'+str(total)+'I',text,4))+[len(text)];result={}
 for label,index in selected.items():
  need(index<total and 4+4*total<=offsets[index]<=offsets[index+1]<=len(text),'selected message bounds');raw=text[offsets[index]:offsets[index+1]];need(len(raw)%2==0 and raw[-2:]==b'\0\0','selected UTF16 terminator');result[int(label)]=list(struct.unpack('>'+str(len(raw)//2)+'H',raw))
 need(set(result)=={140,150},'exact selected message labels');return result
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--game-root',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--reference-repository',type=Path,default=ROOT)
 a=ap.parse_args();owned=a.game_root.resolve(strict=True);out=a.output.resolve();reference=a.reference_repository.resolve(strict=True)
 need(not out.exists()and out.parent.is_dir()and not out.is_relative_to(ROOT)and not out.is_relative_to(reference)and not out.is_relative_to(owned),'fresh private output outside game/source roots with existing parent')
 sr=(COMP/'build-spec.json').read_bytes();need(sha(sr)==SPEC_SHA,'exact173 authored specification');spec=json.loads(sr);cap=spec['capacity'];template=(ROOT/cap['generator_template']).read_bytes();need(sha(template)==cap['template_sha256'],'exact authored chooser template')
 parser_row=next(x for x in spec['references']if x['path']=='patcher/title_engine.py');parser=reference/parser_row['path'];need(sha(parser.read_bytes())==parser_row['sha256'],'pinned public Yaz0/SARC parser')
 s=importlib.util.spec_from_file_location('capacity173_public_title_parser',parser);T=importlib.util.module_from_spec(s);sys.modules[s.name]=T;s.loader.exec_module(T)
 texts=[];removed=[];recipes=[];input_paths=[];message_hashes=[]
 def units(value):return list(struct.unpack('>'+str(len(value.encode('utf-16-be'))//2)+'H',value.encode('utf-16-be')))
 default=[14,0,3,2,65535,14,0,3,2,0]
 for row in cap['owned_pack_pins']:
  name=row['filename'];need(Path(name).name==name and name.startswith('permanent_2d_Eu')and name.endswith('.pack'),'fixed owned language filename')
  p=(owned/'content/Common/Pack'/name).resolve(strict=True);need(p.is_relative_to(owned),'owned input containment');raw=p.read_bytes();need(len(raw)==row['bytes']and sha(raw)==row['sha256'],'exact owned language pack '+row['language']);input_paths.append((p,row['sha256']))
  _,pack,_,_=T.entries(raw);need('message_msbt.szs'in pack,'exact message archive');_,files,_,_=T.entries(pack['message_msbt.szs']);need('message.msbt'in files,'exact MSBT provider');selected=messages(files['message.msbt']);lang=row['language']
  for message,tag in((140,23),(150,22)):
   original=selected[message]
   needle=units({'English':'30 arrows','French':'30 flèches','German':'30 Pfeile','Italian':'30 Frecce','Spanish':'30 flechas'}[lang])if message==140 else units('30 '+''.join(chr(v)for v in default)+'bombes')if lang=='French'else units('30 Stück')if lang=='German'else units('30')
   hits=[j for j in range(len(original)-len(needle)+1)if original[j:j+len(needle)]==needle];need(len(hits)==1,'unique authored capacity span '+lang+' '+str(message));at=hits[0]
   insert=[14,2,tag,0]+(default if message==150 and lang=='French'else[]);changed=original[:at]+insert+original[at+len(needle):];off=len(texts);rm=len(removed);texts+=changed;removed+=needle
   r=dict(id=message,original_units=len(original),prefix_units=at,removed_units=len(needle),inserted_units=len(insert),changed_units=len(changed),changed_offset=off,removed_offset=rm);need(all(0<=v<=65535 for v in r.values()),'u16 recipe range');recipes.append(r)
   message_hashes.append(dict(language=lang,message=message,original_units=len(original),original_sha256=sha(struct.pack('>'+str(len(original))+'H',*original)),changed_sha256=sha(struct.pack('>'+str(len(changed))+'H',*changed))))
 names=['id','original_units','prefix_units','removed_units','inserted_units','changed_units','changed_offset','removed_offset']
 c='''/* Original owned EUR text, altered only at the capacity span.
 * Non-owning source borrow is valid only within the native provider context. */
typedef unsigned short u16;
typedef unsigned int u32;
struct recipe {u16 id, original_units, prefix_units, removed_units, inserted_units, changed_units, changed_offset, removed_offset;};
'''
 c+='static const u16 changed_text[] = {'+','.join(str(v)for v in texts)+'};\nstatic const u16 removed_text[] = {'+','.join(str(v)for v in removed)+'};\n'
 c+='static const struct recipe recipes[] = {'+','.join('{'+','.join(str(r[k])for k in names)+'}'for r in recipes)+'};\n\n'
 c+=template.decode().replace('\r\n','\n');generated=c.replace('\r\n','\n').replace('\n','\r\n').encode();need(sha(generated)==cap['generated_policy_sha256'],'exact selected generated C identity')
 for p,d in input_paths:need(sha(p.read_bytes())==d,'owned input changed during generation')
 need((COMP/'build-spec.json').read_bytes()==sr and(ROOT/cap['generator_template']).read_bytes()==template,'authored source inputs unchanged')
 out.mkdir();put(out/'capacity_policy.c',generated)
 report=dict(schema='owned-EUR173-capacity-generation/1',passed=True,contains_game_text=True,source_pack_pins=cap['owned_pack_pins'],generated_policy_sha256=sha(generated),messages=message_hashes,read_only_owned_inputs=True,source_repository_modified=False,game_image_generated=False,warning='PRIVATE GAME-DERIVED OUTPUT. Exclude this whole directory from source backups/publication.')
 put(out/'generation.json',(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(passed=True,contains_game_text=True,generated_policy_sha256=sha(generated),messages=len(message_hashes))))
if __name__=='__main__':main()
