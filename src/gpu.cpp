#include "32blit.hpp"

#define WASM4_SIZE 160
#define WASM4_PIXELS_PER_BYTE 4

/**
 * Write rgb to given pointer (3 bytes are written)
 * @param target
 * @param colour
 * @return
 */
uint8_t *rgb(uint8_t *target, uint32_t colour) {
  uint8_t r = (((colour & 0xff0000) >> 8) >> 8);
  uint8_t g = ((colour & 0xff00) >> 8);
  uint8_t b = (colour & 0xff);
  *target++ = r;
  *target++ = g;
  *target++ = b;
  return target;
}

extern "C" {
void wasm4_draw_callback(const uint32_t *palette, const uint8_t *framebuffer) {
  int pixel = -1, c1, c2, c3, c4, x, y;
  const int framebuffer_items = WASM4_SIZE * WASM4_SIZE / WASM4_PIXELS_PER_BYTE;
  for (int n = 0; n < framebuffer_items; ++n) {
    uint8_t quartet = framebuffer[n];
    c1 = (quartet & 0b00000011) >> 0;
    c2 = (quartet & 0b00001100) >> 2;
    c3 = (quartet & 0b00110000) >> 4;
    c4 = (quartet & 0b11000000) >> 6;

    pixel++;
    y = pixel / WASM4_SIZE;
    x = pixel % WASM4_SIZE;
    rgb(blit::screen.ptr(x, y), palette[c1]);

    pixel++;
    y = pixel / WASM4_SIZE;
    x = pixel % WASM4_SIZE;
    rgb(blit::screen.ptr(x, y), palette[c2]);

    pixel++;
    y = pixel / WASM4_SIZE;
    x = pixel % WASM4_SIZE;
    rgb(blit::screen.ptr(x, y), palette[c3]);

    pixel++;
    y = pixel / WASM4_SIZE;
    x = pixel % WASM4_SIZE;
    rgb(blit::screen.ptr(x, y), palette[c4]);
  }
}
}
