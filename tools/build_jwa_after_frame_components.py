"""Build the reviewed source-only JWA after-frame components with devkitPPC/WUT.

Compiles both owned TUs and performs a relocatable partial link. No game,
profile, recovery directory, private binary, SDK implementation or Native
activation input is consumed.
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
REL = 'experiments/maintained-components/jwa-after-frame-initialization'
COMPONENT = ROOT/REL


def need(value, message):
    if not value:
        raise ValueError(message)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def source_path(relative):
    need(isinstance(relative, str), 'source path must be text')
    rel = PurePosixPath(relative)
    need('\\' not in relative and not rel.is_absolute() and rel.parts and
         not any(part in ('.', '..') for part in rel.parts), 'invalid source path')
    unresolved = ROOT/relative
    path = unresolved.resolve(strict=True)
    need(path.is_relative_to(ROOT) and path.is_file(), 'source outside source tree')
    need(not any(p.is_symlink() for p in [unresolved, *unresolved.parents]
                 if p.is_relative_to(ROOT)), 'source symlink refused')
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--devkitppc', type=Path, required=True,
                        help='devkitPPC root containing bin/powerpc-eabi-gcc and ld (reviewed r49)')
    parser.add_argument('--wut', type=Path, required=True,
                        help='genuine WUT root containing include/coreinit (reviewed 1.3.0)')
    parser.add_argument('--output', type=Path, required=True,
                        help='new directory outside this source tree, with existing parent')
    args = parser.parse_args()
    devkit, wut = args.devkitppc.resolve(strict=True), args.wut.resolve(strict=True)
    tools = {}
    for name in ('gcc', 'ld'):
        candidates = [devkit/'bin'/('powerpc-eabi-'+name+suffix) for suffix in ('.exe', '')]
        candidates = [path for path in candidates if path.is_file()]
        need(len(candidates) == 1, 'one explicit devkitPPC '+name+' required')
        tools[name] = candidates[0]
    include = wut/'include'
    for header in ('condition.h', 'mutex.h', 'thread.h', 'interrupts.h'):
        need((include/'coreinit'/header).is_file(), 'genuine WUT coreinit headers required')
    out = args.output.resolve()
    need(not out.exists() and out.parent.is_dir() and not out.is_relative_to(ROOT),
         'new output outside source tree with existing parent required')
    spec = json.loads((COMPONENT/'build-spec.json').read_text())
    manifest = json.loads((COMPONENT/'source-manifest.json').read_text())
    need(spec['schema'] == 'authored-JWA-after-frame-components-build/1', 'build spec schema')
    need(manifest['schema'] == 'authored-JWA-after-frame-source-manifest/1', 'source manifest schema')
    rows = manifest['files']
    paths = [row['path'] for row in rows]
    need(len(paths) == len(set(paths)) and set(paths) == set(manifest['declared_paths']), 'source allowlist')
    for row in rows:
        raw = source_path(row['path']).read_bytes()
        need(len(raw) == row['bytes'] and sha(raw) == row['sha256'], 'changed source '+row['path'])
    need(Path(__file__).resolve().relative_to(ROOT).as_posix() in paths, 'manifest-covered builder required')
    expected = {path for path in paths if path.startswith(REL+'/')} | {REL+'/source-manifest.json'}
    actual = {path.relative_to(ROOT).as_posix() for path in COMPONENT.rglob('*') if path.is_file()}
    need(actual == expected, 'unexpected component file')
    for path in COMPONENT.rglob('*'):
        need(not path.is_symlink() and path.suffix not in ('.gch', '.pch'), 'precompiled header/symlink refused')
    units = spec['translation_units']
    need([(r['source'], r['object']) for r in units] == [
         ('provider-source/provider.c', 'frame-init-provider.o'),
         ('jol-source/jol_context.c', 'frame-init-jol.o')], 'selected authored TUs')
    need(spec['standard_headers'] == ['stdint.h', 'stddef.h'], 'declared standard headers')
    need(spec['linker_flags'] == ['-r', '--emit-relocs'] and
         spec['combined_object'] == 'frame-init-components.o', 'relocatable link only')
    env = {key: value for key, value in os.environ.items()
           if key.upper() in {'SYSTEMROOT', 'WINDIR', 'COMSPEC', 'TEMP', 'TMP', 'PATHEXT', 'SYSTEMDRIVE'}}
    env['PATH'] = str(tools['gcc'].parent)+os.pathsep+os.defpath
    env['LC_ALL'] = env['LANG'] = 'C'
    out.mkdir()
    commands, objects = [], []
    def normalized(value):
        return str(value).replace(str(ROOT), '@SOURCE@').replace(str(out), '@OUTPUT@').replace(
                str(devkit), '@DEVKITPPC@').replace(str(wut), '@WUT@')
    def run(command, log):
        result = subprocess.run([str(value) for value in command], cwd=COMPONENT,
                                env=env, capture_output=True, text=True)
        (out/log).write_text(result.stdout+result.stderr)
        need(result.returncode == 0, result.stdout+result.stderr)
        commands.append([normalized(value) for value in command])
    for unit in units:
        source = COMPONENT/unit['source']
        run([tools['gcc'], *spec['C_flags'], '-I', source.parent, '-I', include,
             '-c', source, '-o', out/unit['object']], unit['object']+'.log')
        raw = (out/unit['object']).read_bytes()
        need(raw[:7] == b'\x7fELF\x01\x02\x01' and sha(raw) == unit['sha256'],
             'component differs from reviewed source/codegen: '+unit['object'])
        objects.append(dict(name=unit['object'], bytes=len(raw), sha256=sha(raw)))
    combined = out/spec['combined_object']
    run([tools['ld'], *spec['linker_flags'], *[out/u['object'] for u in units], '-o', combined], 'partial-link.log')
    raw = combined.read_bytes()
    need(raw[:7] == b'\x7fELF\x01\x02\x01' and sha(raw) == spec['combined_sha256'], 'partial link differs')
    objects.append(dict(name=combined.name, bytes=len(raw), sha256=sha(raw)))
    for row in rows:
        need(sha(source_path(row['path']).read_bytes()) == row['sha256'], 'source changed during build')
    report = dict(schema='authored-JWA-after-frame-reproduced-source-components/1', passed=True,
                  source_files=rows, commands=commands, objects=objects,
                  compiler_sha256=sha(tools['gcc'].read_bytes()), linker_sha256=sha(tools['ld'].read_bytes()),
                  reference_toolchain_exact=sha(tools['gcc'].read_bytes()) == spec['reference_compiler_sha256'] and
                                             sha(tools['ld'].read_bytes()) == spec['reference_linker_sha256'],
                  standard_headers=spec['standard_headers'], external_WUT_headers=spec['external_WUT_headers'],
                  exact_reviewed_objects_and_partial_link=True, component_only=True,
                  selected=False, runtime_ready=False,
                  no_game_profile_recovery_private_binary_or_SDK_implementation_input=True,
                  no_fullSource_Native_activation_or_console_operation=True)
    encoded = (json.dumps(report, indent=2)+'\n').encode()
    with (out/'build-report.json').open('xb') as stream:
        stream.write(encoded)
        stream.flush()
        os.fsync(stream.fileno())
    print(json.dumps(dict(passed=True, objects=3, combined_sha256=sha(raw), report_sha256=sha(encoded))))


if __name__ == '__main__':
    main()
