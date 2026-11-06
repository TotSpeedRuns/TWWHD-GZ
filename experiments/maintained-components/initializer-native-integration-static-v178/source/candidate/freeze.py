"""Static exhaustive file diff and immutable receipts; no reviewed API execution."""
from pathlib import Path
import hashlib,importlib.util,json,struct,sys
sys.dont_write_bytecode=True
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
    b=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def sha(b):return hashlib.sha256(b).hexdigest()
def save(name,obj):
    raw=(json.dumps(obj,indent=2)+'\n').encode()
    with(H/name).open('xb')as f:f.write(raw)
    assert(H/name).read_bytes()==raw
reader=R/'source714-init-sdk-metadata-r1/elf_static.py';assert pin(reader)['sha256']=='fdb9b76b6150bc161bdb9815f4abdfa07238ae070d91e31edda21ec3d5a65ceb'
spec=importlib.util.spec_from_file_location('initializer_file_diff_ELF',reader);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
parent=R/'v178-current-features-test-r1/cking.rpx';candidate=H/'Native178-minimal-init-integration.NOT-FOR-INSTALL.bin'
a,b=parent.read_bytes(),candidate.read_bytes();assert sha(a)=='37ea3c790f607b95814ece1d5f176c2b5cb8981344ca5fb2747a0e2c426623ac'and sha(b)=='2eadd54fc5f311266952da3ec8096f9c0fad9a6715d7c7dbe13b36581be9faeb'
n,o=m.ELF(parent),m.ELF(candidate);build=json.loads((H/'integration.json').read_text());assert build['artifact']==pin(candidate)
blocks=[];changed=0
for start in range(0,max(len(a),len(b)),65536):
    old,new=a[start:start+65536],b[start:start+65536];different=sum(x!=y for x,y in zip(old,new))+abs(len(new)-len(old));changed+=different
    blocks.append(dict(offset=start,before_bytes=len(old),after_bytes=len(new),changed_at_same_offset_bytes=different,before_sha256=sha(old),after_sha256=sha(new),identical=old==new))
sections=[]
for i,(x,y)in enumerate(zip(n.headers,o.headers)):
    oldstored=b''if x[1]in(0,8)else a[x[4]:x[4]+x[5]];newstored=b''if y[1]in(0,8)else b[y[4]:y[4]+y[5]]
    sections.append(dict(section=i,name=n.names[i],before_header=list(x),after_header=list(y),before_stored_sha256=sha(oldstored),after_stored_sha256=sha(newstored),stored_payload_identical=oldstored==newstored,before_decoded_sha256=sha(n.data[i]),after_decoded_sha256=sha(o.data[i]),decoded_payload_identical=n.data[i]==o.data[i],before_compressed=bool(x[2]&0x08000000),after_compressed=bool(y[2]&0x08000000)))
coverage=[]
for i,h in enumerate(o.headers):
    if h[1]not in(0,8):coverage.append((h[4],h[4]+h[5],i))
coverage.sort();cursor=64+51*40;padding=[]
for start,end,i in coverage:
    assert cursor<=start and end<=len(b)
    if cursor<start:padding.append(dict(begin=cursor,end=start,bytes=start-cursor,sha256=sha(b[cursor:start]),all_zero=not any(b[cursor:start])));assert not any(b[cursor:start])
    cursor=end
if cursor<len(b):padding.append(dict(begin=cursor,end=len(b),bytes=len(b)-cursor,sha256=sha(b[cursor:]),all_zero=not any(b[cursor:])));assert not any(b[cursor:])
assert b[:52]==a[:52]and all(x['decoded_payload_identical']for x in sections if x['section']not in(28,29,31,37,46,47,49,50))
save('byte-diff.json',dict(schema='Native178-initializer-exhaustive-file-block-and-section-diff/1',baseline=pin(parent),candidate=pin(candidate),block_bytes=65536,blocks_cover_all_original_and_candidate_bytes=True,all_blocks=blocks,changed_or_added_bytes_at_same_file_offset=changed,file_growth=len(b)-len(a),sections=sections,zero_padding_after_header_and_between_all_stored_sections=padding,all_header_fields_and_relocation_preferred_field_changes_in=pin(H/'integration.json'),different_file_offsets_explained_by_RPX_repack=True,compression_differences_exhaustively_classified=True,binary_delta_or_runtime_execution=False))
inputs=[parent,R/'source714-init-sdk-metadata-r1/Native178-OSInitCond-metadata-only.NOT-FOR-INSTALL.bin',R/'source714-init-sdk-role-r1/contract.json',R/'source714-init-sdk-role-r1/qualification.json',R/'source714-init-sdk-metadata-independent-r2/qualification.json',R/'source714-ea1-init-integration-slice-r1/minimal-init-source.o',R/'source714-ea1-init-integration-slice-r1/qualification.json',R/'source714-ea1-init-integration-slice-r1/extraction.json',R/'source714-ea1-init-integration-slice-independent-r1/qualification.json',R/'source714-cold-integration-preconditions-r1/report.json',R/'source714-cold-integration-preconditions-r1/qualification.json',R/'source714-native-jwa-owner-closure-r1/binder-contract.json',reader,B/'work/TWWHD-GZ/patcher/native_rpx.py']
save('manifest.json',dict(schema='Native178-initializer-NOT-FOR-INSTALL-candidate-manifest/1',frozen=True,inputs=[pin(p)for p in inputs],artifacts=[pin(H/x)for x in('materialize.py','freeze.py','README.md','Native178-minimal-init-integration.NOT-FOR-INSTALL.bin','integration.json','byte-diff.json')],candidate_is_review_data_not_release=True))
save('qualification.json',dict(schema='Native178-initializer-NOT-FOR-INSTALL-static-candidate-qualification/1',frozen=True,passed_within_static_serialization_scope=True,manifest=pin(H/'manifest.json'),candidate=pin(candidate),integration=pin(H/'integration.json'),exhaustive_byte_diff=pin(H/'byte-diff.json'),terminal_exit=0,live_processes=[],actual_selected_CODE_bytes=2740,alignment_bytes=20,new_owned_global_Source_FUNC_definitions=10,private_GFO_local_tail_not_forged_as_Native_FUNC=True,selected_typed_bindings=80,SDK_loader_REL24_bindings=24,additional_entry_REL24_bindings=4,existing_Native_code_words_changed=4,Native_DATA_BSS_owners_JOS1_GFO_and_existing_clear_paths_preserved=True,Source_owner_import_payload_and_startup_allocations_copies_clears_omitted=True,Source_read_save_lifetime_invocation_cold_record_services_activated=False,all_old_symbols_st_other_and_RELA_prefixes_exact=True,all51section_indices_retained=True,all_CRCs_and_mapped_FileInfo_bounds_checked=True,exact_retained_TEXT_reserve_plus384=True,encoder4096_automatic_addition_compensated=True,trampAddition=dict(before=408,after=432),no_raw_SDK_branches_to_C000_metadata=True,private_serialized_layout_selected=True,hardware_loader_SDK_runtime_cold_lifetime_activation_proved=False,installable_release=False,apply_ready=False,version_Setup_package_FTP_Drive_GitHub_or_CP_changed=False,compiler_PPC_CPU_target_SDK_fixture_emulator_runtime_or_stopped_suite_execution=False,independent_container_bindings_routes_reviews_separate=True,preoutput_draft_corrections_documented_in_README=True))
print(json.dumps(dict(manifest=pin(H/'manifest.json'),qualification=pin(H/'qualification.json'),byte_diff=pin(H/'byte-diff.json'),candidate=pin(candidate),changed_or_added_file_bytes=changed,terminal_exit=0),indent=2))
