"""Explain a bounded local fixed175 JSONL journal. No live imports or sockets."""
from pathlib import Path
import argparse,hashlib,json,ntpath,stat

MAX_BYTES=128*1024
TARGET='5915c5d4b7a37d2e80d49272000747c3003c0bd88ae453f64583f1e86f780a90'
LAYOUT_SHA='257ba7850ad98b9ad7fe5270024a22fcac1547d5247c30ee19861abcd3f2c286'
raw=Path(__file__).with_name('layout.json').read_bytes()
if hashlib.sha256(raw).hexdigest()!=LAYOUT_SHA:raise RuntimeError('Changed fixed175 explanation data')
LAYOUT=json.loads(raw)
RECORDS={r['name']:r['bytes']//4 for r in LAYOUT['records']}

class JournalError(ValueError):pass
def need(value,message):
    if not value:raise JournalError(message)
def uint(value,upper=0xffffffff):return type(value)is int and 0<=value<=upper
def keys(value,expected):need(type(value)is dict and set(value)==set(expected),'Unexpected event fields')
def pairs(items):
    need(len({k for k,v in items})==len(items),'Duplicate JSON key')
    return dict(items)
def invalid_constant(value):raise JournalError('Non-finite JSON value')

def validate(data):
    need(type(data)is bytes and 0<len(data)<=MAX_BYTES,'Local journal must be1..131072 bytes')
    try:
        lines=data.decode('utf-8').splitlines()
        need(1<=len(lines)<=10,'Journal event count must be1..10')
        events=[json.loads(line,object_pairs_hook=pairs,parse_constant=invalid_constant)for line in lines]
    except (UnicodeError,ValueError,RecursionError)as ex:raise JournalError('Malformed JSONL; no samples accepted')from ex
    begin=events[0]
    keys(begin,['kind','schema','offline_prepared','network_opened','target_sha256','profile_candidate','records',
                'snapshot_metadata_bytes','runtime_identity','pointers_followed','console_writes',
                'live_requires_fresh_human_confirmation','requested_samples'])
    need(begin['kind']=='begin'and begin['schema']==LAYOUT['collector_schema']and begin['target_sha256']==TARGET,'Wrong collector schema/target175')
    need(begin['offline_prepared']is True and begin['network_opened']is False and begin['pointers_followed']is False
         and begin['console_writes']is False and begin['live_requires_fresh_human_confirmation']is True,'Unsafe/malformed begin flags')
    need(uint(begin['profile_candidate'])and 0x10000000<=begin['profile_candidate']<0xbffe0000,'Invalid profile candidate scalar')
    need(type(begin['records'])is list and len(begin['records'])==24,'Exactly24 fixed record descriptions required')
    for actual,expected in zip(begin['records'],LAYOUT['records']):
        keys(actual,['name','preferred','bytes'])
        need(type(actual['name'])is str and uint(actual['preferred'])and uint(actual['bytes'])and actual==expected,'Changed fixed record name/address/length')
    need(uint(begin['snapshot_metadata_bytes'])and begin['snapshot_metadata_bytes']==LAYOUT['snapshot_metadata_bytes'],'Wrong metadata byte total')
    need(begin['runtime_identity']==LAYOUT['runtime_identity'],'Wrong identity-scope description')
    requested=begin['requested_samples'];need(uint(requested,8)and requested>=1,'Requested samples must be1..8')
    samples=[];ending=None
    for position,event in enumerate(events[1:],1):
        need(type(event)is dict,'Event must be an object')
        kind=event.get('kind')
        if kind=='sample':
            need(ending is None,'Sample after final event')
            keys(event,['kind','index','records','sequential_metadata_only','atomic_or_owned_snapshot','pointers_followed'])
            need(uint(event['index'],7)and event['index']==len(samples)and len(samples)<requested,'Sample indexes/requested bound mismatch')
            need(event['sequential_metadata_only']is True and event['atomic_or_owned_snapshot']is False and event['pointers_followed']is False,'Unsafe/malformed sample flags')
            need(type(event['records'])is dict and set(event['records'])==set(RECORDS),'Missing/extra fixed records')
            for name,length in RECORDS.items():
                words=event['records'][name]
                need(type(words)is list and len(words)==length and all(uint(w)for w in words),'Exact uint32 record length required: '+name)
            samples.append(event)
        elif kind in('complete','stopped'):
            need(ending is None and position==len(events)-1,'Final event must be last and unique')
            ending=event
            if kind=='complete':
                keys(event,['kind','passed','logical_reads','logical_bytes','no_retries','no_dynamic_pointer_reads'])
                need(event['passed']is True and event['no_retries']is True and event['no_dynamic_pointer_reads']is True,'Unsafe/malformed completion flags')
                need(len(samples)==requested,'Complete event without all requested samples')
                need(uint(event['logical_reads'],400)and len(samples)*24<=event['logical_reads'],'Logical-read bounds invalid')
                need(uint(event['logical_bytes'],32768)and len(samples)*LAYOUT['snapshot_metadata_bytes']<=event['logical_bytes'],'Logical-byte bounds invalid')
            else:
                keys(event,['kind','passed','error'])
                need(event['passed']is False and type(event['error'])is str and len(event['error'])<=4096,'Malformed stopped event')
        else:raise JournalError('Unknown/misordered event kind')
    return begin,samples,ending

def ghs(words):
    if words[:3]!=[0x47485331,128,1]:return dict(status='unrecognized_header_raw_only')
    req,frame,phase,result,command,slot=(words[i]for i in(4,5,3,9,24,7))
    if not(req and frame):status='incomplete_attempt_identity'
    elif phase not in(0,6):status='nonterminal_metadata'
    elif command not in(1,2)or slot>=10:status='incomplete_attempt_identity'
    elif phase==6:status='retained_phase_recorded_not_safe_completion'
    elif result==0:status='terminal_zero_result_not_success_evidence'
    else:status='terminal_result_recorded'
    out=dict(status=status,request=req,frame=frame,phase_raw=phase,result_raw=result,result_hex=f'0x{result:08X}',
             command={1:'Save',2:'Restore'}.get(command,'unknown'),command_raw=command,slot_raw=slot,
             cleanup_raw=words[18:21],held_view_check_raw=words[29],record_only=True)
    if result and status=='terminal_result_recorded':
        out['source_result_description']=LAYOUT['result_descriptions'].get(str(result),'Unknown source result code; preserve raw value.')
    return out

def explain_sample(sample):
    records=sample['records'];a=records['GHS report'];c=records['GHC report'];p=records['RNS pending and retained trace']
    known=a[:3]==[0x47485331,128,1];attempt=known and bool(a[4]and a[5]);capture=dict(status='unrecognized_header_raw_only')
    if c[:3]==[0x41534831,128,10]:
        if not attempt or c[4:6]!=a[4:6]:capture=dict(status='stale_or_unmatched_ignored')
        elif c[29]:capture=dict(status='matching_publication_format_fields',publication_marker_raw=c[29],generation_raw=c[27],type_raw=c[28],stage_raw=c[23])
        elif c[27]in range(0x7100,0x7104):capture=dict(status='matching_unpublished_trace',tag_raw=c[27],tag_hex=hex(c[27]),raw_result=c[28],stage_raw=c[23],detail_raw=c[31],advisory=True)
        else:capture=dict(status='matching_unpublished_raw_only',stage_raw=c[23],word27_raw=c[27],word28_raw=c[28])
    trace=dict(status='no_trace')
    if any(p[12:16]):
        if not attempt or not(p[1]and p[2])or p[1:3]!=a[4:6]or p[15]!=p[2]:trace=dict(status='stale_or_mismatched_trace_ignored')
        else:
            stage=p[12]>>24;step=p[12]&0xffffff;name=LAYOUT['trace_steps'].get(hex(step))
            group=step&0xffff00
            if group in(0x2200,0x3200):
                walk_pass=(step>>4)&15;substep=step&15;limit=3 if group==0x2200 else 7
                name=('RT_RCG_WALK'if group==0x2200 else'RT_RCL_WALK')if walk_pass<=1 and 1<=substep<=limit else None
            trace=dict(status='matching_retained_trace_advisory'if name and stage<=4 else'matching_unknown_or_torn_trace_raw_only',
                       encoded_stage_step_raw=p[12],step_raw=step,raw_result=p[13],opaque_bad_or_detail=p[14],trace_frame=p[15],
                       same_request_frame_words_only=True,advisory=True)
            if name and stage<=4:trace.update(step_name=name,order_stage_name=LAYOUT['trace_phases'][str(stage)])
    trace['config_version_raw']=p[16];trace['config_bits_raw']=p[17]
    hl=records['HL control'];hx=records['HX index header'];lifetime=dict(status='unrecognized_header_raw_only');index=dict(status='unrecognized_header_raw_only')
    if hl[:3]==[0x484c4231,64,1]:
        flags=hl[4];mask=sum(int(x)for x in LAYOUT['hl_flags'])
        lifetime=dict(status='recognized_sticky_metadata',busy_raw=hl[3],flags_raw=flags,
                      flag_names=[n for bit,n in LAYOUT['hl_flags'].items()if flags&int(bit)],unknown_flag_bits=flags&(~mask&0xffffffff),
                      active_heaps_raw=hl[7],active_allocations_raw=hl[8],dropped_raw=hl[14],note='Sticky observations are not an immediate cause or an ownership grant.')
    if hx[:3]==[0x48584831,2164840,2]:index=dict(status='recognized_metadata',ready_raw=hx[3],occupied_historical_nonzero_keys_raw=hx[4],generation_raw=hx[5],note='Occupied includes historical nonzero keys; it is not a live allocation count.')
    return dict(index=sample['index'],GHS=ghs(a),GHC=capture,RNS=trace,HL=lifetime,HX=index,
                H=dict(available=False,reason='No GHW record exists in the24-record readset.'),raw_records=records)

def explain_bytes(data):
    begin,samples,ending=validate(data)
    complete=ending is not None and ending['kind']=='complete'
    return dict(schema='offline-fixed175-explanation/1',target_sha256=TARGET,
                journal_status='complete_collection'if complete else'incomplete_stopped'if ending else'incomplete_EOF',
                requested_samples=begin['requested_samples'],valid_samples=len(samples),samples=[explain_sample(s)for s in samples],
                stopped_error=ending['error']if ending and ending['kind']=='stopped'else None,
                atomic_or_owned_snapshot=False,pointers_followed=False,live_connection_opened=False,
                displayed_toast_observed=False,success_established=False,
                limits=['Expected target175 is a journal claim; this offline tool does not authenticate the running executable.',
                        'Sequential fixed metadata may be torn or stale; matching request/frame words do not establish freshness, ownership or causality.',
                        'Collection completion is not a Save/Restore completion. Zero result never establishes success.',
                        'No displayed message, GHW/H reason, payload or physical console result is observed.'])

def explain_path(path):
    text=str(path);drive,_=ntpath.splitdrive(text)
    need(not text.startswith(('\\\\','//'))and not drive.startswith(('\\\\','//'))and '://'not in text,'Only local regular files are accepted')
    p=Path(path);need(not p.is_symlink()and stat.S_ISREG(p.stat().st_mode),'Local regular file required')
    with p.open('rb')as f:data=f.read(MAX_BYTES+1)
    return explain_bytes(data)

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('journal',type=Path);a=ap.parse_args()
    try:result=explain_path(a.journal)
    except (JournalError,OSError)as ex:ap.error(str(ex))
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()
