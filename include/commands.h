#ifndef COMMANDS_H
#define COMMANDS_H

#include <git2.h>

void view_history(git_repository *repo, const char *selected_editor);
int prompt_editor_selection(char *selected_editor);

#endif // COMMANDS_H
