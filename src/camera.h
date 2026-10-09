#pragma once

#include <stdbool.h>
#include "frame.h"

//bool camera_list (void);
bool camera_open (int *width, int *height);
void camera_close (void);
bool camera_read_frame (Frame *f);