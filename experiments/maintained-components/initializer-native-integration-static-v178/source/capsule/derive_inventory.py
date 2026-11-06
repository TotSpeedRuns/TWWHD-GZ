"""Reuse the bounded static graph reader with exact current-ea1 seed edits."""
from pathlib import Path
import hashlib,json
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
parent=R/'source714-init-integration-slice-r1/inspect.py'
def pin(p):
 raw=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest())
manifest=json.loads((parent.parent/'manifest.json').read_text())
assert pin(parent)==next(v for v in manifest['files']if v['path'].endswith('/inspect.py'))
old=parent.read_text();new=old
edits=[("R/'source714-native-cache-init-join-r1/source-native-cache-init-bound.o'","R/'source714-native-cold-entry-join-r1/source-native-cold-entry-bound.o'"),('fbde0bf42feb6b7c3b2ed3a09e88cd6138a507ea219ceedf5cafdd4197879765','ea1be3dfad78db219c6fb577012ff9931fc7bb4840d371422f822d1144ef8f57'),("'_gz_jwa_init_open']","'_gz_jwa_init_open','_gz_jwa_init_cold_for_native_entry']"),('Current714-initializer-static-dependency-slice-discovery/1','Current714-ea1-initializer-static-dependency-slice-discovery/1')]
for a,b in edits:assert new.count(a)==1;new=new.replace(a,b)
output=H/'inspect.py';assert not output.exists();output.write_text(new,newline='\n')
(H/'inventory-derivation.json').write_text(json.dumps(dict(parent=pin(parent),current=pin(output),exact_replacements=edits,no_runtime_or_physical_binding=True),indent=2)+'\n')
print(json.dumps(dict(inspector=pin(output)),indent=2))
