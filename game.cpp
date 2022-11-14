#include "game.hpp"
#include <iostream>
#include <sstream>
using namespace blit;

extern "C" {
#include "src/fake_wasm4_core.h"
void w4_framebufferClear();
}

uint32_t prev_time_ms = 0;
bool first_time = true;
double fps = 0.0;
std::stringstream temp_sstream;

void init() {
  set_screen_mode(ScreenMode::hires);
  blit::channels[0].waveforms = blit::Waveform::SQUARE;
  blit::channels[1].waveforms = blit::Waveform::SQUARE;
  blit::channels[2].waveforms = blit::Waveform::TRIANGLE;
  blit::channels[3].waveforms = blit::Waveform::NOISE;
  init_wasm4();
  wasm4_export_start();
}

void render(uint32_t time) {
  blit::screen.alpha = 255;
  blit::screen.mask = nullptr;
  blit::screen.pen = blit::Pen(0, 0, 0);
  blit::screen.clear();
  wasm4_draw();
  blit::screen.pen = blit::Pen(255, 255, 255);
  temp_sstream.str("");
  temp_sstream.clear();
  temp_sstream << "FPS:  " << fps;
  blit::screen.text(temp_sstream.str(), blit::minimal_font,
                    blit::Point(5, 165));
}

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
  int pixel = -1;
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

int g_min(int x, int y) { return x < y ? x : y; }
int g_max(int x, int y) { return x > y ? x : y; }

enum class AUDIO_STEP {
  N /*Nothing*/,
  A_START,
  A_HOLD,
  D_START,
  D_HOLD,
  S_START,
  S_HOLD,
  R_START,
  R_HOLD
};
int attack_ms[4] = {0, 0, 0, 0};
int decay_ms[4] = {0, 0, 0, 0};
int sustain_ms[4] = {0, 0, 0, 0};
int release_ms[4] = {0, 0, 0, 0};
AUDIO_STEP audio_task[4] = {AUDIO_STEP::N, AUDIO_STEP::N, AUDIO_STEP::N,
                            AUDIO_STEP::N};

extern "C" {
void wasm4_tone_callback(uint32_t frequency, uint32_t duration, uint32_t volume,
                         uint32_t flags) {
  int freq1 = frequency & 0xffff;
  int freq2 = (frequency >> 16) & 0xffff;

  int sustain = duration & 0xff;
  int release = (duration >> 8) & 0xff;
  int decay = (duration >> 16) & 0xff;
  int attack = (duration >> 24) & 0xff;

  int sustainVolume = g_min(volume & 0xff, 100);
  int peakVolume = g_min((volume >> 8) & 0xff, 100);

  int channelIdx = flags & 0x03;
  int mode = (flags >> 2) & 0x3; // TODO
  int pan = (flags >> 4) & 0x3;  // Cannot be done unless we support stereo
  blit::channels[channelIdx].frequency = freq1;
  blit::channels[channelIdx].filter_cutoff_frequency = freq2;
  blit::channels[channelIdx].sustain = 0xffff; // * sustainVolume / 100;
  blit::channels[channelIdx].volume = 0xffff;  //* peakVolume / 100;
  blit::channels[channelIdx].attack_ms = g_max(attack * 1000 / 60, 1);
  blit::channels[channelIdx].decay_ms = g_max(decay * 1000 / 60, 1);
  blit::channels[channelIdx].release_ms = g_max(release * 1000 / 60, 1);
  attack_ms[channelIdx] = g_max(attack * 1000 / 60, 1);
  decay_ms[channelIdx] = g_max(decay * 1000 / 60, 1);
  sustain_ms[channelIdx] = g_max(sustain * 1000 / 60, 1);
  release_ms[channelIdx] = g_max(release * 1000 / 60, 1);
  audio_task[channelIdx] = AUDIO_STEP::A_START;
}
}



void play_audio(uint32_t time_ms) {
  int delta = 0;
  if (first_time) {
    first_time = false;
  } else {
    delta = time_ms - prev_time_ms;
    prev_time_ms = time_ms;
  }
  for (int chan = 0; chan < 4; chan++) {
    if (audio_task[chan] == AUDIO_STEP::N) {
      continue;
    }
    // ATTACK
    if (audio_task[chan] == AUDIO_STEP::A_START) {
      if (attack_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::D_START;
      } else {
        audio_task[chan] = AUDIO_STEP::A_HOLD;
        blit::channels[chan].trigger_attack();
      }
      continue;
    }
    if (audio_task[chan] == AUDIO_STEP::A_HOLD) {
      attack_ms[chan] -= delta;
      if (attack_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::D_START;
      }
    }
    // DECAY
    if (audio_task[chan] == AUDIO_STEP::D_START) {
      if (decay_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::S_START;
      } else {
        audio_task[chan] = AUDIO_STEP::D_HOLD;
        blit::channels[chan].trigger_decay();
      }
      continue;
    }
    if (audio_task[chan] == AUDIO_STEP::D_HOLD) {
      decay_ms[chan] -= delta;
      if (decay_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::S_START;
      }
    }
    // SUSTAIN
    if (audio_task[chan] == AUDIO_STEP::S_START) {
      if (sustain_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::R_START;
      } else {
        audio_task[chan] = AUDIO_STEP::S_HOLD;
        blit::channels[chan].trigger_sustain();
      }
      continue;
    }
    if (audio_task[chan] == AUDIO_STEP::S_HOLD) {
      sustain_ms[chan] -= delta;
      if (sustain_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::R_START;
      }
    }
    // RELEASE
    if (audio_task[chan] == AUDIO_STEP::R_START) {
      if (release_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::N;
        blit::channels[chan].off();
      } else {
        audio_task[chan] = AUDIO_STEP::R_HOLD;
        blit::channels[chan].trigger_release();
      }
      continue;
    }
    if (audio_task[chan] == AUDIO_STEP::R_HOLD) {
      release_ms[chan] -= delta;
      if (release_ms[chan] <= 0) {
        audio_task[chan] = AUDIO_STEP::N;
        blit::channels[chan].off();
      }
    }
  }
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
  if (!first_time) {
    double frame_time = time - prev_time_ms;
    fps = (1.0 / frame_time) * 1000.0;
  }
  capture_input();
  if (!(W4FAKE_SYSTEM_FLAGS & SYSTEM_PRESERVE_FRAMEBUFFER)) {
    w4_framebufferClear();
  }
  wasm4_export_update();
  play_audio(time);
}
