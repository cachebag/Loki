#include <stdio.h>
#include <git2.h>
#include <ncurses.h>
#include "commands.h"

void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide the cursor
    refresh();    // Refresh the screen to ensure a clear start
}

void end_ncurses() {
    endwin();
}

int main(int argc, char *argv[]) {
    git_libgit2_init();
    git_repository *repo = NULL;

    if (git_repository_open(&repo, ".")) {
        printf("Could not open repository\n");
        return -1;
    }

    init_ncurses();

    // Display commit history using ncurses interface
    view_history(repo);

    end_ncurses();

    git_repository_free(repo);
    git_libgit2_shutdown();
    return 0;
}

