#include "../include/commands.h"
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <git2.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_MESSAGE_LENGTH 50  // Define the maximum length for the commit message

typedef struct {
    char date[20];
    char message[256];
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

void truncate_message(char *message, size_t max_length) {
    if (strlen(message) > max_length) {
        message[max_length - 3] = '.';  // Add ellipsis
        message[max_length - 2] = '.';
        message[max_length - 1] = '.';
        message[max_length] = '\0';  // Null-terminate the string
    }
}

const char *editor_names[] = {"nano", "vim", "nvim", "code", "Other"};
const char *default_paths[] = {"/usr/bin/nano", "/usr/bin/vim", "/usr/bin/nvim", "/usr/bin/code"};

int prompt_editor_selection(char *selected_editor) {
    int ch, row = 0;
    int max_name_length = 0;

    // Find the maximum length of editor names for centering
    for (int i = 0; i < 5; i++) {
        int name_length = strlen(editor_names[i]);
        if (name_length > max_name_length) {
            max_name_length = name_length;
        }
    }

    while (1) {
        clear();
        center_text(0, "Select an Editor");
        for (int i = 0; i < 5; i++) {
            if (i == row) attron(A_REVERSE);
            int x = (COLS - strlen(editor_names[i])) / 2;
            mvprintw(i + 2, x, "%s", editor_names[i]);
            if (i == row) attroff(A_REVERSE);
        }
        refresh();
        ch = getch();
        switch (ch) {
            case KEY_UP:
                if (row > 0) row--;
                break;
            case KEY_DOWN:
                if (row < 4) row++;
                break;
            case 10:  // Enter key
                if (row < 4) {
                    snprintf(selected_editor, 256, "%s", default_paths[row]);
                } else {
                    echo();
                    mvprintw(LINES / 2 + 4, (COLS - 30) / 2, "Enter the full path to your editor: ");
                    getstr(selected_editor);
                    noecho();
                }
                return 0;
        }
    }
}


void open_in_editor(const char *editor_command, const char *commit_hash) {
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

void view_history(git_repository *repo, const char *selected_editor) {
    git_revwalk *walker = NULL;
    git_oid oid;
    git_commit *commit = NULL;

    if (git_revwalk_new(&walker, repo) != 0) {
        fprintf(stderr, "Failed to create revwalker\n");
        return;
    }
    git_revwalk_push_head(walker);
    git_revwalk_sorting(walker, GIT_SORT_TIME);

    int commit_capacity = 1000;
    Commit *commits = malloc(commit_capacity * sizeof(Commit));
    if (commits == NULL) {
        fprintf(stderr, "Failed to allocate memory for commits\n");
        git_revwalk_free(walker);
        return;
    }

    int count = 0;

    while (!git_revwalk_next(&oid, walker)) {
        if (count >= commit_capacity) {
            commit_capacity *= 2;
            commits = realloc(commits, commit_capacity * sizeof(Commit));
            if (commits == NULL) {
                fprintf(stderr, "Failed to reallocate memory for commits\n");
                git_revwalk_free(walker);
                return;
            }
        }

        if (git_commit_lookup(&commit, repo, &oid) < 0) {
            fprintf(stderr, "Failed to lookup commit\n");
            continue;
        }

        time_t commit_time = git_commit_time(commit);
        struct tm *tm_info = localtime(&commit_time);

        strftime(commits[count].date, sizeof(commits[count].date), "%Y-%m-%d", tm_info);
        snprintf(commits[count].message, sizeof(commits[count].message), "%s", git_commit_message(commit));
        sanitize_message(commits[count].message, sizeof(commits[count].message));
        truncate_message(commits[count].message, MAX_MESSAGE_LENGTH);  // Truncate message to fit the desired length
        snprintf(commits[count].full_hash, sizeof(commits[count].full_hash), "%s", git_oid_tostr_s(&oid));
        snprintf(commits[count].author, sizeof(commits[count].author), "%s", git_commit_author(commit)->name);

        git_commit_free(commit);
        count++;
    }

    git_revwalk_free(walker);

    // Initialize color pairs for the different elements
    init_pair(1, COLOR_RED, COLOR_BLACK);    // Hash
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Date
    init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Author
    init_pair(4, COLOR_CYAN, COLOR_BLACK);   // Message

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
            int line_y = i + 3;
            if (i == row) attron(A_REVERSE);

            int date_len = strlen(commits[i].date);
            int msg_len = strlen(commits[i].message);
            int hash_len = 9; // Abbreviated hash length
            int author_len = strlen(commits[i].author);

            int total_len = date_len + msg_len + hash_len + author_len + 10; // 10 for spaces between fields
            int x = (COLS - total_len) / 2;

            // Print date
            attron(COLOR_PAIR(2));
            mvprintw(line_y, x, "%s", commits[i].date);
            attroff(COLOR_PAIR(2));
            x += date_len + 2;

            // Print message
            attron(COLOR_PAIR(4));
            mvprintw(line_y, x, "%s", commits[i].message);
            attroff(COLOR_PAIR(4));
            x += msg_len + 2;

            // Print hash
            attron(COLOR_PAIR(1));
            mvprintw(line_y, x, "%.9s", commits[i].full_hash);
            attroff(COLOR_PAIR(1));
            x += hash_len + 2;

            // Print author
            attron(COLOR_PAIR(3));
            mvprintw(line_y, x, "%s", commits[i].author);
            attroff(COLOR_PAIR(3));

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
                open_in_editor(selected_editor, commits[row].full_hash);
                break;
            case 3:  // Ctrl + C to exit
                endwin();
                free(commits);  // Free the allocated memory
                return;
        }
    }
}
