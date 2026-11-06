#!/usr/bin/env python3
"""Rebuild the standalone recovered reader ET_REL; never run its instructions."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def identity(path):
    raw = path.read_bytes()
    return {"bytes": len(raw), "sha256": hashlib.sha256(raw).hexdigest()}


def require_identity(path, expected, label):
    actual = identity(path)
    if any(actual[key] != expected[key] for key in ("bytes", "sha256")):
        raise SystemExit(label + " identity differs: " + str(path))
    return actual


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", required=True, type=Path,
                        help="Path to pinned devkitPPC r49 powerpc-eabi-gcc.exe")
    parser.add_argument("--output", required=True, type=Path,
                        help="New output .o path; existing output/receipt is refused")
    args = parser.parse_args()
    repo = Path(__file__).resolve().parent.parent
    spec_path = repo / "experiments/maintained-components/ordinary-reader-recovery/BUILD.json"
    spec = json.loads(spec_path.read_text(encoding="utf-8"))
    source, header = repo / spec["source"]["path"], repo / spec["header"]["path"]
    compiler = args.compiler.resolve(strict=True)
    output = args.output.resolve()
    receipt_path = output.with_suffix(output.suffix + ".build.json")
    if output.exists() or receipt_path.exists():
        raise SystemExit("Output or receipt already exists; select a new path")
    require_identity(source, spec["source"], "Source")
    require_identity(header, spec["header"], "Header")
    compiler_id = require_identity(compiler, spec["compiler"], "Compiler")
    output.parent.mkdir(parents=True, exist_ok=True)
    command = [str(compiler), *spec["flags"], "-c", str(source), "-o", str(output)]
    run = subprocess.run(command, cwd=repo, shell=False, capture_output=True, text=True)
    if run.stdout:
        print(run.stdout, end="")
    if run.stderr:
        print(run.stderr, end="")
    actual = identity(output) if output.exists() else None
    receipt = {
        "schema": "portable-ordinary-reader-assembly-build-receipt/1",
        "compiler": {"path": str(compiler), **compiler_id},
        "source": {"path": str(source), **identity(source)},
        "header": {"path": str(header), **identity(header)},
        "command": command,
        "returncode": run.returncode,
        "object": {"path": str(output), **actual} if actual else None,
        "object_exact": actual == spec["expected_object"],
        "assembler_compiler_invocations": 1,
        "target_or_SDK_execution": False,
        "game_or_SDK_input": False,
        "linked_or_installable": False
    }
    with receipt_path.open("x", encoding="utf-8", newline="\n") as stream:
        json.dump(receipt, stream, indent=2)
        stream.write("\n")
    if run.returncode != 0 or actual != spec["expected_object"]:
        raise SystemExit("Assembly build failed or object differs; see " + str(receipt_path))
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()
