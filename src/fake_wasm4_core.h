#pragma once

#include <stdint.h>

/**
 * Initialize wasm4
 */
void init_wasm4();
/**
 * Exported start() method
 */
void wasm4_export_start();
/**
 * Exported update method
 */
void wasm4_export_update();
/**
 * Call this before export update is called
 */
void wasm4_before_update();
// Call this to invoke draw callback
void wasm4_draw();
// To be implemented by game engine
void wasm4_draw_callback(const uint32_t *palette, const uint8_t *framebuffer);
// To be implemented by game engine
void wasm4_tone_callback(uint32_t frequency, uint32_t duration, uint32_t volume,
                         uint32_t flags);

// ┌───────────────────────────────────────────────────────────────────────────┐
// │                                                                           │
// │ Platform Constants                                                        │
// │                                                                           │
// └───────────────────────────────────────────────────────────────────────────┘

#define SCREEN_SIZE 160

// ┌───────────────────────────────────────────────────────────────────────────┐
// │                                                                           │
// │ Memory Addresses                                                          │
// │                                                                           │
// └───────────────────────────────────────────────────────────────────────────┘

extern uint32_t W4FAKE_PALETTE[4];
extern uint16_t W4FAKE_DRAW_COLORS;
extern uint8_t W4FAKE_GAMEPAD1;
extern uint8_t W4FAKE_GAMEPAD2;
extern uint8_t W4FAKE_GAMEPAD3;
extern uint8_t W4FAKE_GAMEPAD4;
extern int16_t W4FAKE_MOUSE_X;
extern int16_t W4FAKE_MOUSE_Y;
extern uint8_t W4FAKE_MOUSE_BUTTONS;
extern uint8_t W4FAKE_SYSTEM_FLAGS;
extern uint8_t W4FAKE_FRAME_BUFFER[160 * 160 >> 2];

#define PALETTE (W4FAKE_PALETTE)
#define DRAW_COLORS (&W4FAKE_DRAW_COLORS)
#define GAMEPAD1 ((const uint8_t *)&W4FAKE_GAMEPAD1)
#define GAMEPAD2 ((const uint8_t *)&W4FAKE_GAMEPAD2)
#define GAMEPAD3 ((const uint8_t *)&W4FAKE_GAMEPAD3)
#define GAMEPAD4 ((const uint8_t *)&W4FAKE_GAMEPAD4)
#define MOUSE_X ((const int16_t *)&W4FAKE_MOUSE_X)
#define MOUSE_Y ((const int16_t *)&W4FAKE_MOUSE_Y)
#define MOUSE_BUTTONS ((const uint8_t *)&W4FAKE_MOUSE_BUTTONS)
#define SYSTEM_FLAGS (&W4FAKE_SYSTEM_FLAGS)
#define FRAMEBUFFER (W4FAKE_FRAME_BUFFER)

#define BUTTON_1 1
#define BUTTON_2 2
#define BUTTON_LEFT 16
#define BUTTON_RIGHT 32
#define BUTTON_UP 64
#define BUTTON_DOWN 128

#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4

#define SYSTEM_PRESERVE_FRAMEBUFFER 1
#define SYSTEM_HIDE_GAMEPAD_OVERLAY 2

// ┌───────────────────────────────────────────────────────────────────────────┐
// │                                                                           │
// │ Drawing Functions                                                         │
// │                                                                           │
// └───────────────────────────────────────────────────────────────────────────┘

/** Copies pixels to the framebuffer. */
void wasm4_blit(const uint8_t *data, int32_t x, int32_t y, uint32_t width,
                uint32_t height, uint32_t flags);

/** Copies a subregion within a larger sprite atlas to the framebuffer. */
void wasm4_blitSub(const uint8_t *data, int32_t x, int32_t y, uint32_t width,
                   uint32_t height, uint32_t srcX, uint32_t srcY,
                   uint32_t stride, uint32_t flags);

#define BLIT_2BPP 1
#define BLIT_1BPP 0
#define BLIT_FLIP_X 2
#define BLIT_FLIP_Y 4
#define BLIT_ROTATE 8

/** Draws a line between two points. */
void wasm4_line(int32_t x, int32_t y, uint32_t width, uint32_t height);

/** Draws a horizontal line. */
void wasm4_hline(int32_t x, int32_t y, uint32_t len);

/** Draws a vertical line. */
void wasm4_vline(int32_t x, int32_t y, uint32_t len);

/** Draws an oval (or circle). */
void wasm4_oval(int32_t x, int32_t y, uint32_t width, uint32_t height);

/** Draws a rectangle. */
void wasm4_rect(int32_t x, int32_t y, uint32_t width, uint32_t height);

/** Draws text using the built-in system font. */
void wasm4_text(const char *text, int32_t x, int32_t y);

// ┌───────────────────────────────────────────────────────────────────────────┐
// │                                                                           │
// │ Sound Functions                                                           │
// │                                                                           │
// └───────────────────────────────────────────────────────────────────────────┘

/** Plays a sound tone. */
void wasm4_tone(uint32_t frequency, uint32_t duration, uint32_t volume,
                uint32_t flags);

#define TONE_PULSE1 0
#define TONE_PULSE2 1
#define TONE_TRIANGLE 2
#define TONE_NOISE 3
#define TONE_MODE1 0
#define TONE_MODE2 4
#define TONE_MODE3 8
#define TONE_MODE4 12

// ┌───────────────────────────────────────────────────────────────────────────┐
// │                                                                           │
// │ Storage Functions                                                         │
// │                                                                           │
// └───────────────────────────────────────────────────────────────────────────┘

/** Reads up to `size` bytes from persistent storage into the pointer `dest`. */
uint32_t wasm4_diskr(void *dest, uint32_t size);

/** Writes up to `size` bytes from the pointer `src` into persistent storage. */
uint32_t wasm4_diskw(const void *src, uint32_t size);

/** Prints a message to the debug console. */
void wasm4_trace(const char *str);
