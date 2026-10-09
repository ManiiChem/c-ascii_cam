#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "frame.h"
#include "ascii.h"
#include "term.h"
#include "camera.h"

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
        /*
        int center_x = i * 10 % first_frame.width;
        int center_y = first_frame.height / 2;

        // debug function to fill in a gradient to test brightness ramping  
        frame_fill_circle(&first_frame, center_x, center_y);
        */
        if (!camera_read_frame(&first_frame)) {
            break;
        }

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
    }

    // cleanup
    camera_close();
    frame_destroy(&first_frame);
    printf("\x1b[?25h"); // show the cursor again
    restore_terminal();

    return EXIT_SUCCESS;
}
