"""Static named-import metadata serializer; no reviewed target/API execution."""
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib
sys.dont_write_bytecode = True
from elf_static import ELF, need

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
PARENT = ROOT/'work/recovery/source714-minimal-native-init-candidate-r1/Native178-minimal-init-integration.NOT-FOR-INSTALL.bin'
OUTPUT = HERE/'Native178-minimal-init-OSWaitCond-metadata.NOT-FOR-INSTALL.bin'
inputs = []
sha = lambda b: hashlib.sha256(b).hexdigest()
align = lambda n,a: (n+a-1)&~(a-1)

def pin(p):
    b = p.read_bytes()
    return dict(path=p.relative_to(ROOT).as_posix(),bytes=len(b),sha256=sha(b))

def read(p, digest=None):
    b = p.read_bytes()
    need(digest is None or sha(b) == digest, 'input SHA '+str(p))
    inputs.append(pin(p))
    return b

def save(name, data):
    p = HERE/name
    with p.open('x',encoding='utf-8',newline='\n') as f:
        json.dump(data,f,indent=2)
        f.write('\n')
    return pin(p)

raw = read(PARENT,'2eadd54fc5f311266952da3ec8096f9c0fad9a6715d7c7dbe13b36581be9faeb')
read(ROOT/'work/recovery/source714-minimal-native-init-candidate-r1/qualification.json',
     '1a50f1222190e0f50c2838f8aebe1f7dd1789acab2ce7bd7d63d51c01db63805')
context_path = ROOT/'work/recovery/source714-context-family-slice-r1/context-callback-family.o'
read(context_path,'b5ee5b828e5b9997cf8add9501529fc72af1a52617b5040da09301b8dce1508d')
prior = json.loads(read(ROOT/'work/recovery/source711-sdk-binding-r1/report.json',
                       'cd688533b23c49bd0d6754a66bd0a6f1e198564c3e165466c4276e7cc1b224cf'))
official = ROOT/'work/recovery/source714-init-sdk-role-r1/official-rplimportgen.json'
read(official)
parent = ELF(PARENT)
context = ELF(context_path)
need(len(raw) == 30174912 and not OUTPUT.exists(),'new distinct non-installable output')
hoff = struct.unpack_from('>I',raw,32)[0]
stride,count,shstr = struct.unpack_from('>HHH',raw,46)
need((hoff,stride,count,shstr) == (64,40,51,30),'actual51-section archive shape')
need(len(parent.symbols) == 5025 and len(parent.data[28]) == 80400 and
     len(parent.data[29]) == 9983 and len(parent.data[30]) == 618,'actual parent metadata extents')
need(parent.headers[25][1] == 0x80000002 and parent.headers[25][3] == 0xc0008880 and
     parent.data[25] == context.data[23] and len(parent.data[25]) == 8264,
     'whole genuine coreinit8264 identical to current callback Source')
source_wait = context.named('OSWaitCond')
need((source_wait['value'],source_wait['info'],source_wait['size'],source_wait['section']) ==
     (8256,18,0,23),'genuine Source SDK role')
need(parent.data[25][8256:8264] == bytes(8) and
     not [s for s in parent.symbols.values() if s['name'] == 'OSWaitCond' or s['value'] == 0xc000a8c0],
     'unused eight-byte genuine function-import slot, no alias/name')
exports = [v for v in prior['genuine_coreinit_function_exports'] if v['name'] == 'OSWaitCond']
need(len(exports) == 1 and exports[0]['symbol']['info'] == 18 and
     exports[0]['actual_CODE_bytes'] == 280,'retained real named function export evidence')

headers = [list(h) for h in parent.headers]
stored = {i:raw[h[4]:h[4]+h[5]] for i,h in enumerate(parent.headers) if h[1] not in (0,8)}
for i in [28,29,30,49,50]:
    need(not headers[i][2]&0x08000000,'all edited metadata uncompressed')
need(raw[headers[28][4]+headers[28][5]:headers[29][4]] == bytes(48), 'SYM48B zero file gap')
need(raw[headers[29][4]+headers[29][5]:headers[30][4]] == bytes(1), 'STR1B zero file gap insufficient')

def encode(template, hs, blobs):
    ranges = []
    need(len(hs) == 51,'same section count')
    for i,h in enumerate(hs):
        if h[1] in (0,8):
            need(h == list(parent.headers[i]),'NOBITS/null header unchanged')
            continue
        need(len(blobs[i]) == h[5],'stored payload extent')
        begin,end = h[4],h[4]+h[5]
        need(hoff+count*40 <= begin < end <= len(template),'stored payload within complete archive')
        need(all(end <= a or b <= begin for _,a,b in ranges),'all stored payload spans disjoint')
        ranges.append((i,begin,end))
    result = bytearray(template)
    for i,h in enumerate(hs):
        if h[1] not in (0,8):
            result[h[4]:h[4]+h[5]] = blobs[i]
        struct.pack_into('>10I',result,hoff+i*40,*h)
    need(len(result) == len(template),'no implicit archive growth')
    return bytes(result)

need(encode(raw,headers,stored) == raw,'parent parse/serialize roundtrip exact')
name = b'OSWaitCond\0'
symbol = struct.pack('>IIIBBH',9983,0xc000a8c0,0,18,0,25)
need(len(name) == 11 and len(symbol) == 16,'exact import metadata widths')
decoded = list(parent.data)
decoded[28] += symbol
decoded[29] += name
old_string_offset = headers[29][4]
new_string_offset = align(len(raw),64)
new_length = align(new_string_offset+len(decoded[29]),64)
need(new_string_offset == len(raw) and new_length == 30184960,'bounded64B-aligned append')
headers[28][5] += 16
headers[29][4] = new_string_offset
headers[29][5] += 11
headers[30][3] += 16
need(headers[30][3] == 0xc001e6a0,'SHSTR memory shift after expanded SYM')

fi = bytearray(parent.data[50])
old_load,load_align = struct.unpack_from('>II',fi,20)
pad = struct.unpack_from('>I',fi,76)[0]
need((old_load,load_align,pad) == (0x1e984,4,136),'actual FI loadSize/loadAlign/pad')
mapped = []
for i,h in enumerate(headers):
    if not h[2]&2 or not h[3] or not h[5]:
        continue
    size = h[5] if h[1] == 8 else len(decoded[i])
    begin,end = h[3],h[3]+size
    need(end <= 0x100000000 and all(end <= a or b <= begin for _,a,b in mapped),
         'all Native metadata/CODE/DATA memory spans disjoint')
    mapped.append((i,begin,end))
meta_end = max(b for i,a,b in mapped if a >= 0xc0000000)
new_load = align(meta_end-0xc0000000+pad,load_align)
need(new_load == 0x1e994 and new_load == old_load+16,'minimal aligned loader reserve only+16')
struct.pack_into('>I',fi,20,new_load)
decoded[50] = bytes(fi)
need(fi[:20] == parent.data[50][:20] and fi[24:] == parent.data[50][24:],
     'all FI CODE/DATA/temp/tramp/pad/flags unchanged, no encoder4096 adjustment')
crc = bytearray(parent.data[49])
need(len(crc) == 51*4,'actual CRC table count')
for i,h in enumerate(parent.headers):
    want = 0 if i == 49 or h[1] in (0,8) else zlib.crc32(parent.data[i])&0xffffffff
    need(struct.unpack_from('>I',crc,i*4)[0] == want,'all parent CRCs valid')
for i in [28,29,50]:
    struct.pack_into('>I',crc,i*4,zlib.crc32(decoded[i])&0xffffffff)
decoded[49] = bytes(crc)
for i in [28,29,49,50]:
    stored[i] = decoded[i]
template = raw+bytes(new_length-len(raw))
artifact = encode(template,headers,stored)

# Explicit independent byte-edit recipe: only metadata header fields, one
# symbol, three CRC fields, loadSize, and appended STR+padding may change.
ops = []
expected = bytearray(template)
def operation(label,offset,value):
    need(all(offset+len(value) <= x['offset'] or x['offset']+x['bytes'] <= offset
             for x in ops),'nonoverlapping allowed raw edits')
    before = bytes(expected[offset:offset+len(value)])
    expected[offset:offset+len(value)] = value
    ops.append(dict(label=label,offset=offset,bytes=len(value),
                    before=before.hex(),after=value.hex()))
operation('SYM28 sh_size',hoff+28*40+20,struct.pack('>I',headers[28][5]))
operation('STR29 sh_offset',hoff+29*40+16,struct.pack('>I',new_string_offset))
operation('STR29 sh_size',hoff+29*40+20,struct.pack('>I',headers[29][5]))
operation('SHSTR30 sh_addr',hoff+30*40+12,struct.pack('>I',headers[30][3]))
operation('one appended named FUNC18',parent.headers[28][4]+80400,symbol)
operation('FI50 minimal loadSize',headers[50][4]+20,struct.pack('>I',new_load))
for i in [28,29,50]:
    operation('CRC49 record'+str(i),headers[49][4]+i*4,decoded[49][i*4:i*4+4])
expected[new_string_offset:new_string_offset+len(decoded[29])] = decoded[29]
need(bytes(expected) == artifact,'full archive equals exact allowed metadata edits+append')
need(artifact[old_string_offset:old_string_offset+9983] == parent.data[29],
     'old stored STR prefix retained as unreferenced file bytes')
with OUTPUT.open('xb') as f:
    f.write(artifact)
    f.flush()

out = ELF(OUTPUT)
need(OUTPUT.read_bytes() == artifact and artifact[:52] == raw[:52], 'exact saved readback; ELF header/entry unchanged')
changes = []
for i,(old,new) in enumerate(zip(parent.headers,out.headers)):
    for field,(a,b) in enumerate(zip(old,new)):
        if a != b:
            changes.append(dict(section=i,field=field,file_offset=hoff+i*40+field*4,before=a,after=b))
need([(v['section'],v['field']) for v in changes] == [(28,5),(29,4),(29,5),(30,3)],
     'exact four section header changes')
need([i for i,(a,b) in enumerate(zip(parent.data,out.data)) if a != b] == [28,29,49,50],
     'exact four decoded metadata payload changes')
need(out.data[28][:-16] == parent.data[28] and out.data[29][:-11] == parent.data[29],
     'all old symbol/string bytes exact prefixes')
need(all(out.symbols[k] == v for k,v in parent.symbols.items()) and len(out.symbols) == 5026,
     'all old5025 symbol roles/st_other unchanged')
need(out.named('OSWaitCond') == dict(name='OSWaitCond',value=0xc000a8c0,size=0,
     info=18,other=0,section=25,table=28,index=5025),'new genuine named SDK FUNC18')
need(out.rows == parent.rows,'all existing typed RELA rows exact; no new calls/tramp budget')
preserved = []
for i,h in enumerate(parent.headers):
    if i not in [28,29,49,50]:
        need(out.data[i] == parent.data[i],'every other decoded payload exact')
        if h[1] not in (0,8):
            need(artifact[h[4]:h[4]+h[5]] == raw[h[4]:h[4]+h[5]],
                 'all other stored/compressed payload bytes exact')
    want = 0 if i == 49 or out.headers[i][1] in (0,8) else zlib.crc32(out.data[i])&0xffffffff
    need(struct.unpack_from('>I',out.data[49],i*4)[0] == want,'all output CRCs valid')
    if h[1] in (1,8) and h[2]&2 and h[3] and h[5]:
        preserved.append(dict(section=i,kind=h[1],bytes=h[5] if h[1] == 8 else len(parent.data[i]),
                              decoded_sha256=sha(parent.data[i])))
need(len(preserved) == 12,'all twelve real Native spans preserved')
need(encode(artifact,[list(h) for h in out.headers],
     {i:artifact[h[4]:h[4]+h[5]] for i,h in enumerate(out.headers) if h[1] not in (0,8)}) == artifact,
     'output parse/serialize roundtrip exact')
diff_positions = [i for i,(a,b) in enumerate(zip(raw,artifact)) if a != b]
runs = []
for pos in diff_positions:
    if runs and runs[-1][1] == pos:
        runs[-1][1] += 1
    else:
        runs.append([pos,pos+1])
byte_runs = [dict(offset=a,bytes=b-a,before=raw[a:b].hex(),after=artifact[a:b].hex()) for a,b in runs]
diff = dict(schema='initializer178-one-OSWaitCond-import-metadata-successor/1',
    parent=pin(PARENT),artifact=pin(OUTPUT),installable_release=False,
    actual_archive_length_before=len(raw),actual_archive_length_after=len(artifact),
    changed_existing_bytes=len(diff_positions),actual_existing_byte_runs=byte_runs,
    allowed_existing_byte_operations=ops,changed_header_fields=changes,
    changed_decoded_payloads=[28,29,49,50],
    appended_STR=dict(offset=new_string_offset,bytes=len(decoded[29]),sha256=sha(decoded[29]),
                      old_prefix_bytes=9983,old_prefix_sha256=sha(parent.data[29]),
                      new_name_hex=name.hex(),trailing_zero_pad=new_length-new_string_offset-len(decoded[29])),
    old_stored_STR_bytes_preserved=True,all_other_section_file_offsets_preserved=True,
    new_named_import=out.named('OSWaitCond'),coreinit_whole8264_SHA=sha(out.data[25]),
    FileInfo=dict(loadSize_before=old_load,loadSize_after=new_load,loadAlign=load_align,pad=pad,
                  mapped_metadata_end=meta_end,minimal_aligned_reserve_verified=True,
                  every_other_FileInfo_byte_preserved=True),
    all_twelve_real_CODE_DATA_BSS_preserved=preserved,
    all_old_symbol_roles_st_other_RELA_and_import_payloads_preserved=True,
    no_new_call_rows_or_entry_words_or_trampoline_budget=True,
    no_encoder4096_or_TEXT_reserve_change=True,parse_roundtrips=2,
    official_import_format_snapshot=pin(official),retained_real_export=exports[0],
    inputs=inputs,
    limits=['Non-installable static metadata review artifact only; initializer parent already contains its separately reviewed four entry words, preserved unchanged.',
            'No target/PPC/SDK/module load, callback execution, compiler, Source/Native runtime or stopped composition.',
            'OSWaitCond symbol is a real named function import at a genuine reserved slot; it grants no SDK scheduler/lock/context readiness or owner lifetime.',
            'Physical string payload moved by explicit ELF sh_offset; its original file bytes remain unreferenced. Loader acceptance is not hardware tested.',
            'No new callback calls, trampolines, owner aliases, read/save/lifetime services, Setup/version/package/console/network/CP/repo changes.'])
save('diff.json',diff)
print(json.dumps(dict(terminal='STATIC_METADATA_SERIALIZED_AND_VERIFIED',artifact=pin(OUTPUT),
                      diff=pin(HERE/'diff.json'),old_changed_bytes=len(diff_positions),appended_bytes=len(artifact)-len(raw))))
