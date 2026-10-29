from pathlib import Path
import difflib
import hashlib
import json
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
HOME = ROOT / 'experiments/maintained-components/single-restore-continuation'


class SingleRestoreSourceTests(unittest.TestCase):
    def test_exact_source_inventory_and_patch(self):
        manifest = json.loads((HOME / 'source-pins.json').read_text())
        self.assertEqual(manifest['hash_normalization'], 'UTF-8 with LF line endings')
        actual = {p.relative_to(HOME).as_posix() for p in HOME.rglob('*') if p.is_file()}
        self.assertEqual(actual, set(manifest['sources']) | {'source-pins.json'})
        for name, row in manifest['sources'].items():
            data = (HOME / name).read_bytes()
            with self.subTest(path=name):
                self.assertNotIn(b'\r', data)
                self.assertNotIn(b'\0', data)
                self.assertEqual((len(data), hashlib.sha256(data).hexdigest()),
                                 (row['bytes'], row['sha256']))
        patch = (HOME / 'single-restore-continuation.patch').read_text()
        targets = [line[4:] for line in patch.splitlines() if line.startswith('+++ ')]
        self.assertEqual(len(targets), 18)
        self.assertEqual(len(set(targets)), 18)
        reconstructed = []
        for target in targets:
            path = Path(target)
            self.assertFalse(path.is_absolute())
            self.assertNotIn('..', path.parts)
            self.assertEqual(path.parts[0], 'candidate')
            before = HOME / 'baseline' / Path(*path.parts[1:])
            after = HOME / path
            reconstructed.extend(difflib.unified_diff(
                before.read_text().splitlines(True) if before.exists() else [],
                after.read_text().splitlines(True),
                fromfile=before.relative_to(HOME).as_posix() if before.exists() else '/dev/null',
                tofile=target))
        self.assertEqual(''.join(reconstructed), patch)

    def test_finite_results_do_not_claim_full_game_support(self):
        manifest = json.loads((HOME / 'source-pins.json').read_text())
        self.assertTrue(manifest['source_only_partial_snapshot'])
        self.assertEqual(manifest['complete_reproducible_integrated_export'], 'v126')
        self.assertEqual(manifest['finite_native_case_counts'], dict(
            heart_current_only_items=2, green_current_only_items=3,
            mixed_qualified_cohorts=0, human_Restore_confirmations_per_passed_case=1,
            genuine_WCT_requests_per_passed_case=2, automatic_native_retry=True))
        self.assertEqual(manifest['new_private_state_bytes'], 160)
        self.assertTrue(manifest['no_WiiU_active_effect_arbitrary_actor_cutscene_cross_area_or_full_game_qualification'])

    def test_actual_source_reservation_policy(self):
        compiler = next((shutil.which(name) for name in ('cl', 'cc', 'clang', 'gcc')
                         if shutil.which(name)), None)
        self.assertIsNotNone(compiler, 'A host C11 compiler is required')
        with tempfile.TemporaryDirectory(dir=os.environ.get('GZ_TEST_TMP')) as folder:
            output = Path(folder) / ('policy.exe' if os.name == 'nt' else 'policy')
            source = HOME / 'source_intent_policy_cases.c'
            if Path(compiler).stem.lower() == 'cl':
                command = [compiler, '/nologo', '/std:c11', '/O2', '/W4', '/WX',
                           '/UNDEBUG', str(source), '/Fe:' + str(output)]
            else:
                command = [compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                           '-UNDEBUG', str(source), '-o', str(output)]
            compiled = subprocess.run(command, cwd=folder, text=True, capture_output=True)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            result = subprocess.run([str(output)], cwd=folder, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
