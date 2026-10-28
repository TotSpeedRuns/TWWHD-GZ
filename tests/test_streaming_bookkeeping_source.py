from pathlib import Path
import importlib.util
import json
import os
import random
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
HOME = ROOT / 'experiments/maintained-components/streaming-bookkeeping-v148'
spec = importlib.util.spec_from_file_location('streaming_models', HOME/'portable_models.py')
models = importlib.util.module_from_spec(spec)
spec.loader.exec_module(models)


class StreamingBookkeepingTests(unittest.TestCase):
    def test_scope_keeps_partial_source_and_no_hardware_claim(self):
        scope = json.loads((HOME/'scope.json').read_text())
        self.assertEqual(scope['delivered_revision'], 'v148')
        self.assertEqual(scope['full_reproducible_export'], 'v126')
        self.assertFalse(scope['complete_current_runtime_reproducible'])
        self.assertFalse(scope['WiiU_performance_fix_verified'])
        self.assertFalse(scope['WORLD_READY'])
        self.assertFalse(scope['READ_READY'])
        self.assertFalse(scope['triangular_probe_included'])

    def test_collider_cache_and_overflow_preserve_all_targets(self):
        for matches in (0, 1, 2, 31, 32, 33, 64, 8192):
            rows = [(0x10010000+i*0x100, 1, 1) if i < matches else (0, 0, 0) for i in range(8192)]
            n = matches*0x100 or 4
            self.assertEqual(models.collider_plan(rows, 0x10010000, n, matches, 1, False),
                             models.collider_plan(rows, 0x10010000, n, matches, 1, True))
        rows = [(0x10010000, 1, 1)] + [(0, 0, 0)]*8191
        for bad in ((1, 0, 0), (0, 1, 0), (0x10020000, 2, 1), (0x10020000, 0, 1), (0x10020000, 1, 3), (0x10020000, 0, 7)):
            corrupted = rows[:]
            corrupted[-1] = bad
            self.assertIsNone(models.collider_plan(corrupted, 0x10010000, 4, 1, 1, True))
        self.assertIsNone(models.collider_plan(rows, 0x10010000, 4, 0, 1, True))
        self.assertIsNone(models.collider_plan(rows, 0xbffffffc, 8, 1, 1, True))

    def test_sdk_repeat_rule_reaches_the_same_metadata_closure(self):
        rng = random.Random(148)
        # Exercise lower/higher-index dependent heaps and exact/unknown backing.
        for case in range(128):
            heaps = [[1, 0x1000+i*0x100, i+1, 0, 0, 0, 0, 0] for i in range(8)]
            allocations = []
            for i in range(16):
                parent = rng.randrange(8)
                allocations.append([1, heaps[parent][1], heaps[parent][2], 0x100000+i*0x100, i+1, 0])
            for i, h in enumerate(heaps):
                r = allocations[rng.randrange(len(allocations))]
                if case & 1:
                    h[3] = r[3]
                else:
                    h[4:7] = [r[1], r[3], r[4]]
            heaps[case % 8][0] = 2
            heaps[case % 8][7] = 42
            old = models.sdk_drain(heaps, allocations, False)
            new = models.sdk_drain(heaps, allocations, True)
            self.assertEqual(old[:2], new[:2])
            self.assertLessEqual(new[2], old[2])
        empty = [[1, 1, 1, 0, 0, 0, 0, 0]]
        self.assertEqual(models.sdk_drain(empty, [], True)[2], 1)

    def test_portable_exact_existing_key_selectors(self):
        compiler = next((shutil.which(n) for n in ('cc', 'gcc', 'clang') if shutil.which(n)), None)
        msvc = shutil.which('cl') if os.name == 'nt' else None
        if compiler is None and msvc is None:
            self.skipTest('A compatible C11 host compiler is not installed')
        with tempfile.TemporaryDirectory(dir=os.environ.get('GZ_TEST_TMP')) as d:
            output = Path(d)/('lookup.exe' if os.name == 'nt' else 'lookup')
            command = ([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', str(HOME/'lookup_cases.c'), '-o', str(output)]
                       if compiler else [msvc, '/nologo', '/std:c11', '/W4', '/WX', str(HOME/'lookup_cases.c'), '/Fe:'+str(output)])
            subprocess.run(command, check=True, cwd=d)
            result = subprocess.run([str(output)], check=True, capture_output=True, text=True)
            self.assertIn('HL selector cases: 21 passed', result.stdout)


if __name__ == '__main__':
    unittest.main()
