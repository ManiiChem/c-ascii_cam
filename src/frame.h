#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int width;
    int height;
    int stride;
    uint8_t *pixels;
} Frame;

bool frame_create (Frame *f, int width, int height);
void frame_destroy (Frame *f);
void frame_fill_gradient (Frame *f);
void frame_fill_circle (Frame *f, int center_x, int center_y);
void frame_auto_contrast (Frame *f);