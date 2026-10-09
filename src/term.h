#pragma once

#include <stdbool.h>

bool get_size_terminal (int *cols, int *rows);
bool enable_ansi_terminal (void);
void restore_terminal (void);