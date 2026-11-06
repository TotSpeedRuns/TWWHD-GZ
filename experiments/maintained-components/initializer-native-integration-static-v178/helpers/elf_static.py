import struct
import zlib

def need(value,message):
    if not value: raise ValueError(message)

class ELF:
    """Local independent data parser; never imports or executes reviewed files."""
    def __init__(self, path):
        raw = path.read_bytes()
        need(raw[:7] == b'\x7fELF\x01\x02\x01' and struct.unpack_from('>H', raw, 18)[0] == 20, 'ELF32BE PPC')
        offset = struct.unpack_from('>I', raw, 32)[0]
        stride, count, names = struct.unpack_from('>HHH', raw, 46)
        need(stride == 40 and 0 < count <= 1024 and offset + count * stride <= len(raw), 'section bounds')
        self.headers = [struct.unpack_from('>10I', raw, offset + i * stride) for i in range(count)]
        self.data = []
        for h in self.headers:
            need(h[1] in (0, 8) or h[4] + h[5] <= len(raw), 'payload bounds')
            data = b'' if h[1] in (0, 8) else raw[h[4]:h[4] + h[5]]
            if h[2] & 0x08000000 and h[1] not in (0, 8):
                size = struct.unpack_from('>I', data)[0]
                need(size <= 64 * 1024 * 1024, 'bounded decompression')
                decoder = zlib.decompressobj()
                data = decoder.decompress(data[4:], size + 1)
                need(len(data) == size and decoder.eof and not decoder.unused_data, 'compressed extent')
            self.data.append(data)
        strings = self.data[names]
        self.names = [strings[h[0]:].split(b'\0', 1)[0].decode('ascii') for h in self.headers]
        self.symbols = {}
        for i, h in enumerate(self.headers):
            if h[1] != 2:
                continue
            need(h[9] == 16 and len(self.data[i]) % 16 == 0, 'symbol records')
            strings = self.data[h[6]]
            for j, (name, value, size, info, other, section) in enumerate(struct.iter_unpack('>IIIBBH', self.data[i])):
                self.symbols[i, j] = dict(name=strings[name:].split(b'\0', 1)[0].decode('ascii'), value=value, size=size, info=info, other=other, section=section, table=i, index=j)
        self.rows = []
        for h, data in zip(self.headers, self.data):
            if h[1] == 4:
                need(h[9] == 12 and len(data) % 12 == 0, 'RELA records')
                for site, info, addend in struct.iter_unpack('>IIi', data):
                    self.rows.append(dict(source=h[7], site=site, kind=info & 255, addend=addend, symbol=self.symbols[h[6], info >> 8]))

    def named(self, name):
        found = [s for s in self.symbols.values() if s['name'] == name]
        need(len(found) == 1, 'unique symbol ' + name)
        return found[0]
