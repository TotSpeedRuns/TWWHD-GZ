"""Shrink native menu-only glyph sets; retain tool-panel glyphs unchanged."""
import re,struct
from PIL import Image

def shrink_menu_font(source):
 if '_gz_all_text_contact_kerning:' in source:
  # This font already has per-bank sizes, palette coverage and pair spacing.
  # Rebuilding with the older action font would discard its reviewed outline
  # and invalidate the kerning table; numeric palette indices are not alpha.
  return source
 paletted='; Action-label palette: white face, blue border, dark outer edge.' in source
 if paletted:
  # These RLE values select RGBA palette entries, not grayscale coverage.
  # Resampling their numeric indices creates unrelated colors. Render again
  # from the owned native font at the smaller menu size instead.
  from action_button_text import rebuild_glyphs
  source=rebuild_glyphs(source,menu_scale=.8)
 sharp='; Sharp glyph coverage, tight one-pixel outline.' in source
 for i in ([] if paletted else list(range(95))+list(range(190,285))):
  metric=re.search(rf'^\.int _gz_font_glyph{i},(\d+),(\d+),(-?\d+),(\d+)$',source,re.M)
  if metric is None:raise ValueError('Missing menu glyph metrics')
  w,h,bearing,advance=map(int,metric.groups())
  block=re.search(rf'^_gz_font_glyph{i}:\n((?:\.int 0x[0-9A-Fa-f]{{8}}\n)+)',source,re.M)
  if block is None:raise ValueError('Missing menu glyph data')
  packed=b''.join(struct.pack('>I',int(word,16)) for word in re.findall(r'0x([0-9A-Fa-f]{8})',block[1]))
  decoded=bytearray()
  for off in range(0,len(packed),2):
   count,value=packed[off:off+2]
   decoded.extend(bytes([value])*count)
   if len(decoded)>=w*h:break
  if len(decoded)!=w*h:raise ValueError('Glyph RLE length mismatch')
  nw,nh=round(w*.8),round(h*.8)
  image=Image.frombytes('L',(w,h),bytes(decoded)).resize((nw,nh),Image.Resampling.LANCZOS)
  raw=image.tobytes()
  encoded=bytearray();pos=0
  while pos<len(raw):
   end=pos+1
   while end<len(raw) and raw[end]==raw[pos] and end-pos<255:end+=1
   encoded.extend((end-pos,raw[pos]));pos=end
  encoded.extend(bytes((-len(encoded))%4))
  replacement=f'_gz_font_glyph{i}:\n'+''.join(f'.int 0x{struct.unpack_from(">I",encoded,n)[0]:08X}\n' for n in range(0,len(encoded),4))
  source=source[:block.start()]+replacement+source[block.end():]
  source=source.replace(metric[0],f'.int _gz_font_glyph{i},{nw},{nh},{round(bearing*.8)},{max(1,round(advance*.8))}',1)
 # Cursor RGBA sprites use the same menu scale.
 for label,size in [('_gz_cursor_pixels21',21),('_gz_cursor_pixels',32)]:
  block=re.search(rf'^{label}:\n((?:\.int 0x[0-9A-Fa-f]{{8}}\n)+)',source,re.M)
  if block is None:raise ValueError('Missing cursor sprite')
  packed=b''.join(struct.pack('>I',int(word,16)) for word in re.findall(r'0x([0-9A-Fa-f]{8})',block[1]))
  if len(packed)!=size*size*4:raise ValueError('Cursor dimensions differ')
  new=round(size*.8)
  raw=Image.frombytes('RGBA',(size,size),packed).resize((new,new),Image.Resampling.LANCZOS).tobytes()
  replacement=label+':\n'+''.join(f'.int 0x{struct.unpack_from(">I",raw,n)[0]:08X}\n' for n in range(0,len(raw),4))
  source=source[:block.start()]+replacement+source[block.end():]
 for label,end in [('_gz_game_text:','_gz_font_small:'),('_gz_draw_menu_cursor:','_gz_cursor_positioned:'),('_gz_inv_cursor:','_gz_inv_hearts_text:')]:
  start=source.index(label);stop=source.index(end,start);block=source[start:stop]
  for old,new in [('mulli r4, r4, 21','mulli r4, r4, 17'),('mulli r5, r5, 32','mulli r5, r5, 26'),('mulli r4, r4, 32','mulli r4, r4, 26'),('mulli r5, r5, 48','mulli r5, r5, 38'),('li r3, 21','li r3, 17'),('li r3, 32','li r3, 26'),('addi r5, r5, 6','addi r5, r5, 5'),('addi r5, r5, 9','addi r5, r5, 7')]:block=block.replace(old,new)
  source=source[:start]+block+source[stop:]
 # Keep the new outline masks at the same scale and advance as menu fills.
 for i in list(range(95))+list(range(190,285)):
  metric=re.search(rf'^\.int _gz_outline_glyph{i},(\d+),(\d+),(-?\d+),(\d+)$',source,re.M)
  if metric is None:continue
  w,h,bearing,advance=map(int,metric.groups())
  block=re.search(rf'^_gz_outline_glyph{i}:\n((?:\.int 0x[0-9A-Fa-f]{{8}}\n)+)',source,re.M)
  assert block is not None
  packed=b''.join(struct.pack('>I',int(word,16)) for word in re.findall(r'0x([0-9A-Fa-f]{8})',block[1]))
  decoded=bytearray()
  for off in range(0,len(packed),2):
   count,value=packed[off:off+2];decoded.extend(bytes([value])*count)
   if len(decoded)>=w*h:break
  assert len(decoded)==w*h
  nw,nh=round(w*.8),round(h*.8)
  raw=Image.frombytes('L',(w,h),bytes(decoded)).resize((nw,nh),Image.Resampling.LANCZOS).tobytes()
  encoded=bytearray();pos=0
  while pos<len(raw):
   end=pos+1
   while end<len(raw) and raw[end]==raw[pos] and end-pos<255:end+=1
   encoded.extend((end-pos,raw[pos]));pos=end
  encoded.extend(bytes((-len(encoded))%4))
  replacement=f'_gz_outline_glyph{i}:\n'+''.join(f'.int 0x{struct.unpack_from(">I",encoded,n)[0]:08X}\n' for n in range(0,len(encoded),4))
  source=source[:block.start()]+replacement+source[block.end():]
  source=source.replace(metric[0],f'.int _gz_outline_glyph{i},{nw},{nh},{round(bearing*.8)},{max(1,round(advance*.8))}',1)
 return source
