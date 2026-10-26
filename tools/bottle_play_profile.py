"""Extract one private compatibility profile from the exact owned EUR v0 RPX.

The generated C initializer stays in ignored local-assets/. No game bytes are
embedded here, downloaded, installed or added to the source publication.
"""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'patcher'))
from native_rpx import Elf


def declaration(data):
    if len(data) != 88:
        raise ValueError('The GetPlay compatibility profile must be exactly 88 bytes')
    words = struct.unpack('>22I', data)
    lines = ['static const U btl_play_profile[22] = {']
    for index in range(0, len(words), 4):
        suffix = ',' if index + 4 < len(words) else ''
        lines.append('    ' + ', '.join(f'0x{word:08x}u' for word in words[index:index + 4]) + suffix)
    lines.append('};')
    return '\n'.join(lines) + '\n'


def extract(source, destination, metadata):
    expected_keys = {'format', 'original_EUR_v0_sha256', 'address', 'bytes',
                     'sha256', 'local_only', 'output'}
    if set(metadata) != expected_keys or metadata['format'] != 'TWWHD-GZ-bottle-play-profile/1':
        raise ValueError('Unsupported compatibility profile metadata')
    if metadata['address'] != 0x025200d8 or metadata['bytes'] != 88 or metadata['local_only'] is not True:
        raise ValueError('Only the exact bounded GetPlay compatibility profile is supported')
    local = (ROOT / 'local-assets').resolve()
    output = Path(destination).resolve()
    if not local.is_relative_to(ROOT.resolve()) or not output.is_relative_to(local) or output == local:
        raise ValueError('Generated retail bytes must remain under ignored local-assets/')
    raw = Path(source).read_bytes()
    if hashlib.sha256(raw).hexdigest() != metadata['original_EUR_v0_sha256']:
        raise ValueError('Use the exact owned original EUR v0 RPX identified by the metadata')
    elf = Elf(raw)
    elf.verify_crcs()
    _, section, offset = elf.section_at(metadata['address'], metadata['bytes'])
    data = section.data[offset:offset + metadata['bytes']]
    if len(data) != 88 or hashlib.sha256(data).hexdigest() != metadata['sha256']:
        raise ValueError('Owned input does not match the reviewed GetPlay profile')
    text = '/* Generated local-only retail profile. Do not publish this file. */\n' + declaration(data)
    if output.exists():
        if not output.is_file() or output.read_text() != text:
            raise ValueError('Preserve the existing different local profile; do not overwrite it')
        return output
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(text, newline='\n')
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--retail-rpx', required=True, type=Path)
    parser.add_argument('--metadata', type=Path,
                        default=ROOT / 'experiments/maintained-components/bottle_play_profile.json')
    parser.add_argument('--destination', type=Path,
                        default=ROOT / 'local-assets/bottle_play_profile.inc')
    args = parser.parse_args()
    metadata = json.loads(args.metadata.read_text())
    output = extract(args.retail_rpx, args.destination, metadata)
    print('Prepared local-only bottle compatibility profile:', output)


if __name__ == '__main__':
    main()
