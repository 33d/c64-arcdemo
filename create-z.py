#!/usr/bin/env python3

frames=9
a=10
b=1500
c=132

proj = lambda x: int(1/(x+a)*b+c)

raster_top = 28

def create_line(n):
  n = 200 - (frames + n)
  prev_off = None
  while n > 0:
    next_on = proj(n)
    next_off = proj(n-2)
    if next_off > 240:
      break
    if next_on != prev_off and next_on != next_off:
      yield(next_on + raster_top)
      yield(next_off + raster_top)
    prev_off = next_off
    n -= frames
  yield(0)

data = [f"{x}u" for n in range(frames) for x in create_line(n)]
data.reverse()

with open("zframes.c", "w") as f:
  f.write("#include <stdint.h>\n")
  f.write("#include <stddef.h>\n")
  f.write('#include "zframes.h"\n')
  f.write("const uint8_t all_lines[] = {\n")
  f.write(", ".join(data) + '\n')
  f.write('};\n')

with open("zframes.h", "w") as f:
  f.write("#if !defined(ZFRAMES_H)\n")
  f.write("#define ZFRAMES_H\n")
  f.write(f'#define ALL_LINES_COUNT {len(data)}\n') 
  f.write(f'extern const uint8_t all_lines[{len(data)}];\n')
  f.write("#endif\n");
  

