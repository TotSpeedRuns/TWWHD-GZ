"""Portable source-text checks, not native API or gameplay tests."""
from pathlib import Path
import importlib.util
import json
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location("gear_source_applier", HERE / "apply.py")
APPLY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(APPLY)


class SourceSeamTests(unittest.TestCase):
    def setUp(self):
        self.seams = json.loads((HERE / "seams.json").read_text())
        self.leaf = (HERE / "gear_travel.s").read_bytes()
        self.source = b"fixture prefix\n" + b"middle\n".join(
            seam["before"].encode() for seam in self.seams["travel_calls"])
        self.source += self.seams["ganondorf_aliases"]["before"].encode() * 3 + b"\nfixture suffix\n"

    def test_exact_edit_counts_and_whole_reversal(self):
        result = APPLY.transform(self.source, self.leaf, self.seams)
        self.assertEqual(result.count(b"bl _gz_practice_gear_travel\n"), 2)
        self.assertEqual(result.count(self.seams["ganondorf_aliases"]["after"].encode()), 3)
        self.assertTrue(result.endswith(b"\n" + self.leaf))
        restored = result.replace(self.seams["ganondorf_aliases"]["after"].encode(),
                                  self.seams["ganondorf_aliases"]["before"].encode())
        restored = restored[:-len(b"\n" + self.leaf)]
        for seam in reversed(self.seams["travel_calls"]):
            restored = restored.replace(seam["after"].encode(), seam["before"].encode())
        self.assertEqual(restored, self.source)

    def test_missing_or_duplicate_call_refuses(self):
        for seam in self.seams["travel_calls"]:
            before = seam["before"].encode()
            for source in (self.source.replace(before, b""), self.source + before):
                with self.assertRaises(ValueError):
                    APPLY.transform(source, self.leaf, self.seams)

    def test_missing_duplicate_or_already_changed_alias_refuses(self):
        before = self.seams["ganondorf_aliases"]["before"].encode()
        after = self.seams["ganondorf_aliases"]["after"].encode()
        for source in (self.source.replace(before, b"", 1), self.source + before,
                       self.source.replace(before, after, 1)):
            with self.assertRaises(ValueError):
                APPLY.transform(source, self.leaf, self.seams)

    def test_already_applied_refuses(self):
        result = APPLY.transform(self.source, self.leaf, self.seams)
        with self.assertRaises(ValueError):
            APPLY.transform(result, self.leaf, self.seams)

    def test_wrong_baseline_and_edition_refuse(self):
        for edition in ("personal", "public", "other"):
            with self.assertRaises(ValueError):
                APPLY.compose(self.source, edition)

    def test_cli_refusals_preserve_input_and_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            home = Path(temporary)
            source, output = home / "source.s", home / "output.s"
            source.write_bytes(self.source)
            with self.assertRaises(ValueError):
                APPLY.main(["--edition", "personal", "--source", str(source), "--output", str(output)])
            self.assertFalse(output.exists())
            output.write_bytes(b"existing output")
            with self.assertRaises(ValueError):
                APPLY.main(["--edition", "personal", "--source", str(source), "--output", str(output)])
            self.assertEqual(output.read_bytes(), b"existing output")
            with self.assertRaises(ValueError):
                APPLY.main(["--edition", "personal", "--source", str(source), "--output", str(source)])
            self.assertEqual(source.read_bytes(), self.source)


if __name__ == "__main__":
    unittest.main()
