"""Portable source/pin/refusal tests; these do not execute game code."""
from pathlib import Path
import importlib.util
import unittest

HERE = Path(__file__).resolve().parent
module_spec = importlib.util.spec_from_file_location('delivery_source', HERE / 'apply_source.py')
source = importlib.util.module_from_spec(module_spec)
module_spec.loader.exec_module(source)


class DeliverySourceTests(unittest.TestCase):
    def test_exact_authored_pins(self):
        self.assertEqual(len(source.specification()['source_pins']), 3)

    def test_four_readable_authored_seams(self):
        seams = source.specification()['seams']
        self.assertEqual(len({row['name'] for row in seams}), 4)
        self.assertIn('Left/Right: -/+1', seams[-1]['after'])

    def test_small_source_fixture_reverses(self):
        seams = source.specification()['seams']
        before = '\n'.join(row['before'] for row in seams)
        bridge = source.canonical((HERE / 'delivery_bridge.s').read_bytes())
        after = source.replace_seams(before, seams, bridge)
        reverse = after[:-len('\n' + bridge)]
        for row in reversed(seams):
            self.assertEqual(reverse.count(row['after']), 1)
            reverse = reverse.replace(row['after'], row['before'], 1)
        self.assertEqual(reverse, before)

    def test_missing_seam_refused(self):
        with self.assertRaisesRegex(ValueError, 'Missing or ambiguous'):
            source.replace_seams('', source.specification()['seams'], 'bridge')

    def test_duplicate_seam_refused(self):
        seams = source.specification()['seams']
        with self.assertRaisesRegex(ValueError, 'Missing or ambiguous'):
            source.replace_seams(seams[0]['before'] * 2, seams, 'bridge')

    def test_already_applied_bridge_refused(self):
        with self.assertRaisesRegex(ValueError, 'already present'):
            source.replace_seams('_gz_del_apply_bridge:\n', [], 'bridge')

    def test_wrong_parent_refused(self):
        with self.assertRaisesRegex(ValueError, 'Exact reviewed parent'):
            source.transform('unrelated authored source\n')

    def test_binary_source_refused(self):
        with self.assertRaisesRegex(ValueError, 'Binary'):
            source.canonical(b'source\0bytes')


if __name__ == '__main__':
    unittest.main()
