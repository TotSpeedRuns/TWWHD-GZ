from pathlib import Path
import json
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
HOME = ROOT / 'experiments/maintained-components'


class MssOrderTests(unittest.TestCase):
    def test_scope_keeps_order_duration_and_release_separate(self):
        scope = json.loads((HOME / 'mss-source-scope.json').read_text())
        self.assertEqual(scope['delivered_revision'], 'v147')
        self.assertEqual(scope['full_reproducible_export'], 'v126')
        self.assertFalse(scope['complete_current_runtime_reproducible'])
        self.assertFalse(scope['pause_duration_type_authority'])
        self.assertTrue(scope['last_completed_display_latched'])
        self.assertEqual(scope['MSJ_ABI'], 2)
        self.assertEqual(scope['rules'], {
            'Slow': 'Plus-stick-Plus', 'Fast': 'Plus-Plus-stick',
            'Insta': 'Plus-B-stick or B+stick closing sample'})

    def test_portable_actual_classifier_cases(self):
        compiler = next((shutil.which(n) for n in ('cc', 'gcc', 'clang')
                         if shutil.which(n)), None)
        msvc = shutil.which('cl') if os.name == 'nt' else None
        if compiler is None and msvc is None:
            self.skipTest('A compatible C11 host compiler is not installed')
        with tempfile.TemporaryDirectory(dir=os.environ.get('GZ_TEST_TMP')) as d:
            output = Path(d) / ('mss-order-test.exe' if os.name == 'nt' else 'mss-order-test')
            if compiler:
                command = [compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                           '-I', str(HOME), str(HOME / 'mss_order.c'),
                           str(HOME / 'mss_order_cases.c'), '-o', str(output)]
            else:
                command = [msvc, '/nologo', '/std:c11', '/W4', '/WX',
                           '/I' + str(HOME), str(HOME / 'mss_order.c'),
                           str(HOME / 'mss_order_cases.c'), '/Fe:' + str(output)]
            subprocess.run(command, check=True, cwd=d)
            result = subprocess.run([str(output)], check=True, capture_output=True, text=True)
            self.assertIn('33 passed', result.stdout)


if __name__ == '__main__':
    unittest.main()
