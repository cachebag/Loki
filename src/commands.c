#include "commands.h"
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <git2.h>

typedef struct {
    char date[20];
    char message[256];
    char hash[41];
    char author[256];
} Commit;

void center_text(int y, const char *text) {
    int x = (COLS - strlen(text)) / 2;
    mvprintw(y, x, "%s", text);
}

void view_history(git_repository *repo) {
    git_revwalk *walker = NULL;
    git_oid oid;
    git_commit *commit = NULL;

    git_revwalk_new(&walker, repo);
    git_revwalk_push_head(walker);
    git_revwalk_sorting(walker, GIT_SORT_TIME);

    Commit commits[1000];
    int count = 0;

    while (!git_revwalk_next(&oid, walker) && count < 1000) {
        git_commit_lookup(&commit, repo, &oid);

        time_t commit_time = git_commit_time(commit);
        struct tm *tm_info = localtime(&commit_time);

        strftime(commits[count].date, sizeof(commits[count].date), "%Y-%m-%d", tm_info);
        snprintf(commits[count].message, sizeof(commits[count].message), "%s", git_commit_message(commit));
        snprintf(commits[count].hash, sizeof(commits[count].hash), "%s", git_oid_tostr_s(&oid));
        snprintf(commits[count].author, sizeof(commits[count].author), "%s", git_commit_author(commit)->name);

        git_commit_free(commit);
        count++;
    }

    git_revwalk_free(walker);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int ch, row = 0;
    while (1) {
        clear();
        center_text(0, "Commit History");
        center_text(1, "Up and Down arrows to move - Ctrl + C to end session");

        for (int i = 0; i < count; i++) {
            if (i == row) attron(A_REVERSE);
            mvprintw(i + 3, 0, "%s -- %s -- %s -- %s", commits[i].date, commits[i].message, commits[i].hash, commits[i].author);
            if (i == row) attroff(A_REVERSE);
        }

        refresh();
        ch = getch();
        switch (ch) {
            case KEY_UP:
                if (row > 0) row--;
                break;
            case KEY_DOWN:
                if (row < count - 1) row++;
                break;
            case 3:  // Ctrl + C to exit
                endwin();
                return;
        }
    }
}

