"""Pure local decoder for existing RnsPending64 word records; no memory I/O."""
from pathlib import Path
import json
ENUM=json.loads((Path(__file__).resolve().parent/'diagnostic_enum.json').read_text())
def decode_pending(words,expected_request=None,expected_frame=None):
 if type(words)is not list or len(words)!=16 or any(type(x)is not int or not 0<=x<=0xffffffff for x in words):raise ValueError('Exact16 unsigned pending words required')
 a,raw,bad,frame=words[12:16];phase=a>>24;step=a&0xffffff
 if not any(words[12:16]):return dict(observation='no_trace',atomic=False,authority=False)
 name=ENUM['steps'].get(hex(step));substep=None;walk_pass=None
 if step&0xff00 in(0x2200,0x3200):
  name='RT_RCG_WALK'if step&0xff00==0x2200 else'RT_RCL_WALK';walk_pass=(step>>4)&15;substep=step&15
 recognized=name is not None and phase in range(5)
 local_match=bool(words[1] and words[2] and frame==words[2])
 supplied_match=None if expected_request is None or expected_frame is None else local_match and (words[1],words[2])==(expected_request,expected_frame)
 return dict(observation='recognized_trace'if recognized else'unknown_or_torn_trace',step=step,step_name=name,phase=phase,phase_name=ENUM['phase'].get(str(phase)),raw_result=raw,bad_or_detail=bad,trace_frame=frame,pending_request=words[1],pending_frame=words[2],pending_frame_matches=local_match,matches_supplied_attempt=supplied_match,walk_pass=walk_pass,walk_substep=substep,atomic=False,authority=False,scope='Advisory sequential fixed metadata only; no freshness or admission grant.')
