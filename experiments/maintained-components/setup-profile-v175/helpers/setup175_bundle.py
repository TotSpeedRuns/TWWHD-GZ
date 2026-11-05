"""Authored bounded WUHB parser; no installer executable or resource bytes."""
import struct

def bundle(raw):
    if len(raw) < 80:
        raise ValueError('Truncated WUHB')
    header = struct.unpack_from('>4sI9Q', raw)
    if header[:2] != (b'WUHB', 80):
        raise ValueError('Unsupported WUHB header')
    dh, dhn, dt, dtn, fh, fhn, ft, ftn, partition = header[2:]
    for at, size in ((dh,dhn),(dt,dtn),(fh,fhn),(ft,ftn)):
        if at + size > len(raw):
            raise ValueError('WUHB table exceeds file')
    rows, cursor = [], ft
    while cursor < ft + ftn:
        parent, sibling, offset, size, chained, namesize = struct.unpack_from('>IIQQII', raw, cursor)
        name = raw[cursor+32:cursor+32+namesize].decode('ascii')
        if cursor+32+namesize > ft+ftn or partition+offset+size > len(raw):
            raise ValueError('WUHB file exceeds bounds')
        rows.append(dict(name=name,parent=parent,table_offset=cursor,offset=partition+offset,
                         relative_offset=offset,size=size,data=raw[partition+offset:partition+offset+size]))
        cursor += (32+namesize+3)&~3
    if cursor != ft+ftn or [r['name'] for r in rows] != ['root.rpx','meta.ini','iconTex.tga.gz']:
        raise ValueError('Exact released bundle resource inventory changed')
    return header, rows
