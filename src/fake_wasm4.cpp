#include "32blit.hpp"

extern "C" {
uint32_t W4FAKE_PALETTE[4] = {0, 0, 0, 0};
uint16_t W4FAKE_DRAW_COLORS = 0;
uint8_t W4FAKE_GAMEPAD1 = 0;
uint8_t W4FAKE_GAMEPAD2 = 0;
uint8_t W4FAKE_GAMEPAD3 = 0;
uint8_t W4FAKE_GAMEPAD4 = 0;
int16_t W4FAKE_MOUSE_X = 0;
int16_t W4FAKE_MOUSE_Y = 0;
uint8_t W4FAKE_MOUSE_BUTTONS = 0;
uint8_t W4FAKE_SYSTEM_FLAGS = 0;
uint8_t W4FAKE_FRAME_BUFFER[160 * 160 >> 2] = {};

#include "fake_wasm4_core.h"
#include "framebuffer.h"
#include "util.h"

void wasm4_before_update() {
  if (!(W4FAKE_SYSTEM_FLAGS & SYSTEM_PRESERVE_FRAMEBUFFER)) {
    w4_framebufferClear();
  }
}

void init_wasm4() {
  w4_write32LE(&W4FAKE_PALETTE[0], 0xe0f8cf);
  w4_write32LE(&W4FAKE_PALETTE[1], 0x86c06c);
  w4_write32LE(&W4FAKE_PALETTE[2], 0x306850);
  w4_write32LE(&W4FAKE_PALETTE[3], 0x071821);
  w4_framebufferInit(reinterpret_cast<const uint8_t *>(&W4FAKE_DRAW_COLORS),
                     W4FAKE_FRAME_BUFFER);
}

void wasm4_blit(const uint8_t *data, int32_t x, int32_t y, uint32_t width,
                uint32_t height, uint32_t flags) {
  wasm4_blitSub(data, x, y, width, height, 0, 0, width, flags);
}

void wasm4_blitSub(const uint8_t *data, int32_t x, int32_t y, uint32_t width,
                   uint32_t height, uint32_t srcX, uint32_t srcY,
                   uint32_t stride, uint32_t flags) {
  bool bpp2 = (flags & 1);
  bool flipX = (flags & 2);
  bool flipY = (flags & 4);
  bool rotate = (flags & 8);
  w4_framebufferBlit(data, x, y, width, height, srcX, srcY, stride, bpp2, flipX,
                     flipY, rotate);
}

void wasm4_line(int32_t x, int32_t y, uint32_t width, uint32_t height) {
  w4_framebufferLine(x, y, width, height);
}

void wasm4_hline(int32_t x, int32_t y, uint32_t len) {
  w4_framebufferHLine(x, y, len);
}

void wasm4_vline(int32_t x, int32_t y, uint32_t len) {
  w4_framebufferVLine(x, y, len);
}

void wasm4_oval(int32_t x, int32_t y, uint32_t width, uint32_t height) {
  w4_framebufferOval(x, y, width, height);
}

void wasm4_rect(int32_t x, int32_t y, uint32_t width, uint32_t height) {
  w4_framebufferRect(x, y, width, height);
}

void wasm4_text(const char *text, int32_t x, int32_t y) {
  w4_framebufferText(reinterpret_cast<const uint8_t *>(text), x, y);
}

void wasm4_tone(uint32_t frequency, uint32_t duration, uint32_t volume,
                uint32_t flags) {
  wasm4_tone_callback(frequency, duration, volume, flags);
}

uint32_t wasm4_diskr(void *dest, uint32_t size) { return 0; }

uint32_t wasm4_diskw(const void *src, uint32_t size) { return 0; }

void wasm4_trace(const char *str) {}

void wasm4_draw() {
  uint32_t palette[4] = {
      w4_read32LE(&W4FAKE_PALETTE[0]),
      w4_read32LE(&W4FAKE_PALETTE[1]),
      w4_read32LE(&W4FAKE_PALETTE[2]),
      w4_read32LE(&W4FAKE_PALETTE[3]),
  };
  wasm4_draw_callback(palette, W4FAKE_FRAME_BUFFER);
}
}
