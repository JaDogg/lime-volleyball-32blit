#include "32blit.hpp"
#include "src/apu.hpp"
#include <sstream>

extern "C" {
#include "src/fake_wasm4_core.h"
}

uint32_t prev_time_ms = 0;
bool first_time = true;
double fps = 0.0;

void init() {
  blit::set_screen_mode(blit::ScreenMode::hires);
  init_apu();
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

void update(uint32_t time) {
  capture_input();
  wasm4_before_update();
  wasm4_export_update();
  play_audio(time, prev_time_ms, first_time);
  if (first_time) {
    first_time = false;
  } else {
    double frame_time = time - prev_time_ms;
    fps = (1.0 / frame_time) * 1000.0;
    prev_time_ms = time;
  }
}
