"""Rebuild the authored 684-byte v173 release-reader component only.

No game, SDK, private object, image patch, console or network input is used.
The unresolved module interfaces require a separate parent-image proof.
"""
from pathlib import Path, PurePosixPath
import argparse
import hashlib
import json
import os
import struct
import subprocess
import sys

if not __debug__:
    raise RuntimeError('Run without optimized Python.')
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
REL = 'experiments/maintained-components/release-readers-v173'
COMP = ROOT / REL
SPEC_SHA = 'ef1759d9542adcc3fc25f4b2f764b7ea2e8ea84ff495e60e81bc897f3562e5e2'


def need(value, message):
    if not value:
        raise ValueError(message)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def source(relative):
    need(isinstance(relative, str) and '\\' not in relative, 'POSIX source path')
    q = PurePosixPath(relative)
    need(q.parts and not q.is_absolute() and not any(x in ('.', '..') for x in q.parts), 'contained source path')
    p = ROOT / relative
    need(not any(x.is_symlink() for x in [p, *p.parents] if x.is_relative_to(ROOT)), 'source symlink refused')
    p = p.resolve(strict=True)
    need(p.is_file() and p.is_relative_to(ROOT), 'source escaped tree')
    return p


def shape(raw):
    need(len(raw) >= 52 and raw[:7] == b'\x7fELF\x01\x02\x01', 'ELF32 BE required')
    h = struct.unpack_from('>16sHHIIIIIHHHHHH', raw)
    need(h[1] == 1 and h[2] == 20 and h[11] == 40 and 0 < h[12] < 1024, 'PPC ET_REL required')
    need(h[6] + h[11] * h[12] <= len(raw), 'section table bounds')
    sections = [struct.unpack_from('>10I', raw, h[6] + 40 * i) for i in range(h[12])]
    code = readonly = mutable = 0
    defined, undefined, kinds = [], [], {}
    for s in sections:
        if s[1] != 8:
            need(s[4] + s[5] <= len(raw), 'section body bounds')
        if s[2] & 2 and s[5]:
            if s[2] & 1:
                mutable += s[5]
            elif s[2] & 4:
                code += s[5]
            else:
                readonly += s[5]
        if s[1] == 4:
            need(s[9] == 12 and s[5] % 12 == 0, 'RELA shape')
            for offset in range(s[4], s[4] + s[5], 12):
                kind = struct.unpack_from('>I', raw, offset + 4)[0] & 255
                kinds[str(kind)] = kinds.get(str(kind), 0) + 1
        if s[1] != 2:
            continue
        need(s[9] == 16 and s[5] % 16 == 0 and s[6] < len(sections), 'symbol table shape')
        strings_section = sections[s[6]]
        strings = raw[strings_section[4]:strings_section[4] + strings_section[5]]
        for offset in range(s[4], s[4] + s[5], 16):
            name, value, size, info, other, index = struct.unpack_from('>IIIBBH', raw, offset)
            need(name < len(strings), 'symbol name bounds')
            end = strings.find(b'\0', name)
            need(end >= name, 'symbol name termination')
            name = strings[name:end].decode('ascii')
            if name and info >> 4 in (1, 2):
                (defined if index else undefined).append(name)
    return dict(CODE=code, readonly=readonly, mutable=mutable,
                defined=sorted(defined), undefined=sorted(undefined), relocation_kinds=kinds)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--compiler', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    cc = args.compiler.resolve(strict=True)
    ld = cc.with_name('powerpc-eabi-ld' + cc.suffix).resolve(strict=True)
    need(cc.is_file() and ld.is_file(), 'compiler/linker files required')
    out = args.output.resolve()
    need(not out.exists() and out.parent.is_dir() and not out.is_relative_to(ROOT), 'fresh output outside source tree required')
    spec_raw = source(REL + '/build-spec.json').read_bytes()
    need(sha(spec_raw) == SPEC_SHA, 'changed selected build spec')
    spec = json.loads(spec_raw)
    need(sha(cc.read_bytes()) == spec['compiler_sha256'], 'wrong reference compiler')
    need(sha(ld.read_bytes()) == spec['linker_sha256'], 'wrong reference linker')
    manifest = json.loads(source(REL + '/source-manifest.json').read_bytes())
    records = manifest['files']
    names = [r['path'] for r in records]
    need(len(names) == len(set(names)) and set(names) == set(spec['source_paths']), 'exact source manifest allowlist')
    need(Path(__file__).resolve().relative_to(ROOT).as_posix() in names, 'builder manifest coverage')
    for r in records:
        data = source(r['path']).read_bytes()
        need(len(data) == r['bytes'] and sha(data) == r['sha256'], 'changed source: ' + r['path'])
    actual = {p.relative_to(ROOT).as_posix() for p in COMP.rglob('*') if p.is_file()}
    expected = {p for p in names if p.startswith(REL + '/')} | {REL + '/source-manifest.json'}
    need(actual == expected, 'unexpected component file')
    for p in COMP.rglob('*'):
        need(not p.is_symlink() and p.suffix.lower() not in ('.gch', '.pch'), 'source symlink/precompiled file refused')
    env = {k: v for k, v in os.environ.items() if k.upper() in {'SYSTEMROOT', 'WINDIR', 'COMSPEC', 'TEMP', 'TMP', 'PATHEXT', 'SYSTEMDRIVE'}}
    env.update(PATH=str(cc.parent) + os.pathsep + os.defpath, LANG='C', LC_ALL='C')
    out.mkdir()
    commands, objects = [], []
    for name in spec['compile_order']:
        target = out / Path(name).with_suffix('.o')
        command = [str(cc), *spec['compiler_flags'], '-c', str(source(REL + '/source/' + name)), '-o', str(target)]
        result = subprocess.run(command, cwd=COMP, env=env, capture_output=True, text=True)
        need(result.returncode == 0, result.stdout + result.stderr)
        objects.append(target)
        commands.append(command)
    target = out / spec['output']
    command = [str(ld), '-r', *map(str, objects), '-o', str(target)]
    result = subprocess.run(command, cwd=COMP, env=env, capture_output=True, text=True)
    need(result.returncode == 0, result.stdout + result.stderr)
    commands.append(command)
    raw = target.read_bytes()
    need(sha(raw) == spec['component_sha256'], 'whole component object differs')
    identity = shape(raw)
    need(identity['CODE'] == 684 and identity['readonly'] == identity['mutable'] == 0, '684 CODE-only shape')
    bindings = json.loads(source(REL + '/bindings.json').read_bytes())
    need(identity['undefined'] == sorted(bindings['interfaces']), 'unresolved interface closure')
    for r in records:
        need(sha(source(r['path']).read_bytes()) == r['sha256'], 'source changed during build')
    normalized = [[x.replace(str(ROOT), '@SOURCE@').replace(str(out), '@OUTPUT@').replace(str(cc.parent), '@TOOLCHAIN_BIN@') for x in command] for command in commands]
    report = dict(schema='reproduced-authored-release-readers/1', passed=True, component_only=True,
                  component_sha256=sha(raw), component_bytes=len(raw), **identity,
                  compiler_sha256=sha(cc.read_bytes()), linker_sha256=sha(ld.read_bytes()),
                  source_files=records, commands=normalized,
                  image_or_SDK_or_private_object_input=False, runtime_activation=False)
    with (out / 'build-report.json').open('x', encoding='utf-8', newline='\n') as f:
        json.dump(report, f, indent=2)
        f.write('\n')
        f.flush()
        os.fsync(f.fileno())
    print(json.dumps(dict(passed=True, component_sha256=sha(raw), CODE=684, mutable=0)))


if __name__ == '__main__':
    main()
