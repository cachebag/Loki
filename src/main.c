#include <stdio.h>
#include <git2.h>
#include <commands.h>

int main(int argc, char *argv[]) {
    git_libgit2_init();
    git_repository *repo = NULL;

    if (git_repository_open(&repo, ".")) {
        printf("Could not open repository\n");
        return -1;
    }

    view_history(repo);

    git_repository_free(repo);
    git_libgit2_shutdown();
    return 0;
}

