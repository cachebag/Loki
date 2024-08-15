#include <stdio.h>
#include <git2.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include "commands.h"

void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide the cursor
    start_color();  // Enable colors
    refresh();    // Refresh the screen to ensure a clear start
}

void end_ncurses() {
    endwin();
}

void display_loading_screen() {
    char *frames[] = {
        "    _    ____   _  ____  ",
        " | |    / __ \\ | |/ / | |",
        " | |   | |  | || ' /  | |",
        " | |   | |  | ||  <   | |",
        " | |___| |__| || . \\  | |",
        " |_____|\\____/ |_|\\_\\ |_|"
    };

    char *message = "Loading Loki...";

    int frame_count = sizeof(frames) / sizeof(frames[0]);

    // Define color pairs for flickering effect
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);

    for (int i = 0; i < 5; i++) {
        clear();

        for (int j = 0; j < frame_count; j++) {
            // Randomly decide whether to show or hide each line of the ASCII art
            if (rand() % 2 == 0) {
                int color_pair = (rand() % 3) + 1;
                attron(COLOR_PAIR(color_pair));
                mvprintw(LINES / 2 - frame_count / 2 + j, (COLS - strlen(frames[j])) / 2, "%s", frames[j]);
                attroff(COLOR_PAIR(color_pair));
            }
        }
        mvprintw(LINES / 2 + frame_count / 2 + 1, (COLS - strlen(message)) / 2, "%s", message);
        
        refresh();
        usleep(300000);  // 300ms sleep
    }

    // Hold the final display for 2 seconds
    attron(COLOR_PAIR(1));
    clear();
    for (int j = 0; j < frame_count; j++) {
        mvprintw(LINES / 2 - frame_count / 2 + j, (COLS - strlen(frames[j])) / 2, "%s", frames[j]);
    }
    mvprintw(LINES / 2 + frame_count / 2 + 1, (COLS - strlen(message)) / 2, "%s", message);
    refresh();
    sleep(1);  // Hold for 2 seconds
    attroff(COLOR_PAIR(1));

    clear();
}

int main(int argc, char *argv[]) {
    git_libgit2_init();
    git_repository *repo = NULL;

    if (git_repository_open(&repo, ".")) {
        printf("Could not open repository\n");
        return -1;
    }

    init_ncurses();

    // Display loading screen with animations
    display_loading_screen();

    // Ask user for editor selection
    char selected_editor[256] = {0};
    prompt_editor_selection(selected_editor);

    // Display commit history using ncurses interface
    view_history(repo, selected_editor);

    end_ncurses();

    git_repository_free(repo);
    git_libgit2_shutdown();
    return 0;
}