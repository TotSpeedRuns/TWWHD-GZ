#!/usr/bin/env python3
"""Compile the authored JWA cache-initialization provider; no linking or runtime."""
import argparse
import hashlib
import json
from pathlib import Path
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
EXPECTED_OBJECT_SHA256 = '358863917014a4acc916f44f3afafb93a2fb03c3d76cce0f8b936a86f96b7d19'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', required=True, type=Path,
                        help='Explicit devkitPPC powerpc-eabi-gcc executable')
    parser.add_argument('--wut-include', required=True, type=Path,
                        help='Explicit genuine WUT include directory')
    parser.add_argument('--output', required=True, type=Path,
                        help='New output directory; must not already exist')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    source = root / 'experiments/maintained-components/jwa-native-cache-initialization/provider-source'
    compiler = args.compiler.resolve()
    wut = args.wut_include.resolve()
    output = args.output.resolve()
    if not compiler.is_file() or not (source / 'provider.c').is_file():
        parser.error('Compiler or authored provider.c is missing')
    for header in ('coreinit/interrupts.h', 'coreinit/thread.h', 'coreinit/mutex.h'):
        if not (wut / header).is_file():
            parser.error('Missing WUT header: ' + header)
    if output.exists():
        parser.error('Use a new controlled output directory')
    output.mkdir(parents=True)
    obj = output / 'native-cache-init-provider.o'
    command = [str(compiler), *FLAGS, '-I', str(source), '-I', str(wut),
               '-c', str(source / 'provider.c'), '-o', str(obj)]
    options = {'creationflags': subprocess.CREATE_NO_WINDOW} if hasattr(subprocess, 'CREATE_NO_WINDOW') else {}
    result = subprocess.run(command, capture_output=True, **options)
    (output / 'compile.log').write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise SystemExit('Static compilation failed; see compile.log')
    actual = digest(obj)
    report = dict(schema='portable-JWA-Native-cache-initialization-static-build/1',
                  command=command, compiler_sha256=digest(compiler),
                  object_bytes=obj.stat().st_size, object_sha256=actual,
                  qualified_object_sha256=EXPECTED_OBJECT_SHA256,
                  qualified_object_reproduced=(actual == EXPECTED_OBJECT_SHA256),
                  source_files=[dict(name=p.name, bytes=p.stat().st_size, sha256=digest(p))
                                for p in sorted(source.iterdir()) if p.is_file()],
                  static_compile_only=True, linked=False, target_executed=False)
    (output / 'build-report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({k: report[k] for k in ('object_bytes', 'object_sha256', 'qualified_object_reproduced')}))
    if actual != EXPECTED_OBJECT_SHA256:
        raise SystemExit('Compiler/codegen differs from qualified R49 object; no compatibility is inferred')


if __name__ == '__main__':
    main()
