import sys
import re

src = sys.stdin.read()

# reverses the bits, from bit twiddling hacks
reverse = lambda b: (((b * 0x0802 & 0x22110) | (b * 0x8020 & 0x88440)) * 0x10101 >> 16) & 0xFF

x = 320
y = 48

data = [int(x[2:], base=16) for x in re.findall('0x[0-9a-f][0-9a-f]', src)]
print(re.match('(.*\n){3}', src)[0].replace("static ", ""))
print(", ".join(hex(reverse(data[(n//8)%(x//8)+(n%8)*x//8+(n//x*x)])) for n in range(x*y//8)))
print("};")

