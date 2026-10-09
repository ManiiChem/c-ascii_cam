#include <windows.h>
#include "term.h"

static DWORD original_mode;

bool get_size_terminal (int *cols, int *rows) {
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return false;
    }

    // right/bottom is the max val, left/top is them minimum, you want the buffer u can see, sometimes the minimum doesnt start at 0
    // also the +1 is because rn the index starts at 0 but we need it to start at 1
    *cols = info.srWindow.Right - info.srWindow.Left + 1;
    *rows = info.srWindow.Bottom - info.srWindow.Top + 1;

    return true;
}

bool enable_ansi_terminal (void) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    if (!GetConsoleMode(out, &mode)) {
        return false;
    }

    original_mode = mode;

    return SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}

void restore_terminal (void) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleMode(out, original_mode);
}