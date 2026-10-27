"""Apply the four authored Delivery seams to an exact separately owned source.

This writes new assembly text only. It is not a current full-release builder,
native patcher, runtime callback or source of game assets.
"""
from pathlib import Path
import argparse
import hashlib
import json

HERE = Path(__file__).resolve().parent


def canonical(data):
    if b'\0' in data:
        raise ValueError('Binary source refused')
    return data.decode('utf-8').replace('\r\n', '\n')


def digest(text):
    return hashlib.sha256(text.encode('utf-8')).hexdigest()


def specification():
    spec = json.loads((HERE / 'integration-seams.json').read_text())
    if spec['schema'] != 'authored-Delivery-source-seams/1' or len(spec['seams']) != 4:
        raise ValueError('Unrecognized seam specification')
    for row in spec['source_pins']:
        if row['name'] not in {'delivery.c', 'delivery.h', 'delivery_bridge.s'}:
            raise ValueError('Unexpected authored source')
        text = canonical((HERE / row['name']).read_bytes())
        if digest(text) != row['published_LF']['sha256']:
            raise ValueError('Authored source changed: ' + row['name'])
    return spec


def replace_seams(text, seams, bridge):
    """Pure source-text operation; no native outcomes or runtime authority."""
    if '_gz_del_apply_bridge:\n' in text:
        raise ValueError('Delivery bridge already present')
    result = text
    for seam in seams:
        if not seam['before'] or result.count(seam['before']) != 1:
            raise ValueError('Missing or ambiguous seam: ' + seam['name'])
        result = result.replace(seam['before'], seam['after'], 1)
    return result + '\n' + bridge


def transform(text):
    spec = specification()
    text = text.replace('\r\n', '\n')
    if digest(text) != spec['parent_assembly']['canonical_LF_sha256']:
        raise ValueError('Exact reviewed parent assembly required')
    bridge = canonical((HERE / 'delivery_bridge.s').read_bytes())
    result = replace_seams(text, spec['seams'], bridge)
    if digest(result) != spec['result_assembly']['canonical_LF_sha256']:
        raise ValueError('Result differs from reviewed source')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or args.input.resolve() == args.output.resolve():
        raise ValueError('Use a new output path; inputs are never overwritten')
    result = transform(canonical(args.input.read_bytes()))
    # All hashes and seam counts are checked before creating the output.
    with args.output.open('x', encoding='utf-8', newline='\n') as stream:
        stream.write(result)


if __name__ == '__main__':
    main()
