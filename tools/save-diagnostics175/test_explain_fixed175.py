"""Meaningful host-only validation/attribution tests. No collector imports."""
from pathlib import Path
import copy,importlib.util,json,subprocess,sys,unittest
from unittest import mock
H=Path(__file__).resolve().parent
s=importlib.util.spec_from_file_location('offline_fixed175',H/'explain_fixed175.py');D=importlib.util.module_from_spec(s);s.loader.exec_module(D)
def begin(requested=1):
    return dict(kind='begin',schema='fixed175-diagnostic-collector-plan/1',offline_prepared=True,network_opened=False,
                target_sha256=D.TARGET,profile_candidate=281946416,records=copy.deepcopy(D.LAYOUT['records']),
                snapshot_metadata_bytes=2124,runtime_identity=D.LAYOUT['runtime_identity'],pointers_followed=False,
                console_writes=False,live_requires_fresh_human_confirmation=True,requested_samples=requested)
def sample(index=0):
    r={name:[0]*n for name,n in D.RECORDS.items()}
    r['GHS report'][:3]=[0x47485331,128,1];g=r['GHS report'];g[4]=11;g[5]=101;g[7]=0;g[24]=1
    r['GHC report'][:3]=[0x41534831,128,10];r['GHC report'][4:6]=[11,101]
    r['HL control'][:3]=[0x484c4231,64,1]
    r['HX index header'][:3]=[0x48584831,2164840,2]
    return dict(kind='sample',index=index,records=r,sequential_metadata_only=True,atomic_or_owned_snapshot=False,pointers_followed=False)
def complete(n=1):return dict(kind='complete',passed=True,logical_reads=38*n,logical_bytes=2400*n,no_retries=True,no_dynamic_pointer_reads=True)
def encoded(events):return('\n'.join(json.dumps(e,separators=(',',':'))for e in events)+'\n').encode()
def explain(s=None,ending=True):return D.explain_bytes(encoded([begin(),s or sample()]+([complete()]if ending else[])))

class Tests(unittest.TestCase):
    def refused(self,events):
        with self.assertRaises(D.JournalError):D.explain_bytes(encoded(events))
    def test_zero_terminal_is_not_success_or_toast(self):
        out=explain();self.assertEqual(out['samples'][0]['GHS']['status'],'terminal_zero_result_not_success_evidence')
        self.assertFalse(out['success_established']);self.assertFalse(out['displayed_toast_observed']);self.assertFalse(out['samples'][0]['H']['available'])
    def test_nonterminal_nonzero_is_raw_only(self):
        s=sample();s['records']['GHS report'][3]=3;s['records']['GHS report'][9]=0xb43
        g=explain(s)['samples'][0]['GHS'];self.assertEqual(g['status'],'nonterminal_metadata');self.assertNotIn('source_result_description',g)
    def test_retained_phase_does_not_infer_completion(self):
        s=sample();s['records']['GHS report'][3]=6;self.assertEqual(explain(s)['samples'][0]['GHS']['status'],'retained_phase_recorded_not_safe_completion')
    def test_result_description_is_source_record_only(self):
        s=sample();s['records']['GHS report'][9]=0xb43;out=explain(s)
        self.assertIn('workspace',out['samples'][0]['GHS']['source_result_description']);self.assertFalse(out['success_established'])
    def test_stale_GHC_ignored(self):
        s=sample();c=s['records']['GHC report'];c[5]=100;c[27:30]=[0x7103,4,0]
        x=explain(s)['samples'][0]['GHC'];self.assertEqual(x,dict(status='stale_or_unmatched_ignored'))
    def test_GHC_needs_nonzero_GHS_request_and_frame(self):
        for field in(4,5):
            s=sample();s['records']['GHS report'][field]=0;s['records']['GHC report'][field]=0
            self.assertEqual(explain(s)['samples'][0]['GHC']['status'],'stale_or_unmatched_ignored')
    def test_unpublished_GHC_tags_are_raw(self):
        for tag in range(0x7100,0x7104):
            s=sample();c=s['records']['GHC report'];c[27:30]=[tag,4,0];c[31]=0x53500000
            x=explain(s)['samples'][0]['GHC'];self.assertEqual(x['tag_raw'],tag);self.assertEqual(x['detail_raw'],0x53500000)
            self.assertNotIn('required_bytes',x)
    def test_published_GHC_fields_are_generation_type(self):
        s=sample();s['records']['GHC report'][27:30]=[0x7103,10,1]
        x=explain(s)['samples'][0]['GHC'];self.assertEqual(x['generation_raw'],0x7103);self.assertEqual(x['type_raw'],10);self.assertNotIn('tag_raw',x)
    def trace_sample(self):
        s=sample();p=s['records']['RNS pending and retained trace'];p[1:3]=[11,101];p[12:18]=[(4<<24)|0x2203,5,0xdeadbeef,101,2,8192];return s
    def test_RNS72_prefix_and_config_are_separate(self):
        x=explain(self.trace_sample())['samples'][0]['RNS'];self.assertEqual(x['step_name'],'RT_RCG_WALK')
        self.assertEqual(x['raw_result'],5);self.assertEqual(x['opaque_bad_or_detail'],0xdeadbeef)
        self.assertEqual((x['config_version_raw'],x['config_bits_raw']),(2,8192));self.assertTrue(x['advisory'])
    def test_stale_RNS_request_frame_and_retained_frame_ignored(self):
        for index in(1,2,15):
            s=self.trace_sample();s['records']['RNS pending and retained trace'][index]+=1
            x=explain(s)['samples'][0]['RNS'];self.assertEqual(x['status'],'stale_or_mismatched_trace_ignored');self.assertNotIn('step_name',x)
    def test_zero_pending_and_unknown_trace_are_not_success(self):
        self.assertEqual(explain()['samples'][0]['RNS']['status'],'no_trace')
        s=self.trace_sample();s['records']['RNS pending and retained trace'][12]=0xff00ffff
        x=explain(s)['samples'][0]['RNS'];self.assertEqual(x['status'],'matching_unknown_or_torn_trace_raw_only');self.assertNotIn('step_name',x)
    def test_high_step_bits_do_not_alias_walk_families(self):
        for step in(0x12203,0x13203):
            s=self.trace_sample();s['records']['RNS pending and retained trace'][12]=(4<<24)|step
            x=explain(s)['samples'][0]['RNS'];self.assertEqual(x['status'],'matching_unknown_or_torn_trace_raw_only');self.assertNotIn('step_name',x)
    def test_wrong_report_headers_are_raw_only(self):
        for record,name in[('GHS report','GHS'),('GHC report','GHC'),('HL control','HL'),('HX index header','HX')]:
            s=self.trace_sample();s['records'][record][0]^=1
            out=explain(s)['samples'][0];self.assertEqual(out[name],dict(status='unrecognized_header_raw_only'))
            self.assertEqual(out['raw_records'][record],s['records'][record])
    def test_absent_GHS_identity_cannot_attribute_retained_RNS(self):
        for zero_header in(False,True):
            s=self.trace_sample();g=s['records']['GHS report']
            if zero_header:g[:3]=[0,0,0]
            else:g[4:6]=[0,0]
            x=explain(s)['samples'][0]['RNS'];self.assertEqual(x['status'],'stale_or_mismatched_trace_ignored');self.assertNotIn('step_name',x)
    def test_HL_sticky_and_HX_historical_count(self):
        s=sample();s['records']['HL control'][4]=8|512;s['records']['HL control'][8]=1
        s['records']['HX index header'][4]=262144;x=explain(s)['samples'][0]
        self.assertEqual(x['HL']['flag_names'],['OVERFLOW','STALE_COMPLETION'])
        self.assertEqual(x['HX']['occupied_historical_nonzero_keys_raw'],262144);self.assertNotIn('cause',x['HL'])
    def test_pointer_poison_is_opaque_and_causes_no_IO(self):
        s=sample()
        for name,words in s['records'].items():
            if name not in('GHS report','GHC report','HL control','HX index header','RNS pending and retained trace'):s['records'][name]=[0xffffffff]*len(words)
        s['records']['GHC report'][8:13]=[0xffffffff]*5
        with mock.patch.object(Path,'open',side_effect=AssertionError('Unexpected value-selected file read')):
            out=explain(s)
        self.assertEqual(out['samples'][0]['raw_records']['mapped_backend'],[0xffffffff]*17);self.assertFalse(out['pointers_followed'])
    def test_stopped_and_EOF_keep_valid_samples_incomplete(self):
        for end,status in[([], 'incomplete_EOF'),([dict(kind='stopped',passed=False,error='TimeoutError: bounded stop')],'incomplete_stopped')]:
            out=D.explain_bytes(encoded([begin(2),sample(),*end]));self.assertEqual(out['journal_status'],status);self.assertEqual(out['valid_samples'],1)
    def test_stopped_before_first_sample_has_no_evidence(self):
        out=D.explain_bytes(encoded([begin(),dict(kind='stopped',passed=False,error='ConnectionError')]))
        self.assertEqual(out['valid_samples'],0);self.assertEqual(out['journal_status'],'incomplete_stopped')
    def test_truncated_or_malformed_lines_reject_whole_journal(self):
        for suffix in(b'{"kind":"sample"',b'\n',b'false\n',b'{"x":NaN}\n'):
            with self.assertRaises(D.JournalError):D.explain_bytes(encoded([begin(),sample()])+suffix)
    def test_wrong_target_schema_and_layout_reject(self):
        for field,value in[('target_sha256','0'*64),('schema','fixed173-diagnostic-collector-plan/1')]:
            b=begin();b[field]=value;self.refused([b,sample(),complete()])
        for field,value in[('name','evil'),('preferred',0),('bytes',8)]:
            b=begin();b['records'][0][field]=value;self.refused([b,sample(),complete()])
    def test_wrong_record_lengths_types_and_uint32(self):
        for value in([0]*31,[0]*33,[True]*32,[-1]*32,[0x100000000]*32,[1.0]*32,'poison'):
            s=sample();s['records']['GHS report']=value;self.refused([begin(),s,complete()])
    def test_boolean_numeric_fields_and_safety_flags_reject(self):
        for field in('profile_candidate','snapshot_metadata_bytes','requested_samples'):
            b=begin();b[field]=True;self.refused([b,sample(),complete()])
        for field,value in[('index',False),('atomic_or_owned_snapshot',True),('pointers_followed',True),('sequential_metadata_only',1)]:
            s=sample();s[field]=value;self.refused([begin(),s,complete()])
    def test_order_requested_and_event_bounds(self):
        self.refused([begin(0),sample(),complete()]);self.refused([begin(9),sample(),complete()])
        s=sample(1);self.refused([begin(),s,complete()]);self.refused([begin(2),sample(),complete()])
        self.refused([begin(),sample(),complete(),sample(1)])
        self.refused([begin(8),*[sample(i%8)for i in range(9)],complete(8)])
    def test_complete_counter_bounds_and_bools(self):
        for field,value in[('logical_reads',401),('logical_bytes',32769),('logical_reads',True),('logical_bytes',False),('logical_reads',0),('logical_bytes',0)]:
            c=complete();c[field]=value;self.refused([begin(),sample(),c])
    def test_eight_samples_and_exact_limits(self):
        events=[begin(8),*[sample(i)for i in range(8)],complete(8)];events[-1].update(logical_reads=400,logical_bytes=32768)
        self.assertEqual(D.explain_bytes(encoded(events))['valid_samples'],8)
    def test_duplicate_keys_oversized_and_bad_encoding(self):
        for raw in(b'{"kind":"begin","kind":"sample"}\n',b' '* (D.MAX_BYTES+1),b'\xff'):
            with self.assertRaises(D.JournalError):D.explain_bytes(raw)
    def test_local_only_before_filesystem_access(self):
        with mock.patch.object(Path,'stat',side_effect=AssertionError('Network path touched')):
            for name in('\\\\server\\share\\file','//server/share/file','https://example/file'):
                with self.assertRaises(D.JournalError):D.explain_path(name)
    def test_cli_fails_without_emitting_partial_explanation(self):
        folder=H/'unit-fixtures';folder.mkdir(exist_ok=True);p=folder/'malformed.jsonl';p.write_bytes(encoded([begin(),sample()])+b'{')
        r=subprocess.run([sys.executable,'-B',str(H/'explain_fixed175.py'),str(p)],capture_output=True,text=True)
        self.assertNotEqual(r.returncode,0);self.assertEqual(r.stdout,'');self.assertIn('Malformed JSONL',r.stderr)
    def test_no_live_or_network_imports(self):
        import ast
        tree=ast.parse((H/'explain_fixed175.py').read_text())
        imports=[n.name for x in ast.walk(tree)if isinstance(x,ast.Import)for n in x.names]
        imports += [x.module or''for x in ast.walk(tree)if isinstance(x,ast.ImportFrom)]
        self.assertFalse(any('socket'in n or 'record175'in n or 'collector'in n for n in imports))

if __name__=='__main__':
    result=unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(Tests))
    report=dict(schema='offline-fixed175-explainer-unit-tests/1',passed=result.wasSuccessful(),tests=result.testsRun,
                failures=len(result.failures),errors=len(result.errors),live_collector_imported=False,network_or_target_execution=False)
    (H/'test-report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    raise SystemExit(0 if result.wasSuccessful()else 1)
