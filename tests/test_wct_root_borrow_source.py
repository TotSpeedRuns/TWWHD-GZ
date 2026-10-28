"""Source contracts only; no native API, Gate or owner success fixtures."""
from pathlib import Path
import hashlib
import json
import unittest

ROOT=Path(__file__).resolve().parents[1]
HOME=ROOT/'experiments/maintained-components/wct-root-borrow'
def read(name):return (HOME/name).read_text()
def contract_borrower(text):
    return all(x in text for x in (
        'whl.identity!=whl_root.identity',
        '_gz_wct_heap_held(whl.token,frame)',
        '_gz_srh_wct_root_context(kind,mode,serial,frame)',
        '_gz_hg_platform_thread()!=whl_root.thread',
        '_gz_wct_heap_identity(whl.token,frame)==whl_root.identity'))

class RootBorrowSourceTests(unittest.TestCase):
    def test_pinned_source_inventory_is_text_lf_only(self):
        manifest=json.loads(read('source-pins.json'))
        actual={p.relative_to(HOME).as_posix() for p in HOME.rglob('*') if p.is_file()}
        self.assertEqual(actual,{r['path'] for r in manifest['files']}|{'source-pins.json'})
        for row in manifest['files']:
            rel=Path(row['path']);self.assertFalse(rel.is_absolute());self.assertNotIn('..',rel.parts)
            b=(HOME/rel).read_bytes()
            with self.subTest(file=row['path']):
                self.assertNotIn(b'\0',b);self.assertNotIn(b'\r',b)
                self.assertEqual((len(b),hashlib.sha256(b).hexdigest()),(row['bytes'],row['sha256']))

    def test_exact_whole_provider_inverse_and_readable_diff(self):
        plan=json.loads(read('integration-seams.json'))
        self.assertEqual(len(plan['seams']),17)
        files={r['file']:r for r in plan['files']}
        for name in ('wct_heap_lease.inc','wct_native_context.inc'):
            text=read(name);key='bomb/'+name
            self.assertEqual(hashlib.sha256(text.encode()).hexdigest(),files[key]['selected_sha256'])
            for seam in reversed([s for s in plan['seams'] if s['file']==key]):
                self.assertEqual(text.count(seam['after']),seam['count'])
                text=text.replace(seam['after'],seam['before'])
            self.assertEqual(hashlib.sha256(text.encode()).hexdigest(),files[key]['baseline_sha256'])
        diff=read('integration-seams.txt')
        for name in files:self.assertIn('+++ b/'+name,diff)
        self.assertFalse(plan['complete_current_runtime_reproducible'])

    def test_getter_fragments_match_exact_declared_seams(self):
        plan=json.loads(read('integration-seams.json'));seams={s['name']:s for s in plan['seams']}
        for file,key in [('save_origin/clock_access.inc','sck_access-borrow-real-WHL-or-keep-original-independent-path'),('save_origin/persistent_access.inc','spk_access-borrow-real-WHL-or-keep-original-independent-path')]:
            self.assertEqual(read(file).strip(),seams[key]['after'].strip())
        seam=seams['fixed-kernel-metadata-producer-plus-fresh-real-seal']
        self.assertTrue(seam['after'].endswith(seam['before']))
        self.assertEqual(read('save_origin/root_context.inc'),seam['after'][:-len(seam['before'])])

    def test_borrower_authenticates_real_identity_and_has_no_admission(self):
        body=read('wct_root_borrow.inc')
        self.assertTrue(contract_borrower(body))
        self.assertFalse(contract_borrower(body.replace('whl.identity!=whl_root.identity','0')))
        for api in ('_gz_hg_capture_begin(','_gz_hg_capture_end(','_gz_jca_exclusive_try(','_gz_jca_exclusive_leave('):
            self.assertNotIn(api,body)
        self.assertIn('sizeof(WhlRootBorrow)==32',body)
        self.assertIn('whl.borrowed=4+kind',body)

    def test_same_gate_is_neither_copied_nor_initialized(self):
        body=read('wct_job_bridge.inc')
        self.assertIn('_gz_jca_exclusive_try(&jwa_gate,&_gz_wct_job_token)',body)
        self.assertIn('_gz_jca_exclusive_leave(&jwa_gate,&_gz_wct_job_token)',body)
        self.assertIn('t->gate==&jwa_gate',body)
        self.assertNotIn('_gz_jca_init(',body)
        self.assertNotIn('GzJcaGate jwa_gate',body)

    def test_failed_pending_borrow_cannot_select_independent_fallback(self):
        for file,kind,serial in [('save_origin/clock_access.inc','WHL_ROOT_CLOCK','_gz_sck_serial'),('save_origin/persistent_access.inc','WHL_ROOT_PERSIST','_gz_spk_serial')]:
            body=read(file)
            self.assertIn('if(_gz_wct_heap_pending()){borrowed=1;gate=_gz_wct_root_borrow('+kind+',mode,'+serial+',frame);}',body)
            self.assertIn('else gate=_gz_hg_capture_begin('+serial+');',body)
            self.assertEqual(body.count('srh_wct_seal('+kind),3)
            self.assertIn('if(!gate){z=86;goto done;}',body)
            self.assertIn('borrowed?_gz_wct_root_unborrow('+kind+',mode,'+serial+',frame,gate):_gz_hg_capture_end(gate)',body)

    def test_pending_borrow_blocks_real_end_and_leaves_actual_tokens_owned(self):
        body=read('wct_heap_lease.inc')
        release=body[body.index('static u32 whl_release_actual'):body.index('u32 _gz_wct_heap_begin')]
        self.assertLess(release.index('_gz_wct_root_pending()'),release.index('_gz_jwa_wct_leave()'))
        self.assertLess(release.index('_gz_jwa_wct_leave()'),release.index('_gz_hg_capture_end(whl.hg)'))
        end=body[body.index('u32 _gz_wct_heap_end'):body.index('void _gz_wct_heap_finish_leaf')]
        self.assertLess(end.index('_gz_wct_root_pending()'),end.index('if(!_gz_wct_heap_pending())'))
        unborrow=read('wct_root_borrow.inc').split('u32 _gz_wct_root_unborrow',1)[1]
        self.assertLess(unborrow.index('_gz_wct_root_held('),unborrow.index('bcl_zero(&whl_root'))
        self.assertNotIn('_gz_wct_job_token.active=',unborrow)
        native=read('wct_native_context.inc')
        self.assertIn('if(_gz_wct_heap_pending()||_gz_wct_root_pending())whl_native.halted=1;',native)
        self.assertIn('whl_native.active=0;',native)

    def test_scope_does_not_promote_private_evidence(self):
        scope=json.loads(read('scope.json'))
        self.assertEqual(scope['full_reproducible_export'],'v126')
        for key in ('complete_current_runtime_reproducible','standalone_library','READ_READY','WORLD_READY','delivered_release_claim','current_only_actor_retirement_included','saved_format_memo_extension_included','unmerged_raw_copy_or_triangular_experiments_included'):
            self.assertFalse(scope[key])
        self.assertEqual(scope['new_root_borrow_private_bytes'],32)
        cases=scope['Cemu_observations']
        self.assertEqual(cases['holstered_save_drawn_current_restores'],3)
        self.assertEqual(cases['standalone_quiet_bomb_expiry_restores'],3)
        self.assertFalse(cases['WiiU_qualification'])
        self.assertFalse(cases['controlled_benchmark'])

if __name__=='__main__':unittest.main()
