/* New CODE-only extension. Bind previous to the existing UNHOOKED UI400+
 * mapped68/heap/cut compound body, never to the native entry patched here.
 * The sole hook is the original genuine _gz_ois_heap_private_apart entry.
 * Existing external()/image_write_allowed()/capture private closure reaches
 * this seam even when earlier allocator private checks were inlined.
 */
typedef unsigned int U;
extern U _gz_rnp_previous_heap_private_apart(U,U);
extern U _gz_rns_private_apart(U,U);
extern U _gz_hx_private_apart(U,U);
extern U _gz_mit_private_apart(U,U);
U _gz_rnp_heap_private_apart(U pointer,U bytes)
{
 return _gz_rnp_previous_heap_private_apart(pointer,bytes)&&
  _gz_rns_private_apart(pointer,bytes)&&
  _gz_hx_private_apart(pointer,bytes)&&
  _gz_mit_private_apart(pointer,bytes);
}
