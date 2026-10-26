from pathlib import Path
import hashlib, json, unittest

ROOT=Path(__file__).resolve().parents[1]
PREFIX='experiments/maintained-components/restore-performance/'

class RestorePerformancePublicationTests(unittest.TestCase):
    def test_complete_manifest_uses_lf_source_hashes(self):
        manifest=json.loads((ROOT/'docs/maintained-components.json').read_text())
        self.assertEqual(manifest['hash_normalization'],'UTF-8 with LF line endings, matching Git source blobs')
        names=[r['path'] for r in manifest['files']]
        self.assertEqual(len(names),len(set(names)))
        for row in manifest['files']:
            data=(ROOT/row['path']).read_bytes().replace(b'\r\n',b'\n')
            with self.subTest(path=row['path']):
                self.assertEqual(len(data),row['bytes'])
                self.assertEqual(hashlib.sha256(data).hexdigest(),row['sha256'])

    def test_snapshot_inventory_and_authored_source_pins(self):
        home=ROOT/PREFIX
        allowed={'README.md','integration-seams.json','integration-seams.txt','source-pins.json',
                 'factory_read_scope.h','factory_read_scope.inc','factory_source_receipt.inc',
                 'members.c','members.h','process.h','observer.h'}
        self.assertEqual({p.name for p in home.iterdir()},allowed)
        pins=json.loads((home/'source-pins.json').read_text())['files']
        for row in pins:
            self.assertIn(row['path'],allowed)
            self.assertEqual(Path(row['path']).name,row['path'])
            data=(home/row['path']).read_bytes()
            self.assertNotIn(b'\0',data)
            self.assertEqual(len(data),row['bytes'])
            self.assertEqual(hashlib.sha256(data).hexdigest(),row['sha256'])

    def test_integration_seams_stay_on_four_relative_authored_inputs(self):
        plan=json.loads((ROOT/PREFIX/'integration-seams.json').read_text())
        self.assertFalse(plan['complete_current_runtime_reproducible'])
        self.assertEqual({r['path'] for r in plan['files']},
                         {'owner.c','mapped_service.c','save_origin/root_access.c','members.c'})
        for row in plan['files']:
            self.assertFalse(Path(row['path']).is_absolute())
            self.assertNotIn('..',Path(row['path']).parts)
            self.assertRegex(row['baseline_sha256'],r'^[0-9a-f]{64}$')
            self.assertRegex(row['selected_sha256'],r'^[0-9a-f]{64}$')
            for hunk in row['hunks']:
                self.assertNotEqual(hunk['before'],hunk['after'])

if __name__=='__main__':
    unittest.main()
