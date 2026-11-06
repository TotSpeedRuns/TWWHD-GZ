"""Freeze already terminal static output; does not replay the serializer."""
from pathlib import Path
import hashlib
import json

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sha = lambda b: hashlib.sha256(b).hexdigest()

def need(value,message):
    if not value:
        raise ValueError(message)

def pin(p):
    data = p.read_bytes()
    return dict(path=p.relative_to(ROOT).as_posix(),bytes=len(data),sha256=sha(data))

def save(name,data):
    with (HERE/name).open('x',encoding='utf-8',newline='\n') as f:
        json.dump(data,f,indent=2)
        f.write('\n')
    return pin(HERE/name)

artifact = HERE/'Native178-minimal-init-OSWaitCond-metadata.NOT-FOR-INSTALL.bin'
need(pin(artifact)['sha256'] == '4039d880c4f0f6fa83f1bef3aa54e64e3fb27bd93c15a3ba135fada3a9917fee',
     'terminal saved artifact')
need(pin(HERE/'diff.json')['sha256'] == '3bb1a8b609e7e5bb05abf9c2adc38f9291328bd81b8d0f4f234889abf14e3028',
     'terminal complete diff')
diff = json.loads((HERE/'diff.json').read_text())
for item in diff['inputs']:
    need(pin(ROOT/item['path']) == item,'all frozen input pins unchanged')
need(diff['changed_existing_bytes'] == 28 and diff['actual_archive_length_after'] == 30184960 and
     diff['FileInfo']['loadSize_after'] == 0x1e994,'actual bounded serialization result')
manifest = save('manifest.json',dict(schema='OSWaitCond-metadata-successor-manifest/1',
    files=[pin(HERE/name) for name in ['materialize.py','elf_static.py','freeze.py','README.md',
                                      'diff.json',artifact.name]], inputs=diff['inputs']))
q = save('qualification.json',dict(schema='OSWaitCond-metadata-successor-static-qualification/1',
    frozen=True,passed_within_static_serialization_scope=True,manifest=manifest,
    artifact=pin(artifact),diff=pin(HERE/'diff.json'),terminal_exit=0,live_processes=[],
    new_named_FUNC18_import=1,old_symbol_count=5025,new_symbol_count=5026,
    existing_changed_bytes=28,appended_file_bytes=10048,changed_section_header_fields=4,
    string_payload_moved_explicitly=True,all_old_CODE_DATA_import_payloads_symbols_and_RELA_preserved=True,
    FileInfo_minimal_loadSize_increase=16,FileInfo_pad=136,
    every_other_FileInfo_byte_and_trampoline_budget_preserved=True,
    no_encoder4096_or_TEXT_reserve_change=True,all_CRCs_and_global_mapped_bounds_passed=True,
    parse_roundtrips=2,new_call_rows=0,new_entry_words=0,installable_release=False,
    SDK_module_load_target_runtime_compiler_emulator_or_stopped_suite_executed=False,
    Source_repo_console_network_CP_Setup_or_version_changed=False,
    independent_container_peer_separate=True))
print(json.dumps(dict(qualification=q,manifest=manifest,artifact=pin(artifact),diff=pin(HERE/'diff.json'))))
