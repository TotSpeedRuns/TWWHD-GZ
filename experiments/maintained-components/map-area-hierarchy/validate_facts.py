"""Portable source checks: resolve local labels, IDs and exact runtime seams."""
from pathlib import Path
import ast
import hashlib
import json
import re

H = Path(__file__).resolve().parent


def table(text, symbol):
    """Read one contiguous authored .long table, without evaluating code."""
    assert text.count(symbol + ':') == 1, symbol
    result = []
    for line in text.split(symbol + ':', 1)[1].splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if not line.startswith('.long '):
            break
        result.append([part.strip() for part in line[6:].split(',')])
    return result


def local_strings(text):
    result = {}
    pattern = r'(?m)^(_gz_mah_label\d+):\s*\n\s*\.string\s+("(?:[^"\\]|\\.)*")\s*$'
    for match in re.finditer(pattern, text):
        symbol, literal = match.groups()
        assert symbol not in result, symbol
        result[symbol] = ast.literal_eval(literal)
    assert set(result) == {'_gz_mah_label' + str(i) for i in range(29)}
    return result


def check_hierarchy(text, facts):
    """Compare source references and resolved local text with semantic facts."""
    roots = table(text, '_gz_mah_roots')
    folders = table(text, '_gz_mah_folders')
    rows = table(text, '_gz_mah_rows')
    strings = local_strings(text)
    assert (len(roots), len(folders), len(rows)) == (49, 14, 232)
    assert (facts['sector_count'], facts['folder_count'], facts['row_count'], facts['original_leaf_count']) == (49, 14, 232, 218)
    assert [s['sector'] for s in facts['sectors']] == list(range(1, 50))
    local_refs = []
    groups = []
    leaves = []

    def label(symbol, expected_symbol, expected_text, require_local=False):
        assert symbol == expected_symbol, ('label reference', symbol, expected_symbol)
        if require_local:
            assert symbol in strings, symbol
        if symbol in strings:
            assert strings[symbol] == expected_text, ('resolved local text', symbol, strings[symbol], expected_text)
            local_refs.append(symbol)

    def span(descriptor):
        assert len(descriptor) == 3
        first, count = map(int, descriptor[1:])
        assert 0 <= first <= len(rows) and 0 <= count <= 33 and first + count <= len(rows)
        return rows[first:first + count]

    for root, sector in zip(roots, facts['sectors']):
        label(root[0], sector['title_symbol'], sector['title'])
        assert root[0] == '_gz_map_quadrant_name' + str(sector['sector'] - 1)
        actual = span(root)
        assert len(actual) == len(sector['root_rows'])
        for row, expected in zip(actual, sector['root_rows']):
            assert len(row) == 2
            value = int(row[1], 0)
            is_folder = expected['kind'] == 'folder'
            label(row[0], expected['label_symbol'], expected['label'], is_folder)
            if is_folder:
                group = value & 0x7fffffff
                assert value & 0x80000000 and group == expected['folder'] and group < 14
                groups.append(group)
            else:
                assert not value & 0x80000000 and value == expected['warp']
                leaves.append(value)
        for folder in sector['folders']:
            descriptor = folders[folder['id']]
            label(descriptor[0], folder['title_symbol'], folder['title'], True)
            actual = span(descriptor)
            assert len(actual) == len(folder['rows'])
            for row, expected in zip(actual, folder['rows']):
                label(row[0], expected['label_symbol'], expected['label'])
                value = int(row[1], 0)
                assert not value & 0x80000000 and value == expected['warp']
                leaves.append(value)
    assert sorted(groups) == list(range(14))
    assert len(leaves) == len(set(leaves)) == 218
    assert sorted(local_refs) == sorted(strings), 'Every local label must be checked exactly once'
    assert len(re.findall(r'^\s*cmplwi\s+(?:9|10),\s*14\s*$', text, re.M)) == 3
    ff = facts['sectors'][0]
    assert [(r['label'], r.get('warp'), r.get('folder')) for r in ff['root_rows']] == [
        ('Exterior (FF2/FF3)', 36, None), ('FF1 >', None, 0),
        ('FF2 >', None, 12), ('FF3 >', None, 13)]
    assert {f['id']: [r['warp'] for r in f['rows']] for f in ff['folders']} == {
        0: [109, 110, 145, 196, 197, 198, 199],
        12: [111, 112, 113, 143, 213], 13: [144]}
    return dict(sectors=49, folders=14, unique_leaves=218, local_strings_checked=len(local_refs))


def main():
    scope = json.loads((H / 'scope.json').read_text())
    for name, expected in scope['exported_source_sha256'].items():
        assert hashlib.sha256((H / name).read_bytes()).hexdigest() == expected, name
    result = check_hierarchy((H / 'hierarchy.s').read_text(), json.loads((H / 'hierarchy-facts.json').read_text()))
    seams = json.loads((H / 'runtime-seams.json').read_text())
    assert seams['seam_count'] == len(seams['seams']) == 8 and seams['module_linked_separately']
    assert all(seam['before'] != seam['after'] for seam in seams['seams'])
    print('Passed:', json.dumps(result, sort_keys=True), 'and eight exact separate runtime seams.')


if __name__ == '__main__':
    main()
