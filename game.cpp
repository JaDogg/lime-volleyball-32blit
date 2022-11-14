#include "game.hpp"

using namespace blit;

extern "C" {
#include "src/fake_wasm4_core.h"
void w4_framebufferClear();
}

void init() {
    set_screen_mode(ScreenMode::hires);
    init_wasm4();
    wasm4_export_start();
}

void render(uint32_t time) {
  blit::screen.alpha = 255;
  blit::screen.mask = nullptr;
  blit::screen.pen = blit::Pen(0, 0, 0);
  blit::screen.clear();
  wasm4_draw();
}


uint8_t* rgb(uint8_t* target, uint32_t colour) {
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
  int pixel = 0;
  for (int n = 0; n < 160 * 160 / 4; ++n) {
    uint8_t quartet = framebuffer[n];
    int c1 = (quartet & 0b00000011) >> 0;
    int c2 = (quartet & 0b00001100) >> 2;
    int c3 = (quartet & 0b00110000) >> 4;
    int c4 = (quartet & 0b11000000) >> 6;

    pixel++;
    int y = pixel / 160;
    int x = pixel % 160;
    rgb(blit::screen.ptr(x, y), palette[c1]);

    pixel++;
    y = pixel / 160;
    x = pixel % 160;
    rgb(blit::screen.ptr(x, y), palette[c2]);

    pixel++;
    y = pixel / 160;
    x = pixel % 160;
    rgb(blit::screen.ptr(x, y), palette[c3]);

    pixel++;
    y = pixel / 160;
    x = pixel % 160;
    rgb(blit::screen.ptr(x, y), palette[c4]);
  }
}
}

void capture_input();
void update(uint32_t time) {
  capture_input();
  if (!(W4FAKE_SYSTEM_FLAGS & SYSTEM_PRESERVE_FRAMEBUFFER)) {
    w4_framebufferClear();
  }
  wasm4_export_update();
}

void capture_input() {
  uint8_t gamepad = 0;
  if (blit::buttons & blit::Button::X) {
    gamepad |= BUTTON_1;
  }
  if (blit::buttons & blit::Button::Y) {
    gamepad |= BUTTON_2;
  }
  if (blit::buttons & blit::Button::DPAD_LEFT) {
    gamepad |= BUTTON_LEFT;
  }
  if (blit::buttons & blit::Button::DPAD_RIGHT) {
    gamepad |= BUTTON_RIGHT;
  }
  if (blit::buttons & blit::Button::DPAD_UP) {
    gamepad |= BUTTON_UP;
  }
  if (blit::buttons & blit::Button::DPAD_DOWN) {
    gamepad |= BUTTON_DOWN;
  }
  // update gamepad
  W4FAKE_GAMEPAD1 = gamepad;
}
