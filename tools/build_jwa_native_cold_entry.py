#!/usr/bin/env python3
"""Validate an authored baseline, overlay two files, and statically compile."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

FLAGS = [
    '-mcpu=750', '-O2', '-ffreestanding', '-fno-builtin', '-msdata=none',
    '-fno-common', '-fno-section-anchors', '-fno-stack-protector',
    '-fno-asynchronous-unwind-tables', '-fno-tree-vectorize',
    '-fno-tree-loop-distribute-patterns', '-msoft-float', '-mregnames',
    '-Wall', '-Wextra', '-Werror', '-Wno-misleading-indentation', '-fstack-usage',
    '-fno-jump-tables', '-fno-tree-switch-conversion', '-Wno-unused-function',
    '-Wno-unused-variable', '-Wno-misleading-indentation',
    '-ffunction-sections', '-fdata-sections',
]
EXPECTED = '56c1624e2a091a4b237f8708190ba4ffdbb590e63d0a1572e88f93d4734fdb59'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def checked_file(path, row):
    raw = path.read_bytes()
    if len(raw) != row['bytes'] or hashlib.sha256(raw).hexdigest() != row['sha256']:
        raise SystemExit('Authored source differs: ' + str(path))
    if raw.decode('utf-8', errors='strict').encode('utf-8') != raw or b'\x00' in raw:
        raise SystemExit('Expected complete UTF-8 source: ' + str(path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', required=True, type=Path,
                        help='Explicit devkitPPC powerpc-eabi-gcc executable')
    parser.add_argument('--wut-include', required=True, type=Path,
                        help='Explicit genuine WUT include directory')
    parser.add_argument('--output', required=True, type=Path,
                        help='New output directory; must not exist')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    area = root / 'experiments/maintained-components/jwa-native-cold-entry'
    baseline = area.parent / 'jwa-native-cache-initialization/provider-source'
    manifest = json.loads((area / 'baseline-files.json').read_text(encoding='utf-8'))
    rows = manifest['baseline_files']
    if len(rows) != 23 or {r['name'] for r in rows} != {p.name for p in baseline.iterdir() if p.is_file()}:
        parser.error('Exact complete 23-file published baseline required')
    for row in rows:
        if Path(row['name']).name != row['name']:
            parser.error('Baseline entries must be plain source filenames')
        checked_file(baseline / row['name'], row)
    overlays = manifest['overlays']
    if {r['name'] for r in overlays} != {'frame_init.h', 'frame_init.inc'} or len(overlays) != 2:
        parser.error('Only the two qualified authored overlays are permitted')
    for row in overlays:
        checked_file(area / 'source' / row['name'], row)
    compiler, wut, output = args.compiler.resolve(), args.wut_include.resolve(), args.output.resolve()
    if not compiler.is_file():
        parser.error('Compiler is missing')
    for header in ('coreinit/interrupts.h', 'coreinit/thread.h', 'coreinit/mutex.h'):
        if not (wut / header).is_file():
            parser.error('Missing genuine WUT header: ' + header)
    if output.exists():
        parser.error('Use a new controlled output directory')
    output.mkdir(parents=True)
    stage = output / 'provider-source'
    stage.mkdir()
    for row in rows:
        shutil.copyfile(baseline / row['name'], stage / row['name'])
    for row in overlays:
        shutil.copyfile(area / 'source' / row['name'], stage / row['name'])
    obj = output / 'native-cold-entry-provider.o'
    command = [str(compiler), *FLAGS, '-I', str(stage), '-I', str(wut),
               '-c', str(stage / 'provider.c'), '-o', str(obj)]
    options = {'creationflags': subprocess.CREATE_NO_WINDOW} if hasattr(subprocess, 'CREATE_NO_WINDOW') else {}
    result = subprocess.run(command, capture_output=True, **options)
    (output / 'compile.log').write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise SystemExit('Static compilation failed; see compile.log')
    actual = digest(obj)
    report = dict(schema='portable-baseline-derived-JWA-cold-entry-static-build/1',
                  command=command, compiler_sha256=digest(compiler),
                  validated_baseline_files=23, overlays=['frame_init.h', 'frame_init.inc'],
                  staged_source_files=[dict(name=p.name, bytes=p.stat().st_size, sha256=digest(p))
                                       for p in sorted(stage.iterdir()) if p.is_file()],
                  object_bytes=obj.stat().st_size, object_sha256=actual,
                  qualified_object_sha256=EXPECTED, qualified_object_reproduced=(actual == EXPECTED),
                  static_compile_only=True, linked=False, target_or_SDK_executed=False)
    (output / 'build-report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({k: report[k] for k in ('object_bytes', 'object_sha256', 'qualified_object_reproduced')}))
    if actual != EXPECTED:
        raise SystemExit('Code generation differs from the qualified R49 object; no compatibility is inferred')


if __name__ == '__main__':
    main()
