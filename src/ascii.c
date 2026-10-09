#include <stdio.h>
#include <stdlib.h>
#include "ascii.h"

static const char ramp[] = " .:-=+*#%@";

void ascii_print (const Frame *f, int cols, int rows) {
    char *buffer = malloc(rows * (cols + 1)); // +1 because each cols holds characters AND A newline
    if (buffer == NULL) {
        return;
    }

    int n = 0;

    // we subtract 2 cuz 1 is the null terminator and the other is because we want our index to start from 0
    int total_ramp_size = (int) sizeof(ramp) - 2;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {

            // finally omg this formula messed me up idk whats wrong with me tonight
            // DO THE multiply FIRST, because integer division THROWS AWAY fractions
            int x = col * f->width / cols;
            int y = row * f->height / rows;

            uint8_t pixel = f->pixels[y * f->stride + x];

            // scaling 0-255 down to a position in the ramp
            int ramp_index = pixel * (total_ramp_size + 1) / 256;

            buffer[n++] = ramp[ramp_index];
        }
        buffer[n++] = '\n';
    }

    fwrite(buffer, 1, n, stdout);
    free(buffer);
}

void ascii_fit (const Frame *f, int max_cols, int max_rows, int *out_cols, int *out_rows) {
    // as wide as the terminal
    int cols = max_cols;
    int rows = cols * f->height / f->width / 2;

    // as tall as the terminal
    if (rows > max_rows) {
        rows = max_rows;
        cols = rows * f->width * 2 / f->height;
    }

    *out_cols = cols;
    *out_rows = rows;
}