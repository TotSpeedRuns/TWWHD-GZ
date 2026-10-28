from pathlib import Path
import hashlib,json,os,shutil,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
HOME=ROOT/'experiments/maintained-components/source-worker-custodian'
class WorkerCustodianSourceTests(unittest.TestCase):
 def test_exact_LF_source_inventory(self):
  m=json.loads((HOME/'source-pins.json').read_text());self.assertEqual({p.name for p in HOME.iterdir()},{r['path']for r in m['files']}|{'source-pins.json'})
  for r in m['files']:
   b=(HOME/r['path']).read_bytes();self.assertNotIn(b'\r',b);self.assertNotIn(b'\0',b);self.assertEqual((len(b),hashlib.sha256(b).hexdigest()),(r['bytes'],r['sha256']))
 def test_fixed_real_callback_revision_and_ordinal_contract(self):
  s=(HOME/'source_worker_custodian.inc').read_text();a=s[s.index('static u32 swc_work_replaced('):s.index('/* Normal C ABI')]
  self.assertEqual(a.count('_gz_som_work_previous()'),1);self.assertLess(a.index('!swc_current('),a.index('_gz_som_work_previous()'));self.assertLess(a.index('_gz_som_work_previous()'),a.index('!swc_view()'));self.assertLess(a.index('previous+1'),a.index('swc.work_count=purpose'))
  self.assertIn('if(z){if(_gz_roh_registry_revision!=previous)return swc_fail();return z;}',a);self.assertIn('swc.revision!=previous||swc.work_count!=count',a)
  self.assertIn('if(!z&&(swc.work_count!=SWC_WORK_BFLOWER',s);self.assertIn('SWC_ADOPT)||swc.work_count!=SWC_WORK_BFLOWER',s)
 def test_patch_exposes_only_three_custom_runtime_BLs(self):
  p=(HOME/'integration.patch.txt').read_text().split('--- a/runtime-personal.s\n')[1]
  changes=[x for x in p.splitlines()if x.startswith(('+','-'))and not x.startswith('+++')]
  self.assertEqual(sum(x=='-bl _gz_roh_work_replaced'for x in changes),3);self.assertEqual(sum(x.startswith('+bl _gz_swc_')for x in changes),3);self.assertEqual(len(changes),6)
 def test_pin_transfer_and_clean_cancel_have_no_reacquire(self):
  s=(HOME/'source_worker_custodian.inc').read_text();a=s[s.index('u32 _gz_swc_roh_begin('):s.index('u32 _gz_swc_cancel(')];self.assertNotIn('_gz_osa_acquire(',a);self.assertNotIn('_gz_osa_release',a);self.assertIn('_gz_som_pin!=swc.pin',a);self.assertIn('zero(&swc,sizeof(swc));return 0;',a)
  a=s[s.index('u32 _gz_swc_cancel('):s.index('static u32 swc_install_held(u32 slot)')];self.assertLess(a.index('!swc_current('),a.index('osa_release_owned_leaf()'));self.assertIn('swc.borrowed||!swc_idle()',a)
 def test_portable_actual_scalar_policy(self):
  compiler=next((shutil.which(c)for c in ('cl','cc','clang','gcc')if shutil.which(c)),None)
  self.assertIsNotNone(compiler,'A host C compiler is required for scalar-only tests')
  with tempfile.TemporaryDirectory(dir=os.environ.get('GZ_TEST_TMP'))as d:
   dest=Path(d)/'policy.exe';cmd=[compiler,'/nologo','/std:c11','/W4','/WX','/I'+str(HOME),str(HOME/'policy_cases.c'),'/Fe:'+str(dest)]if Path(compiler).stem.lower()=='cl'else[compiler,'-std=c11','-Wall','-Wextra','-Werror','-I',str(HOME),str(HOME/'policy_cases.c'),'-o',str(dest)]
   p=subprocess.run(cmd,cwd=d,text=True,capture_output=True);self.assertEqual(p.returncode,0,p.stdout+p.stderr);p=subprocess.run([dest],text=True,capture_output=True);self.assertEqual(p.returncode,0,p.stdout+p.stderr)
