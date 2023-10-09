#include "debug.h"

void draw_vert_line(uint8_t *debug_bitmap, int32_t x, int32_t y,
                    int32_t extent) {
  if (extent <= 0 || x >= 400 || x < 0 || y >= 240 || y + extent < 0)
    return;
  if (y + extent >= 240)
    extent = 240 - y;
  if (y < 0) {
    extent += y;
    y = 0;
  }
  for (int i = 0; i < extent; i++) {
    uint8_t *bitmap_row = (uint8_t *)debug_bitmap + ((y + i) * 52);
    bitmap_row[x / 8] |= 1 << (7 - (x % 8));
  }
}

void draw_horiz_line(uint8_t *debug_bitmap, int32_t x, int32_t y,
                     int32_t extent) {
  if (extent <= 0 || y >= 240 || y < 0 || x >= 400 || x + extent < 0)
    return;
  if (x + extent >= 400)
    extent = 400 - x;
  if (x < 0) {
    extent += x;
    x = 0;
  }
  uint8_t *bitmap_row = (uint8_t *)debug_bitmap + (y * 52);
  for (int i = 0; i < extent; i++) {
    bitmap_row[(x + i) / 8] |= 1 << (7 - ((x + i) % 8));
  }
}

// Draws a rectangle directly onto the bitmap data for the debug bitmap.
void debug_draw_rect(uint8_t *debug_bitmap, int32_t x, int32_t y, int32_t width,
                     int32_t height) {
  draw_horiz_line(debug_bitmap, x, y, width);
  draw_horiz_line(debug_bitmap, x, y + height - 1, width);
  draw_vert_line(debug_bitmap, x, y, height);
  draw_vert_line(debug_bitmap, x + width - 1, y, height);
}
