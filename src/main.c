#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include <conio.h>
#include "frame.h"
#include "ascii.h"
#include "term.h"
#include "camera.h"

static bool auto_contrast = true;

static volatile bool keep_running = true;

// handle ctrl+c so it exits cleanly
static BOOL WINAPI on_ctrl_c (DWORD type) {
    if (type == CTRL_C_EVENT) {
        keep_running = false;
        return TRUE;
    }

    return FALSE;
}

int main (void) {
    int camera_width, camera_height;

    if (!camera_open(&camera_width, &camera_height)) {
        printf("Error: Could not open camera.\n");
        return EXIT_FAILURE;
    }

    //printf("Camera: %dx%d\n", camera_width, camera_height);

    Frame first_frame;

    // create the frame (whatever resolution it is)
    if (!frame_create(&first_frame, camera_width, camera_height)) {
        printf("Error: Could not create frame.\n");
        return EXIT_FAILURE;
    }

    // enable vt sequences
    if (!enable_ansi_terminal()) {
        printf("Error: Could not enable switch codes for the terminal.\n");
        return EXIT_FAILURE;
    }

    SetConsoleCtrlHandler(on_ctrl_c, TRUE); // register the handler
    printf("\x1b[?25l"); // hide the blinking cursor

    // clr the screen
    printf("\x1b[2J");
    while (keep_running) {
        int key_waiting = _kbhit();

        // react to key presses without the programing freezing
        if (key_waiting) {
            int key = _getch();
            switch (key) {
                case 'q':
                case 'Q':
                    keep_running = false;
                    break;

                case 'c':
                case 'C':
                    auto_contrast = !auto_contrast;
                    break;
            }
        }

        if (!camera_read_frame(&first_frame)) {
            break;
        }

        if (auto_contrast) frame_auto_contrast(&first_frame);

        // getting the exact size of the terminal
        int cols, rows;
        if (!get_size_terminal(&cols, &rows)) {
            printf("Error: Could not get the size of the terminal.\n");
            break;
        }

        int fit_cols, fit_rows;
        ascii_fit(&first_frame, cols, rows - 1, &fit_cols, &fit_rows); // -1 to leave one line for the shell

        // move the cursor home so the terminal doesnt scroll
        printf("\x1b[H");

        // crunching the camera resolution pic down to whatever outputs cleanly in the terminal
        ascii_print(&first_frame, fit_cols, fit_rows);

        //Sleep(33); // ReadSample alr waits for the cameras next picture
        printf("q: quit | c: toggle auto contrast [%s]\x1b[K", (auto_contrast) ? "ENABLED" : "DISABLED");
    }

    // cleanup
    camera_close();
    frame_destroy(&first_frame);
    printf("\x1b[?25h"); // show the cursor again
    restore_terminal();
    printf("\n");

    return EXIT_SUCCESS;
}
