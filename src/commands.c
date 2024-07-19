#include <git2.h>
#include <stdio.h>
#include "commands.h"

void view_history(git_repository *repo) {
    git_revwalk *walker = NULL;
    git_oid oid;
    git_commit *commit = NULL;

    git_revwalk_new(&walker, repo);
    git_revwalk_push_head(walker);
    git_revwalk_sorting(walker, GIT_SORT_TIME);

    while (!git_revwalk_next(&oid, walker)) {
        git_commit_lookup(&commit, repo, &oid);
        printf("Commit: %s\n", git_commit_message(commit));
        git_commit_free(commit);
    }

    git_revwalk_free(walker);
}

