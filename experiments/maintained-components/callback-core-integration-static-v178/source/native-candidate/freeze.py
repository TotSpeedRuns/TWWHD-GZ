"""Exhaustive static file/section diff and immutable candidate receipts."""
from pathlib import Path
import hashlib,importlib.util,json,sys
sys.dont_write_bytecode=True
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
 raw=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest())
def sha(raw):return hashlib.sha256(raw).hexdigest()
def save(name,obj):
 raw=(json.dumps(obj,indent=2)+'\n').encode()
 with(H/name).open('xb')as f:f.write(raw)
 assert(H/name).read_bytes()==raw
reader=R/'source714-init-sdk-metadata-r1/elf_static.py';assert pin(reader)['sha256']=='fdb9b76b6150bc161bdb9815f4abdfa07238ae070d91e31edda21ec3d5a65ceb'
spec=importlib.util.spec_from_file_location('context_candidate_file_ELF',reader);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
parent=R/'source714-wait-sdk-metadata-r1/Native178-minimal-init-OSWaitCond-metadata.NOT-FOR-INSTALL.bin';init=R/'source714-minimal-native-init-candidate-r1/Native178-minimal-init-integration.NOT-FOR-INSTALL.bin';candidate=H/'Native178-initializer-and-callback-core.NOT-FOR-INSTALL.bin'
a,b=parent.read_bytes(),candidate.read_bytes();assert sha(a)=='4039d880c4f0f6fa83f1bef3aa54e64e3fb27bd93c15a3ba135fada3a9917fee'and sha(b)=='6c090554eccce09ba1d2e7068010bf1791de2b6c224b721c1ff017c9975b6b22'
n,o,i=m.ELF(parent),m.ELF(candidate),m.ELF(init);build=json.loads((H/'integration.json').read_text());assert build['artifact']==pin(candidate)
blocks=[];changes=0
for start in range(0,max(len(a),len(b)),65536):
 old,new=a[start:start+65536],b[start:start+65536];count=sum(x!=y for x,y in zip(old,new))+abs(len(old)-len(new));changes+=count
 blocks.append(dict(offset=start,before_bytes=len(old),after_bytes=len(new),changed_at_same_offset_bytes=count,before_sha256=sha(old),after_sha256=sha(new),identical=old==new))
sections=[]
for sec,(old,new)in enumerate(zip(n.headers,o.headers)):
 oldraw=b''if old[1]in(0,8)else a[old[4]:old[4]+old[5]];newraw=b''if new[1]in(0,8)else b[new[4]:new[4]+new[5]]
 sections.append(dict(section=sec,name=n.names[sec],before_header=list(old),after_header=list(new),before_stored_sha256=sha(oldraw),after_stored_sha256=sha(newraw),stored_payload_identical=oldraw==newraw,before_decoded_sha256=sha(n.data[sec]),after_decoded_sha256=sha(o.data[sec]),decoded_payload_identical=n.data[sec]==o.data[sec],before_compressed=bool(old[2]&0x08000000),after_compressed=bool(new[2]&0x08000000),unchanged_from_initializer_decoded=o.data[sec]==i.data[sec]))
coverage=sorted((h[4],h[4]+h[5],j)for j,h in enumerate(o.headers)if h[1]not in(0,8));cursor=64+51*40;padding=[]
for start,end,sec in coverage:
 assert cursor<=start and end<=len(b)
 if start>cursor:assert not any(b[cursor:start]);padding.append(dict(begin=cursor,end=start,bytes=start-cursor,all_zero=True,sha256=sha(b[cursor:start])))
 cursor=end
if cursor<len(b):assert not any(b[cursor:]);padding.append(dict(begin=cursor,end=len(b),bytes=len(b)-cursor,all_zero=True,sha256=sha(b[cursor:])))
assert b[:52]==a[:52]
save('byte-diff.json',dict(schema='Native178-callback-core-exhaustive-static-file-diff/1',parent=pin(parent),initializer=pin(init),candidate=pin(candidate),all_file_bytes_covered_by_blocks=True,block_bytes=65536,blocks=blocks,changed_or_added_same_offset_bytes=changes,file_growth_vs_metadata_parent=len(b)-len(a),file_growth_vs_initializer=len(b)-init.stat().st_size,all51stored_and_decoded_sections=sections,all_new_intersection_padding_zero=padding,all_exact_field_body_owner_entry_header_details=pin(H/'integration.json'),code_growth_bytes=7276,alignment_growth_bytes=0,ordinary_Game_DATA_BSS_growth_bytes=0,loader_metadata_allocation_growth_bytes=10648,repack_and_discarded_unclaimed_stored_STR_explain_file_size=True,no_runtime_or_binary_delta_application=True))
inputs=[parent,init,R/'source714-wait-sdk-metadata-r1/qualification.json',R/'source714-context-family-slice-r1/context-callback-family.o',R/'source714-context-family-slice-r1/qualification.json',R/'source714-context-family-slice-r1/extraction.json',R/'source714-context-family-slice-r1/source-use-proof.json',R/'source714-context-family-slice-independent-r1/qualification.json',R/'source714-native-invocation-cold-family-r2/qualification.json',R/'source714-native-invocation-cold-family-r2/report.json',reader,B/'work/TWWHD-GZ/patcher/native_rpx.py']
save('manifest.json',dict(schema='Native178-callback-core-NOT-FOR-INSTALL-static-candidate-manifest/1',frozen=True,inputs=[pin(p)for p in inputs],artifacts=[pin(H/x)for x in('materialize.py','freeze.py','README.md','Native178-initializer-and-callback-core.NOT-FOR-INSTALL.bin','integration.json','byte-diff.json')]))
save('qualification.json',dict(schema='Native178-callback-core-NOT-FOR-INSTALL-static-qualification/1',frozen=True,passed_within_static_serialization_scope=True,manifest=pin(H/'manifest.json'),candidate=pin(candidate),integration=pin(H/'integration.json'),exhaustive_byte_diff=pin(H/'byte-diff.json'),terminal_exit=0,live_processes=[],actual_new_CODE_bytes=7276,reused_owned_sampler_bytes=28,additional_alignment_bytes=0,new_true_global_Source_FUNC_definitions=18,selected_symbolic_fields=199,SDK_loader_REL24=28,additional_supported_callback_REL24=4,existing_Native_callback_words_changed=4,old_initializer_code_and_relocations_retained=True,all_16060_ordinary_Source_owner_bytes_borrowed_without_allocation_copy_clear=True,Source478_split_serial_record_and_comparison_only_sentinel=True,all_source53_private_local_edges_preserved=True,old_native_callback_bodies_unchanged=True,LR_rewrite_kind2_6_request_probe_gates_literal_preserved=True,ordinary_Game_DATA_BSS_growth_bytes=0,loader_metadata_allocation_growth_bytes=10648,all_old_SYM_st_name_st_other_and_RELA_prefixes_exact=True,STR_preferred_metadata_VMA_moved_after_grown_SYM_SHSTR=True,all_old_metadata_symbol_and_typed_target_censuses_empty=True,all51section_indices_retained=True,all_CRC_FileInfo_temp_and_mapped_bounds_checked=True,preserve_incoming_actual_TEXT_reserve_plus448=True,encoder_automatic4096_compensated=True,trampAddition=dict(before=432,after=460),no_raw_SDK_B_to_C000_metadata=True,Source_workers_zero_read_save_lifetime_world_services_activated=False,Native_Source_body_equivalence_claim=False,private_static_layout_selected=True,hardware_SDK_loader_runtime_concurrent_lifetime_read_save_authority=False,installable_release=False,apply_ready=False,version_Setup_FTP_Drive_GitHub_CP_or_current_Source_Native_changed=False,compiler_target_PPC_CPU_SDK_fixture_emulator_runtime_or_stopped_suite_execution=False,independent_binding_route_owner_container_reviews_separate=True))
print(json.dumps(dict(qualification=pin(H/'qualification.json'),manifest=pin(H/'manifest.json'),byte_diff=pin(H/'byte-diff.json'),candidate=pin(candidate),changed_file_bytes=changes,terminal_exit=0),indent=2))
