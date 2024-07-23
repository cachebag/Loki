#include "../include/commands.h"
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <git2.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    char date[20];
    char message[256];
    char hash[10];  // Abbreviated hash
    char full_hash[41];  // Full hash for internal use
    char author[256];
} Commit;

void center_text(int y, const char *text) {
    int x = (COLS - strlen(text)) / 2;
    mvprintw(y, x, "%s", text);
}

void sanitize_message(char *message, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (message[i] == '\n' || message[i] == '\r') {
            message[i] = ' ';
        }
    }
}

const char *editor_names[] = {"VSCode", "Vim", "Neovim"};
const char *editor_commands[] = {"/usr/local/bin/code", "/opt/homebrew/bin/vim", "/opt/homebrew/bin/nvim"};

int prompt_editor_selection() {
    int ch, row = 0;
    while (1) {
        clear();
        center_text(0, "Select an Editor");
        for (int i = 0; i < 3; i++) {
            if (i == row) attron(A_REVERSE);
            mvprintw(i + 2, 0, "%s", editor_names[i]);
            if (i == row) attroff(A_REVERSE);
        }
        refresh();
        ch = getch();
        switch (ch) {
            case KEY_UP:
                if (row > 0) row--;
                break;
            case KEY_DOWN:
                if (row < 2) row++;
                break;
            case 10:  // Enter key
                return row;
        }
    }
}

int check_editor_installed(const char *editor_command) {
    char check_command[256];
    snprintf(check_command, sizeof(check_command), "command -v %s", editor_command);
    printf("Running check command: %s\n", check_command); // Debug print
    FILE *fp = popen(check_command, "r");
    if (fp == NULL) {
        return 0;  // popen failed
    }
    int found = (fgetc(fp) != EOF);
    pclose(fp);
    printf("Editor %s found: %d\n", editor_command, found); // Debug print
    return found;
}

void open_in_editor(const char *editor_command, const char *commit_hash) {
    if (!check_editor_installed(editor_command)) {
        endwin();
        printf("Error: %s is not installed.\n", editor_command);
        printf("Press any key to continue...\n");
        getchar();
        return;
    }

    // Stash local changes
    system("git stash -u");

    // Create a temporary worktree
    char temp_dir_template[] = "/tmp/loki_XXXXXX";
    char *temp_dir = mkdtemp(temp_dir_template);
    if (temp_dir == NULL) {
        endwin();
        printf("Error: Could not create temporary directory.\n");
        printf("Press any key to continue...\n");
        getchar();
        return;
    }

    char command[512];
    snprintf(command, sizeof(command), "git worktree add %s %s", temp_dir, commit_hash);
    int result = system(command);
    if (result != 0) {
        endwin();
        printf("Error: Could not create worktree for the commit. Ensure the commit hash is correct.\n");
        printf("Press any key to continue...\n");
        getchar();
        return;
    }

    // Open the editor
    snprintf(command, sizeof(command), "cd %s && %s .", temp_dir, editor_command);
    endwin();  // End ncurses window before running command
    result = system(command);
    if (result != 0) {
        printf("Error: Could not open the editor. Ensure %s is installed and configured correctly.\n", editor_command);
        printf("Press any key to continue...\n");
        getchar();
    }

    // Remove the temporary worktree
    snprintf(command, sizeof(command), "git worktree remove --force %s", temp_dir);
    system(command);

    // Apply the stashed changes
    system("git stash pop");

    initscr();  // Restart ncurses window after command execution
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

void view_history(git_repository *repo) {
    git_revwalk *walker = NULL;
    git_oid oid;
    git_commit *commit = NULL;

    if (git_revwalk_new(&walker, repo) != 0) {
        fprintf(stderr, "Failed to create revwalker\n");
        return;
    }
    git_revwalk_push_head(walker);
    git_revwalk_sorting(walker, GIT_SORT_TIME);

    Commit commits[1000];
    int count = 0;

    FILE *logfile = fopen("commit_log.txt", "w");
    if (!logfile) {
        fprintf(stderr, "Failed to open log file\n");
        git_revwalk_free(walker);
        return;
    }

    while (!git_revwalk_next(&oid, walker) && count < 1000) {
        if (git_commit_lookup(&commit, repo, &oid) < 0) {
            fprintf(logfile, "Failed to lookup commit\n");
            continue;
        }

        time_t commit_time = git_commit_time(commit);
        struct tm *tm_info = localtime(&commit_time);

        strftime(commits[count].date, sizeof(commits[count].date), "%Y-%m-%d", tm_info);
        snprintf(commits[count].message, sizeof(commits[count].message), "%s", git_commit_message(commit));
        sanitize_message(commits[count].message, sizeof(commits[count].message));
        snprintf(commits[count].full_hash, sizeof(commits[count].full_hash), "%s", git_oid_tostr_s(&oid));
        strncpy(commits[count].hash, commits[count].full_hash, 9);  // Abbreviate hash
        commits[count].hash[9] = '\0';  // Null terminate
        snprintf(commits[count].author, sizeof(commits[count].author), "%s", git_commit_author(commit)->name);

        fprintf(logfile, "Date: %s, Message: %s, Full Hash: %s, Author: %s\n", commits[count].date, commits[count].message, commits[count].full_hash, commits[count].author);

        git_commit_free(commit);
        count++;
    }

    fclose(logfile);
    git_revwalk_free(walker);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int ch, row = 0;
    while (1) {
        clear();
        center_text(0, "Commit History");
        center_text(1, "Up and Down arrows to move - Enter to select - Ctrl + C to end session");

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
            case 10:  // Enter key
                endwin();
                int editor_choice = prompt_editor_selection();
                open_in_editor(editor_commands[editor_choice], commits[row].full_hash);
                break;
            case 3:  // Ctrl + C to exit
                endwin();
                return;
        }
    }
}
