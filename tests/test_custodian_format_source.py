from pathlib import Path
import hashlib,json,os,shutil,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
HOME=ROOT/'experiments/maintained-components/save-state-performance'
class CustodianFormatSourceTests(unittest.TestCase):
 def test_exact_LF_projection_and_dependencies(self):
  m=json.loads((HOME/'custodian-format-source-pins.json').read_text())
  for row in m['files']:
   b=(HOME/row['path']).read_bytes();self.assertNotIn(b'\r',b);self.assertNotIn(b'\0',b)
   self.assertEqual((len(b),hashlib.sha256(b).hexdigest()),(row['bytes'],row['sha256']))
  p=json.loads((HOME/'custodian-format-integration-pins.json').read_text())
  for row in p['public_dependencies']:
   b=(ROOT/row['path']).read_bytes().replace(b'\r\n',b'\n')
   self.assertEqual((len(b),hashlib.sha256(b).hexdigest()),(row['bytes'],row['sha256']))
  self.assertEqual(p['files'][0]['hunks'],3);self.assertFalse(p['complete_integrated_reproduction'])
 def test_real_custody_pre_and_post_checks_surround_real_leaf(self):
  s=(HOME/'source_pin_format.inc').read_text()
  d=s[s.index('static SFM_API GzSourceDigest sfm_swc_digest('):s.index('static SFM_API u32 sfm_swc_format_check(')]
  for text in ('!gz_scf_stage(', 'swc.error', '!sfm_swc_view()', '!sfm_swc_idle()', '!sfm_swc_context(', 'swc.revision!=_gz_roh_registry_revision','swc.source!=source || swc.used!=used','sfm_osa_digest(before->pin','before->revision!=_gz_roh_registry_revision','!sfm_swc_same(&swc,before)'):
   self.assertIn(text,d)
  a=s[s.index('static SFM_API u32 sfm_swc_format_check('):s.index('SFM_API u32 _gz_som_source_format_check(')]
  self.assertLess(a.index('before=sfm_swc_digest('),a.index('if (plan==SFM_MINT)'))
  self.assertLess(a.index('if (z) return SFM_REFUSE;'),a.index('after=sfm_swc_digest('))
  self.assertLess(a.index('after!=before || sfm_original_pin()!=pin'),a.index('_gz_sfm_memo.mask|='))
  for name in ('pfb','bcb','lgb','b294m','sand','cga'):
   self.assertEqual(a.count('_gz_'+name+'_archive_saved_validate(source,used)'),1)
  self.assertNotIn('(*',a);self.assertNotIn('_gz_swc_begin(',a);self.assertNotIn('_gz_osa_acquire(',a)
 def test_pending_lost_purpose_refuses_but_unissued_decode_stays_full(self):
  s=(HOME/'source_pin_format.inc').read_text()
  self.assertIn('if (scope==SBP_NONE) return _gz_swc_pending()?SFM_REFUSE:SFM_FULL;',s)
  self.assertIn('if (_gz_swc_pending()) return sfm_swc_format_check(source,used,kind);',s)
  self.assertIn('!gz_sfm_full_lease_phase(owner.lease.active,owner.lease.serial,',s)
  self.assertIn('!sfm_original_view()) return SFM_REFUSE;',s)
  self.assertEqual(s.count('GzSourceFormatMemo _gz_sfm_memo '),1)
 def test_current_work_and_native_authority_are_not_cached(self):
  p=json.loads((HOME/'custodian-format-integration-pins.json').read_text())
  self.assertEqual(p['new_DATA_BSS_RO_bytes'],0);self.assertEqual(p['new_hooks'],0);self.assertEqual(p['new_native_imports'],0)
  scope=json.loads((HOME/'scope.json').read_text());n=scope['custodian_format_extension']
  self.assertEqual(n['ordinary_WCT_SSP']['passed'],3);self.assertEqual(n['separate_factory_bombs']['passed'],3)
  self.assertTrue(n['separate_factory_bombs']['ordinary_WCT_unchanged'])
  self.assertFalse(n['comparative_latency_or_speed_claim']);self.assertFalse(n['hardware_tested'])
  self.assertEqual(scope['reproducible_integrated_export'],'v126')
 def test_portable_exact_scalar_predicates(self):
  cc=next((shutil.which(c)for c in ('cl','cc','clang','gcc')if shutil.which(c)),None)
  self.assertIsNotNone(cc,'A host C compiler is required for scalar tests')
  with tempfile.TemporaryDirectory(dir=os.environ.get('GZ_TEST_TMP'))as d:
   exe=Path(d)/'policy.exe'
   args=[cc,'/nologo','/std:c11','/W4','/WX','/I'+str(HOME),str(HOME/'custodian_format_cases.c'),'/Fe:'+str(exe)]if Path(cc).stem.lower()=='cl'else[cc,'-std=c11','-Wall','-Wextra','-Werror','-I',str(HOME),str(HOME/'custodian_format_cases.c'),'-o',str(exe)]
   p=subprocess.run(args,cwd=d,capture_output=True,text=True);self.assertEqual(p.returncode,0,p.stdout+p.stderr)
   p=subprocess.run([exe],capture_output=True,text=True);self.assertEqual(p.returncode,0,p.stdout+p.stderr)
   self.assertIn('scalar cases passed: 13054',p.stdout)
if __name__=='__main__':unittest.main()
