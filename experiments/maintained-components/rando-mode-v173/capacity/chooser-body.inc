/* Return only image-owned readonly storage; never retain an external pointer. */
const u16 *_gz_rando_capacity_select(const u16 *source, u32 source_units, u32 message_id) {
  u32 j,i; const struct recipe *r; const u16 *changed;
  for(j=0;j<10;j++) {
    r=&recipes[j];
    if(message_id!=r->id || source_units!=r->original_units) continue;
    changed=changed_text+r->changed_offset;
    for(i=0;i<r->prefix_units;i++) if(source[i]!=changed[i]) break;
    if(i!=r->prefix_units) continue;
    for(i=0;i<r->removed_units;i++) if(source[r->prefix_units+i]!=removed_text[r->removed_offset+i]) break;
    if(i!=r->removed_units) continue;
    for(i=r->prefix_units+r->removed_units;i<source_units;i++) if(source[i]!=changed[i-r->removed_units+r->inserted_units]) break;
    if(i==source_units) return changed;
  }
  return (const u16 *)0;
}
