#!/usr/bin/env python3

import os
from pathlib import Path

roms = []
cwd = Path('.')

(cwd / 'gen').mkdir(exist_ok=True)

for f in cwd.glob('rom/*.nes'):
  name = f.stem
  if not (cwd / 'gen' / f'{name}.c').exists():
    os.system(f'echo "const " > gen/{name}.c')
    os.system(f'xxd -i "{f}" >> gen/{name}.c')
  roms.append(name)

for f in cwd.glob('gen/*.c'):
  if f.stem not in roms:
    f.unlink()

def h_file():
  for name in roms:
    yield 'extern const unsigned char rom_%s_nes[];' % (name)
    yield 'extern unsigned int rom_%s_nes_len;' % (name)
  yield '''
struct rom {
  const char *name;
  const void *body;
  unsigned int *size;
};

struct rom roms[] = {'''
  for name in roms:
    yield '  { .name = "%s", .body = rom_%s_nes, .size = &rom_%s_nes_len, },' % (name, name, name)
  yield '};'

  yield 'int nroms = %d;' % (len(roms))

with open('gen/roms.h', 'w') as fp:
  for line in h_file():
    fp.write(line)
    fp.write('\n')
