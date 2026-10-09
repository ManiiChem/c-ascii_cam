#pragma once

#include "frame.h"

void ascii_print (const Frame *f, int cols, int rows);
void ascii_fit (const Frame *f, int max_cols, int max_rows, int *out_cols, int *out_rows);