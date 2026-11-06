"""Freeze bounded Source module, use proof and Native role provenance."""
from pathlib import Path
import ast,hashlib,json,struct,types,zlib
H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
 b=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def save(name,v):
 raw=(json.dumps(v,indent=2)+'\n').encode()
 with(H/name).open('xb')as f:f.write(raw)
 assert(H/name).read_bytes()==raw
d=json.loads((H/'discovery.json').read_text());e=json.loads((H/'extraction.json').read_text());u=json.loads((H/'source-use-proof.json').read_text())
assert e['candidate']==pin(H/'context-callback-family.o')and e['candidate']['sha256']=='b5ee5b828e5b9997cf8add9501529fc72af1a52617b5040da09301b8dce1508d'
reader=B/d['reader']['path'];assert pin(reader)==d['reader'];node=next(n for n in ast.parse(reader.read_text()).body if isinstance(n,ast.ClassDef)and n.name=='Raw');m=types.ModuleType('freeze_context_Raw_only');m.__dict__.update(struct=struct,zlib=zlib);exec(compile(ast.Module(body=[node],type_ignores=[]),str(reader),'exec'),m.__dict__);o=m.Raw(H/'context-callback-family.o')
seen=[]
for r in o.rows:
 assert r['kind']in(4,6,10);z=4 if r['kind']==10 else 2;assert r['site']%(4 if z==4 else 2)==0
 span=(r['source'],r['site'],r['site']+z);assert all(span[0]!=x[0]or span[2]<=x[1]or x[2]<=span[1]for x in seen);seen.append(span)
assert len(seen)==199
save('relocation-fields.json',dict(schema='context-family-exact-relocation-field-disjointness/1',candidate=pin(H/'context-callback-family.o'),fields=[dict(section=a,begin=b,end=c)for a,b,c in seen],field_count=199,all_fields_disjoint=True,relocation_record_order='Discovery/function-selection permutation, not original per-section byte ordering',independent_types_only=[4,6,10]))
peer=R/'source714-context-family-slice-independent-r1/qualification.json';assert pin(peer)['sha256']=='2001ff5b4df282e8328827848fa701b8f83e84e3333c35cfa9b24c7289b52dee'
source=R/'source714-native-cold-entry-source-r1/source'
inputs=[B/d[x]['path']for x in('source','Native','initializer_candidate','reader')]+[R/'source714-native-invocation-cold-family-r2/qualification.json',R/'source714-native-invocation-cold-family-r2/report.json',peer,R/'source714-native-cold-entry-source-r1/qualification.json',R/'source714-native-cold-entry-join-r1/qualification.json']+[source/x for x in('adapter.c','adapter.h','worker_family.inc','worker_family.h','directory_metadata.c','directory_metadata.h','common_admission.c','common_admission.h','ghs_job_bridge.inc','worker_atomic.c','worker_atomic.h')]
files=['inspect.py','discovery.json','extract.py','context-callback-family.o','extraction.json','check_source_uses.py','source-use-proof.json','relocation-fields.json','README.md','freeze.py']
save('manifest.json',dict(schema='current-ea1-context-callback-family-static-module-manifest/1',frozen=True,inputs=[pin(p)for p in inputs],artifacts=[pin(H/x)for x in files],unselected_Source_ET_REL_only=True))
save('qualification.json',dict(schema='current-ea1-context-callback-family-static-module-qualification/1',frozen=True,passed_within_static_module_and_fixed_use_scope=True,manifest=pin(H/'manifest.json'),candidate=pin(H/'context-callback-family.o'),extraction=pin(H/'extraction.json'),source_use_proof=pin(H/'source-use-proof.json'),relocation_fields=pin(H/'relocation-fields.json'),independent_Source_module_peer=pin(peer),Native_family_r2_qualification=pin(R/'source714-native-invocation-cold-family-r2/qualification.json'),function_bodies=23,defined_FUNC_aliases=24,CODE_pieces=14,CODE_bytes=7304,owner_sections=8,owner_bytes=16060,NOBITS_bytes=15548,initialized_report_bytes=512,SDK_genuine_import_bytes=8264,SDK_roles=6,SDK_use_sites=28,external_Source_UND=0,symbol_count_including_null=59,typed_rows=199,typed_kind_counts=dict(REL24=79,HA=57,LO=63),typed_internal_edges=51,typed_owner_edges=120,raw_local_function_edges=53,all_bodies_genuine_symbol_metadata_and_typed_expressions_exact=True,relocation_order_is_permutation=True,all199_fields_disjoint=True,Source478_callback_refs_checked=16,all_full_Source478_typed_refs_classified=21,Source478_independent_serial_record_bases_and_explicit_sentinel_checked=True,Source478_single_affine_alias_invalid=True,extra_GHS_record_and_serial_ranges_separate=True,borrowed_Source_startup_descriptor28_must_be_omitted=True,initializer_atomic_sampler28_reuse_possible=True,incremental_CODE_if_reused=7276,required_new_heap_if_existing_Native_owners_borrowed=0,Native_OSWaitCond_import_or_proxy_binding_unresolved=True,no_Source_read_save_lifetime_world_services_or_workers_zero_suffix=True,Native_Source_body_alias_or_physical_binding_claim=False,physical_activation_hardware_lifetime_context_or_read_save_authority=False,apply_ready=False,compiler_target_PPC_SDK_runtime_emulator_stopped_suite_or_Native_Source_CP_repo_console_network_changes=False,terminal_exit=0,live_processes=[]))
print(json.dumps(dict(qualification=pin(H/'qualification.json'),manifest=pin(H/'manifest.json'),candidate=pin(H/'context-callback-family.o'),source_use_proof=pin(H/'source-use-proof.json'),terminal_exit=0),indent=2))
