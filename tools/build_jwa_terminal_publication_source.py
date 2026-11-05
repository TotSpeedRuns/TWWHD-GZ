"""Build the reviewed JWA source component using explicit devkitPPC and WUT.

Only the owned provider TU is compiled. No game, RPX, profile, recovery folder,
prebuilt object, linker, SDK implementation or activation caller is consumed.
"""
from pathlib import Path, PurePosixPath
import argparse
import hashlib
import json
import os
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
REL = 'experiments/maintained-components/jwa-terminal-publication'
COMPONENT = ROOT/REL


def need(value, message):
    if not value:
        raise ValueError(message)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def contained_source(relative):
    need(isinstance(relative, str), 'source path must be text')
    rel = PurePosixPath(relative)
    need('\\' not in relative and not rel.is_absolute() and rel.parts and
         not any(part in ('.', '..') for part in rel.parts), 'invalid source path')
    path = (ROOT/relative).resolve(strict=True)
    need(path.is_relative_to(ROOT) and path.is_file(), 'source outside repository')
    need(not any(p.is_symlink() for p in [ROOT/relative, *(ROOT/relative).parents]
                 if p.is_relative_to(ROOT)), 'source symlink refused')
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--devkitppc', type=Path, required=True,
                        help='devkitPPC root containing bin/powerpc-eabi-gcc (reviewed r49)')
    parser.add_argument('--wut', type=Path, required=True,
                        help='genuine WUT root containing include/coreinit (reviewed 1.3.0)')
    parser.add_argument('--output', type=Path, required=True,
                        help='fresh directory outside this source tree, with existing parent')
    args = parser.parse_args()
    devkit = args.devkitppc.resolve(strict=True)
    wut = args.wut.resolve(strict=True)
    compilers = [devkit/'bin'/name for name in ('powerpc-eabi-gcc.exe', 'powerpc-eabi-gcc')]
    compilers = [path for path in compilers if path.is_file()]
    need(len(compilers) == 1, 'one explicit devkitPPC compiler required')
    compiler = compilers[0]
    include = wut/'include'
    for header in ('condition.h', 'mutex.h', 'thread.h', 'interrupts.h'):
        need((include/'coreinit'/header).is_file(), 'genuine WUT coreinit headers required')
    out = args.output.resolve()
    need(not out.exists() and out.parent.is_dir() and not out.is_relative_to(ROOT),
         'fresh output outside source tree with existing parent required')
    spec = json.loads((COMPONENT/'build-spec.json').read_text())
    manifest = json.loads((COMPONENT/'source-manifest.json').read_text())
    need(spec['schema'] == 'authored-JWA-terminal-publication-build/1', 'build spec schema')
    need(manifest['schema'] == 'authored-JWA-terminal-publication-source-manifest/1', 'source manifest schema')
    rows = manifest['files']
    paths = [row['path'] for row in rows]
    need(len(paths) == len(set(paths)) and set(paths) == set(manifest['declared_paths']),
         'exact source manifest allowlist')
    source_rows = []
    for row in rows:
        path = contained_source(row['path'])
        raw = path.read_bytes()
        need(len(raw) == row['bytes'] and sha(raw) == row['sha256'], 'changed source '+row['path'])
        source_rows.append(dict(path=row['path'], bytes=len(raw), sha256=sha(raw)))
    need(Path(__file__).resolve().relative_to(ROOT).as_posix() in paths, 'builder must be manifest-covered')
    expected = {p for p in paths if p.startswith(REL+'/')} | {REL+'/source-manifest.json'}
    actual = {p.relative_to(ROOT).as_posix() for p in COMPONENT.rglob('*') if p.is_file()}
    need(actual == expected, 'unexpected component file')
    need(spec['translation_unit'] == 'source/provider.c', 'selected authored TU')
    need(spec['standard_headers'] == ['stdint.h', 'stddef.h'], 'declared standard header set')
    for path in COMPONENT.rglob('*'):
        need(not path.is_symlink() and path.suffix not in ('.gch', '.pch'), 'precompiled header/symlink refused')
    environment = {key: value for key, value in os.environ.items()
                   if key.upper() in {'SYSTEMROOT', 'WINDIR', 'COMSPEC', 'TEMP', 'TMP', 'PATHEXT', 'SYSTEMDRIVE'}}
    environment['PATH'] = str(compiler.parent)+os.pathsep+os.defpath
    environment['LC_ALL'] = environment['LANG'] = 'C'
    out.mkdir()
    object_path = out/spec['object_name']
    command = [str(compiler), *spec['C_flags'], '-I', str(COMPONENT/'source'),
               '-I', str(include), '-c', str(COMPONENT/spec['translation_unit']),
               '-o', str(object_path)]
    result = subprocess.run(command, cwd=COMPONENT, env=environment, capture_output=True, text=True)
    (out/'compile.log').write_text(result.stdout+result.stderr)
    need(result.returncode == 0, result.stdout+result.stderr)
    raw = object_path.read_bytes()
    need(raw[:7] == b'\x7fELF\x01\x02\x01', 'ELF32 big-endian object required')
    need(sha(raw) == spec['expected_object_sha256'], 'object differs from reviewed source/codegen selection')
    for row in source_rows:
        need(sha(contained_source(row['path']).read_bytes()) == row['sha256'], 'source changed during build')
    def normalized(value):
        return str(value).replace(str(ROOT), '@SOURCE@').replace(str(out), '@OUTPUT@').replace(
                str(devkit), '@DEVKITPPC@').replace(str(wut), '@WUT@')
    report = dict(schema='authored-JWA-terminal-publication-reproduced-source-component/1',
                  passed=True, component_only=True, source_files=source_rows,
                  compiler_sha256=sha(compiler.read_bytes()),
                  reference_compiler_exact=sha(compiler.read_bytes()) == spec['reference_compiler_sha256'],
                  command=[normalized(value) for value in command],
                  standard_headers=spec['standard_headers'], external_WUT_headers=spec['external_WUT_headers'],
                  object=dict(name=object_path.name, bytes=len(raw), sha256=sha(raw)),
                  exact_reviewed_object=True, selected=False, runtime_ready=False,
                  no_game_profile_recovery_prebuilt_object_or_SDK_implementation_input=True,
                  no_linker_fullSource_activation_or_console_operation=True)
    encoded = (json.dumps(report, indent=2)+'\n').encode()
    with (out/'build-report.json').open('xb') as stream:
        stream.write(encoded)
        stream.flush()
        os.fsync(stream.fileno())
    print(json.dumps(dict(passed=True, object_sha256=sha(raw), report_sha256=sha(encoded))))


if __name__ == '__main__':
    main()
