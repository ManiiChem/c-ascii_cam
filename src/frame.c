#include <stdlib.h>
#include "frame.h"

bool frame_create (Frame *f, int width, int height) {
    f->width = width;
    f->height = height;

    // setting stride to width for now, if the camera decides to add empty bytes to the frame then ima change it
    f->stride = width;
    f->pixels = malloc(width * height);

    if (f->pixels == NULL) {
        return false;
    }

    return true;
}

void frame_destroy (Frame *f) {
    free(f->pixels);
    f->pixels = NULL; // preventing accidental use of a stale pointer
}

void frame_fill_gradient (Frame *f) {
    for (int y = 0; y < f->height; y++) {
        for (int x = 0; x < f->width; x++) {
            // brightness goes from 0 at the left edge to 255 at the right edge
            int brightness = y * 255 / (f->height - 1); // top to down gradient
            //int brightness = x * 255 / (f->width - 1); // right to left gradient

            f->pixels[y * f->stride + x] = (uint8_t) brightness;
        }
    }
}

void frame_fill_circle (Frame *f, int center_x, int center_y) {
    int radius = f->height / 3;

    for (int y = 0; y < f->height; y++) {
        for (int x = 0; x < f->width; x++) {
            int brightness = 0;
            
            // measuring how far each pixel is from the centre
            int dx = x - center_x;
            int dy = y - center_y;
            
            // good ole pythagoras
            if (dx * dx + dy * dy <= radius * radius) {
                brightness = 255;
            }

            f->pixels[y * f->stride + x] = (uint8_t) brightness;
        }
    }
}