"""Apply only the reviewed authored seams to an exact private R757 source.

This transforms source text. It does not patch an RPX, compile a runtime,
access a console/process, or supply the private baseline/build dependencies.
"""
from pathlib import Path
import argparse
import hashlib
import json

HERE = Path(__file__).resolve().parent


def digest(data):
    return dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


def transform(source, leaf, seams):
    """Perform the bounded authored text edits and prove their full reversal."""
    marker = b"_gz_practice_gear_travel"
    if marker in source or leaf.count(marker + b":") != 1:
        raise ValueError("Expected an unmodified baseline and one authored leaf.")
    current = source
    for seam in seams["travel_calls"]:
        before, after = seam["before"].encode(), seam["after"].encode()
        if current.count(before) != 1 or after in current:
            raise ValueError("Travel call seam is missing, duplicated or already applied.")
        current = current.replace(before, after, 1)
    current += b"\n" + leaf
    before = seams["ganondorf_aliases"]["before"].encode()
    after = seams["ganondorf_aliases"]["after"].encode()
    if current.count(before) != 3 or after in current:
        raise ValueError("Expected the three exact original Ganondorf alias lines.")
    current = current.replace(before, after)
    restored = current.replace(after, before)
    if not restored.endswith(b"\n" + leaf):
        raise ValueError("Authored leaf suffix changed.")
    restored = restored[:-len(b"\n" + leaf)]
    for seam in reversed(seams["travel_calls"]):
        before, after = seam["before"].encode(), seam["after"].encode()
        if restored.count(after) != 1:
            raise ValueError("Travel call reversal is ambiguous.")
        restored = restored.replace(after, before, 1)
    if restored != source:
        raise ValueError("Whole-source reversal failed.")
    return current


def compose(source, edition, directory=HERE):
    pins = json.loads((directory / "source-pins.json").read_text())
    if edition not in pins["editions"]:
        raise ValueError("Edition must be personal or public.")
    expected = pins["editions"][edition]
    if digest(source) != expected["required_R757_assembly"]:
        raise ValueError("Input is not the exact qualified R757 source for this edition.")
    leaf = (directory / "gear_travel.s").read_bytes()
    if digest(leaf) != pins["authored_leaf"]:
        raise ValueError("Authored leaf pin changed.")
    seams = json.loads((directory / "seams.json").read_text())
    if digest((directory / "seams.json").read_bytes()) != pins["seams"]:
        raise ValueError("Application seam pin changed.")
    result = transform(source, leaf, seams)
    if digest(result) != expected["expected_composed_assembly"]:
        raise ValueError("Output differs from the qualified composition.")
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--edition", choices=("personal", "public"), required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists() or args.source.resolve() == args.output.resolve():
        raise ValueError("Output must be a new file, separate from the input.")
    result = compose(args.source.read_bytes(), args.edition)
    # Validate everything before creating any output. 'xb' also refuses a
    # concurrent creator; no original input is opened for writing.
    with args.output.open("xb") as stream:
        stream.write(result)
    print(json.dumps(dict(edition=args.edition, source_only=True, compiled=False,
                          native_executed=False, output=digest(result))))


if __name__ == "__main__":
    main()
