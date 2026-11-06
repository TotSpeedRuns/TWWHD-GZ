"""Freeze only the completed static inventory and ET_REL extraction receipts."""
from pathlib import Path
import hashlib,json

H=Path(__file__).resolve().parent;R=H.parent;B=R.parents[1]
def pin(p):
    b=p.read_bytes();return dict(path=p.relative_to(B).as_posix(),bytes=len(b),sha256=hashlib.sha256(b).hexdigest())
def save(name,v):
    b=(json.dumps(v,indent=2)+'\n').encode()
    with (H/name).open('xb') as f:f.write(b)
    assert (H/name).read_bytes()==b

d=json.loads((H/'discovery.json').read_text());e=json.loads((H/'extraction.json').read_text())
assert e['passed'] and pin(B/e['source']['path'])==e['source'] and pin(B/e['candidate']['path'])==e['candidate']
assert (e['CODE_bytes'],e['owner_bytes'],e['typed_relocation_count'],e['symbol_count'],e['external_UND_count'])==(2740,1172,80,34,4)
assert e['kind_counts']=={'10':42,'6':17,'4':21}
inputs=[
R/'source714-native-cold-entry-join-r1/source-native-cold-entry-bound.o',
R/'source714-native-cold-entry-join-r1/qualification.json',
R/'source714-native-cold-entry-join-independent-r1/qualification.json',
R/'v178-current-features-test-r1/cking.rpx',
R/'ncb-kernel-scope-independent-r1/review.py',
R/'source714-init-integration-slice-r1/inspect.py',
R/'source714-init-integration-slice-r1/qualification.json',
R/'source714-native-jwa-owner-closure-r1/binder-contract.json',
R/'source712-native-cond-dispatch-r1/qualification.json',
]
expected={
'source714-native-cold-entry-join-r1/source-native-cold-entry-bound.o':'ea1be3dfad78db219c6fb577012ff9931fc7bb4840d371422f822d1144ef8f57',
'source714-native-cold-entry-join-r1/qualification.json':'993ee2267d21718c06be844e6ada0350d236b0728f269dfd9746fa72c1b97ab9',
'source714-native-cold-entry-join-independent-r1/qualification.json':'ecccd68e30f7c13ef2e00799b8ba1b1d988bced63b09db6345ef240693468dfe',
'v178-current-features-test-r1/cking.rpx':'37ea3c790f607b95814ece1d5f176c2b5cb8981344ca5fb2747a0e2c426623ac',
'ncb-kernel-scope-independent-r1/review.py':'515f9ba06da06d0136928f8e7a1e6e432ed91cc71cc9d83a9ce82e8fc6023bd1',
'source714-init-integration-slice-r1/qualification.json':'983c968b2c9f7f680b963fc1c049e4d5b24c57c3677f4d8dca5b230156f2c89f',
'source714-native-jwa-owner-closure-r1/binder-contract.json':'9f5ae96193c2e25c93d5a97e3c0adac718c6a5a4d33303160a086cbe571c2bc9',
'source712-native-cond-dispatch-r1/qualification.json':'6233adcbbeb7af287863a80537dde4bd7d434e8568b238c93d10ce8e9affe48f',
}
for p in inputs:
    if p.relative_to(R).as_posix() in expected:assert pin(p)['sha256']==expected[p.relative_to(R).as_posix()]
names=['derive_inventory.py','inspect.py','inventory-derivation.json','discovery.json','extract.py','minimal-init-source.o','extraction.json','README.md','freeze.py']
save('manifest.json',dict(schema='Current714-ea1-minimal-initializer-static-extraction-manifest/1',frozen=True,inputs=[pin(p)for p in inputs],artifacts=[pin(H/n)for n in names],object_is_unselected_ET_REL=True,no_physical_Native_bindings=True,input_documents_are_provenance_not_admission_of_stale_full_Source_profiles=True))
save('qualification.json',dict(schema='Current714-ea1-minimal-initializer-static-extraction-qualification/1',frozen=True,passed_within_static_extraction_scope=True,manifest=pin(H/'manifest.json'),discovery=pin(H/'discovery.json'),extraction=pin(H/'extraction.json'),source=e['source'],candidate=e['candidate'],function_bodies=10,defined_FUNC_symbols=11,CODE_pieces=9,CODE_bytes=2740,shared_owner_bytes=1172,actual_owner_aliases=13,NOBITS_bytes=916,initialized_owner_PROGBITS_bytes=256,retained_genuine_fimport_coreinit_bytes=8264,symbol_count_including_null=34,typed_relocations=80,kind_counts=dict(REL24=42,HA=17,LO=21,ADDR32=0),Source_internal_typed_edges=14,local_GFO_tail_edge=1,SDK_roles=5,SDK_use_sites=24,external_UND_roles=4,external_use_sites=17,owner_use_sites=25,additional_direct_literal_or_indirect_frontier=0,all_selected_bodies_and_genuine_symbol_metadata_preserved=True,all_typed_operands_rebased_and_reopened_exactly=True,one_contiguous_GFO304_local_tail_preserved=True,SDK5_remain_defined_in_genuine_import_pseudosection=True,Source_owner_kinds_aliases_and_contents_retained_once=True,SDK_external_or_physical_owner_bindings_selected=False,Native_cache_context_locks_cold_lifetime_or_activation_proved=False,whole_Source_profiles_applicable_to_capsule=False,compiler_target_SDK_PPC_CPU_runtime_emulator_or_stopped_suite_execution=False,currentSource_Native_CP_repo_console_or_network_changes=False,apply_ready=False,independent_peer_receipt_separate=True))
print(json.dumps(dict(manifest=pin(H/'manifest.json'),qualification=pin(H/'qualification.json'),candidate=e['candidate']),indent=2))
