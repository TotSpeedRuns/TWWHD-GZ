from pathlib import Path
import hashlib
import json
import unittest

ROOT = Path(__file__).resolve().parents[1]
HOME = ROOT / 'experiments/maintained-components'


class GenericObservationSourceTests(unittest.TestCase):
    def test_bounded_partial_scope_is_explicit(self):
        scope = json.loads((HOME / 'generic-cpu-observation/scope.json').read_text())
        self.assertEqual(scope['full_reproducible_export'], 'v126')
        self.assertFalse(scope['complete_current_runtime_reproducible'])
        self.assertFalse(scope['native_capture_claimed'])
        self.assertFalse(scope['replay_or_restore'])
        self.assertEqual(scope['collector']['capacity_bytes'], 65536)
        self.assertEqual(scope['collector']['missing_closure_bits'], 63)
        self.assertEqual(len(scope['missing']), 6)

    def test_authored_source_pins_and_includes_are_complete(self):
        home = HOME / 'generic-cpu-observation'
        allowed = {'generic_capture.c', 'generic_capture.h', 'generic_root_access.inc',
                   'ois_cpu_diagnostic.inc', 'callback.s', 'intent.c', 'intent_bridge.s',
                   'process.h', 'lifetime.h', 'heap_owner.h', 'integration-seams.json',
                   'source-pins.json', 'scope.json', 'README.md'}
        self.assertEqual({p.name for p in home.iterdir()}, allowed)
        pins = json.loads((home / 'source-pins.json').read_text())['files']
        self.assertEqual({p['path'] for p in pins}, allowed - {'source-pins.json', 'scope.json', 'README.md'})
        for row in pins:
            self.assertEqual(Path(row['path']).name, row['path'])
            raw = (home / row['path']).read_bytes().replace(b'\r\n', b'\n')
            with self.subTest(path=row['path']):
                self.assertNotIn(b'\0', raw)
                self.assertEqual(len(raw), row['bytes'])
                self.assertEqual(hashlib.sha256(raw).hexdigest(), row['sha256'])
        for name in ('generic_capture.h', 'process.h', 'lifetime.h', 'heap_owner.h'):
            self.assertIn('#include "' + name + '"', (home / 'generic_capture.c').read_text())

    def test_integration_is_relative_and_keeps_two_runtime_seams(self):
        plan = json.loads((HOME / 'generic-cpu-observation/integration-seams.json').read_text())
        self.assertFalse(plan['complete_current_runtime_reproducible'])
        self.assertEqual(len(plan['runtime']['hunks']), 2)
        self.assertEqual({row['path'] for row in plan['files']},
                         {'save_origin/root_access.c', 'save_origin/render_capture_access.inc',
                          'save_origin/persistent_access.inc', 'save_origin/death_read_access.inc',
                          'save_origin/death_persistent_access.inc', 'particle_guard.c', 'owned_images.c'})
        for row in plan['files']:
            self.assertFalse(Path(row['path']).is_absolute())
            self.assertNotIn('..', Path(row['path']).parts)
            self.assertRegex(row['baseline_sha256'], r'^[0-9a-f]{64}$')
            self.assertRegex(row['selected_sha256'], r'^[0-9a-f]{64}$')
            for hunk in row['hunks']:
                self.assertNotEqual(hunk['before'], hunk['after'])

    def test_mss_current_visit_sources_are_joined_without_wrapper_change(self):
        scope = json.loads((HOME / 'mss-source-scope.json').read_text())
        self.assertEqual(scope['full_reproducible_export'], 'v126')
        self.assertFalse(scope['complete_current_runtime_reproducible'])
        self.assertEqual({row['path'] for row in scope['source_files']},
                         {'mss_order.c', 'mss_pause_frames.c', 'mss_visit_join.c', 'mss_visit_join.h'})
        for row in scope['source_files']:
            raw = (HOME / row['path']).read_bytes().replace(b'\r\n', b'\n')
            self.assertEqual(len(raw), row['bytes'])
            self.assertEqual(hashlib.sha256(raw).hexdigest(), row['sha256'])
        for name in ('mss_order.c', 'mss_pause_frames.c', 'mss_visit_join.c'):
            self.assertIn('#include "mss_visit_join.h"', (HOME / name).read_text())
        self.assertIn('Seven-frame stick eligibility window', scope['not_proven'])
        self.assertEqual(hashlib.sha256((HOME / 'mss_order_wrapper.s').read_bytes().replace(b'\r\n', b'\n')).hexdigest(),
                         '52b666c319da5c4fd7594eefe4a54a66b9d567b44b5c252eed6af0d1354851fe')


if __name__ == '__main__':
    unittest.main()
