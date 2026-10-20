from pathlib import Path
import hashlib, json, os, struct, sys, tempfile, unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from local_assets import materialize, metadata, extract
from audit_source import audit, files

class PublicationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if os.environ.get('GZ_TEST_TMP'):tempfile.tempdir=os.environ['GZ_TEST_TMP']
    def fixture(self,directory):
        root=Path(directory);data=struct.pack('>2I',0x01020304,0xffeeddcc)
        meta={'format':'TWWHD-GZ-local-assets/1','accepted_practice_RPX_sha256':[],
              'assets':[dict(name='_gz_test',bytes=8,address=0x10000000,sha256=hashlib.sha256(data).hexdigest())]}
        manifest=root/'manifest.json';manifest.write_text(json.dumps(meta))
        template=root/'template.asm.in';template.write_text('_gz_test:\n.asset _gz_test\n_next:\n.int 0\n')
        (root/'_gz_test.bin').write_bytes(data);return root,manifest,template
    def test_exact_asset_materialization(self):
        with tempfile.TemporaryDirectory()as d:
            root,m,t=self.fixture(d)
            self.assertEqual(materialize(t,root,m),'_gz_test:\n.int 0x01020304,0xFFEEDDCC\n\n_next:\n.int 0\n')
    def test_modified_asset_rejected(self):
        with tempfile.TemporaryDirectory()as d:
            root,m,t=self.fixture(d);(root/'_gz_test.bin').write_bytes(bytes(8))
            with self.assertRaisesRegex(ValueError,'mismatch'):materialize(t,root,m)
    def test_missing_directive_rejected(self):
        with tempfile.TemporaryDirectory()as d:
            root,m,t=self.fixture(d);t.write_text('.int 0\n')
            with self.assertRaisesRegex(ValueError,'differ'):materialize(t,root,m)
    def test_path_traversal_rejected(self):
        with tempfile.TemporaryDirectory()as d:
            root,m,t=self.fixture(d);meta=json.loads(m.read_text());meta['assets'][0]['name']='../outside';m.write_text(json.dumps(meta))
            with self.assertRaisesRegex(ValueError,'Invalid'):metadata(m)
    def test_duplicate_directive_rejected(self):
        with tempfile.TemporaryDirectory()as d:
            root,m,t=self.fixture(d);t.write_text('.asset _gz_test\n.asset _gz_test\n')
            with self.assertRaisesRegex(ValueError,'duplicate'):materialize(t,root,m)
    def test_wrong_practice_revision_rejected_before_output(self):
        with tempfile.TemporaryDirectory()as d:
            root,m,t=self.fixture(d);rpx=root/'wrong.rpx';rpx.write_bytes(b'wrong');dest=root/'output'
            with self.assertRaisesRegex(ValueError,'matching'):extract(rpx,dest,m)
            self.assertFalse(dest.exists())
    def test_source_tree_passes(self):self.assertGreater(len(audit()),200)
    def test_binary_in_source_rejected(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);(root/'tools').mkdir();(root/'tools/bad.py').write_bytes(b'bad\0file')
            with self.assertRaisesRegex(ValueError,'Binary'):list(files(root))
    def test_private_paths_rejected(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);(root/'tools').mkdir();(root/'tools/bad.py').write_text('C:'+chr(47)+'Users'+chr(47)+'private')
            with self.assertRaisesRegex(ValueError,'Private'):list(files(root))
    def test_local_assets_excluded(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);(root/'local-assets').mkdir();(root/'local-assets/pixels.bin').write_bytes(bytes(16))
            self.assertEqual(list(files(root)),[])
if __name__=='__main__':unittest.main()
