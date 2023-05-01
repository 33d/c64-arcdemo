#include <tgi.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <6502.h>
#include "zframes.h"

#define HILL_BASE (120u)

extern const uint8_t* arclogo_bits;

struct pixel {
  int x;
  int y;
};

void draw_mountains() {
  int i, prev_x, prev_y, x_loop, y_loop, x, y;
  bool not_first_line = false;
  struct pixel prev_line[16];
  prev_line[0].x = 0;
  for (y_loop = 28; y_loop > 0; y_loop -= 8) {
    prev_x = 0;
    prev_y = y = HILL_BASE - rand() % y_loop;
    prev_line[0].y = prev_y;
    x_loop = 12;
    for (i = 1; i < 17; i++, x_loop += 20) {
      struct pixel *prev = prev_line + i;
      int blank_y;
      x = x_loop + (rand() % 16);
      y = HILL_BASE - rand() % y_loop;
      tgi_line(prev_x, prev_y, x, y);
      tgi_setcolor(0);
      for (blank_y = y_loop; blank_y > 1; blank_y--)
        tgi_line(prev_x, prev_y + blank_y, x, y + blank_y);
      tgi_setcolor(1);
      if (not_first_line && prev->y < y) {
        tgi_line(prev->x, prev->y, x, y);
      }
      prev_x = prev->x = x;
      prev_y = prev->y = y;
    }
    not_first_line = true;
  }
}

void draw_grid() {
  int x, base_x;
  for (x = -1024; x < 1024; x += 64) {
    int top_x = 160 + (x >> 3);
    for (base_x = x + 157; base_x < x + 164; base_x++) {
      tgi_line(top_x, HILL_BASE, base_x, 199);
    }
  }
}

/*

VIC memory map:

Address  VIC   Size   Use
  bus    addr
$c000    0     $4000  VIC memory bank
$d000   $1000   $3e8  Screen RAM (hi-res colour)
$d400   $1400   $3e8  Alternate (all purple) screen RAM
$e000   $2000  $1f40  Bitmap memory
                      The character memory goes in the top half
*/

#define SET_COLOR_ITER ((200u - HILL_BASE) / 16 * 40)
#define LINE_SIZE(x) (40u * (x)) 
#define COLOR_LINE_START(x) ((uint8_t*) 0xd000u + LINE_SIZE(x))
#define TEXT_COLOR_LINE_START(x) (0xd800u + LINE_SIZE(x)) 

void set_colors() {
  register uint8_t i;

  __asm__("sei");
  // Writes to $d000 write to memory, not the VIC
  __asm__("lda $01");
  __asm__("pha");
  __asm__("and #%%11111100"); // clear bits 0 and 1
  __asm__("ora #%%00000100"); // clear bits 0 and 1
  __asm__("sta $01");

  // Fill in the memory before the logo with spaces
  memset(COLOR_LINE_START(0), 0, LINE_SIZE(3));

  // Text for the logo
  i = 240;
  do {
    ((uint8_t[240]) COLOR_LINE_START(3))[i] = i;
  } while (--i > 0);

  // The bottom of the screen is filled with 0x40, so it needs
  // to be blank, so replace it
  *((uint8_t*) COLOR_LINE_START(3) + 0x40) = 0xF0;

  // Hill colour
  memset(COLOR_LINE_START((HILL_BASE / 8) - 4), 0xE0, LINE_SIZE(4));
  
  // Ground colour
  memset(COLOR_LINE_START(HILL_BASE / 8), 0x40, 
    COLOR_LINE_START(26) - COLOR_LINE_START(HILL_BASE / 8)
  );
  
  // The logo as character data
  memcpy((void*) 0xE000, &arclogo_bits, 320U*48/8);
  
  // The bottom half of the screen is filled with 0x40, but that
  // forms part of the logo. Copy that somewhere else and blank
  // that character.
  memcpy((void*) (0xE000 + 0xF0 * 8), (void*) (0xE000 + 0x40 * 8), 8);
  memset((void*) (0xE000 + 0x40 * 8), 0, 8);

  // Restore the $d000 address space
  __asm__("pla");
  __asm__("sta $01");
  __asm__("cli");
  
  // Set the text foreground colour to black
  // Do one more line - I don't quite understand why
  memset((uint8_t*) TEXT_COLOR_LINE_START(0), 0, 10*40);

  VIC.bgcolor0 = 1;
}

void animate() {
  uint8_t vicaddr_main, vicaddr_alt, vicaddr_text, textmode, gfxmode;
  uint8_t i, nextlineindex = ALL_LINES_COUNT, nextline;
  
  SEI();

  // VIC.addr for the main colours
  vicaddr_main = VIC.addr;
  // VIC.addr for the alternative colours
  vicaddr_alt = (VIC.addr & 0xF1) | 0x50;
  vicaddr_text = (VIC.addr & 0xF1) | 0x08; // character memory at $2000
  textmode = VIC.ctrl1 & ~0x20; // BMM=0
  gfxmode = VIC.ctrl1 | 0x20; // BMM=1
  
  // http://www.codebase64.org/doku.php?id=base:rasterbars_source
    
  VIC.ctrl2 &= ~0x10; // MCM=0
  
  while (true) {
    i = 0;

    // Wait for the raster to hit the top
    while (VIC.rasterline != 0 || (VIC.ctrl1 & 0x80) != 0); //(50 + 8*3 + 1));

    VIC.bgcolor0 = COLOR_WHITE;

    VIC.addr = vicaddr_text;
    VIC.ctrl1 = textmode;

    while (VIC.rasterline != 51 + 8*6 + 1);
    VIC.bgcolor0 = COLOR_LIGHTBLUE;
    while (VIC.rasterline != 51 + 8*6 + 7);
    VIC.bgcolor0 = COLOR_BLUE;
    while (VIC.rasterline != 51 + 8*6 + 9);
    VIC.bgcolor0 = COLOR_BLACK;
    while (VIC.rasterline != 51 + 8*6 + 11);
    VIC.bgcolor0 = COLOR_BROWN;
    while (VIC.rasterline != 51 + 8*6 + 13);
    VIC.bgcolor0 = COLOR_ORANGE;
    while (VIC.rasterline != 51 + 8*6 + 15);
    VIC.bgcolor0 = COLOR_WHITE;

    // Wait for the raster to pass the logo
    while (VIC.rasterline != 51 + 8*9);
    
    // Enter graphics mode
    VIC.addr = vicaddr_main;
    VIC.ctrl1 = gfxmode;
    VIC.bgcolor0 = COLOR_PURPLE;

    while ((nextline = all_lines[--nextlineindex]) != 0) {
      while (VIC.rasterline != nextline);
      //VIC.addr = vicaddr_alt;
      VIC.ctrl1 = textmode;
      ++i;
    
      (nextline = all_lines[--nextlineindex]);
      while (VIC.rasterline != nextline);
      //VIC.addr = vicaddr_main;
      VIC.ctrl1 = gfxmode;
      ++i;
    }
    
    if (nextlineindex == 0)
      nextlineindex = ALL_LINES_COUNT;
  }
} 

void main(void) {
  tgi_install(tgi_static_stddrv);
  tgi_init();
  tgi_clear();
  
  VIC.bordercolor = COLOR_GRAY1;
  set_colors();  

  draw_mountains();
  draw_grid();
  
  animate();
}

