#ifndef GZ_BRIDGE_CURRENT_CAPTURE_H
#define GZ_BRIDGE_CURRENT_CAPTURE_H
unsigned int _gz_brg_root_read(unsigned int mode, unsigned int frame);
unsigned int _gz_brg_private_apart(unsigned int address, unsigned int bytes);
unsigned int _gz_brg_consume(unsigned int bank);
unsigned int _gz_brg_component_bytes(unsigned int bank);
unsigned int _gz_brg_install_note(unsigned int bank, unsigned int bytes);
unsigned int _gz_brg_parse_component(unsigned int address, unsigned int bytes);
void _gz_brg_discard(void);
#endif
