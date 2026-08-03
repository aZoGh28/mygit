#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void create_folders_from(char *path) {
    char *git_dir = xmalloc(strlen(path) + strlen("/.git/objects") + 1);

    snprintf(git_dir, strlen(path) + strlen("/.git/objects") + 1, "%s/.git", path);
    xmkdir(git_dir, 0777);

    snprintf(git_dir, strlen(path) + strlen("/.git/objects") + 1, "%s/.git/objects", path);
    xmkdir(git_dir, 0777);

    free(git_dir);
}


int main(int nb_of_paths, char **path){
    if(nb_of_paths > 2){
        die("You need to give one path, you gave : %d", nb_of_paths-1);
    }else if (nb_of_paths == 2){
        create_folders_from(path[1]);
    }
    return 0;
}