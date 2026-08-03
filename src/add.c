#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "git_object.h"
#include "utils.h"

void what_path(char *path, char *path_rep, BlobsArray *array){
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char hex_out[GIT_SHA1_HEX_SIZE];
            write_git_blob_object(path, hex_out, path_rep, array);
        } else if (S_ISDIR(st.st_mode)) {
            DIR *dir = xopendir(path);
            struct dirent *entry;
            while((entry = readdir(dir)) != NULL){
                if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0 || strcmp(entry->d_name,".git")==0 || strcmp(entry->d_name,".gitignore")==0) continue;
                size_t len = strlen(path) + strlen(entry->d_name) + 2;
                char *buf = xmalloc(len);
                if(path[strlen(path)-1] == '/'){
                    snprintf(buf, len, "%s%s", path, entry->d_name);;
                }else{
                    snprintf(buf, len, "%s/%s", path, entry->d_name);
                }
                what_path(buf, path_rep, array);
                free(buf);
            }
            closedir(dir);
        } else {
            die("This type of file can't be read '%s'", path);
        }
    } else {
        die("Path is not valid '%s'", path);
    }
}

char* find_git_rep(){
    char *buf = getcwd(NULL, 0);
    if ( buf == NULL ) {
        die("Cannot get current working directory path\n" );
        if ( errno == ERANGE ) {
            die("Your path size is too big.\n" );
        }
    }
    while(1){
        struct stat st;
        char* test_path = xmalloc(strlen(buf) + strlen("/.git")+1);
        snprintf(test_path,strlen(buf) + strlen("/.git")+1,"%s/.git",buf);
        if (stat(test_path, &st) == 0 && S_ISDIR(st.st_mode)){
            free(buf);
            return test_path;
        }
        char *p = strrchr(buf, '/');
        if(p == NULL || p - buf == 0){
            free(buf);
            free(test_path);
            die("You are not in a git repository");
        }
        long int last_slash = p - buf;
        buf[last_slash] = '\0';
        free(test_path);
    }
}

void *write_index(BlobsArray array, char* path_rep){
    struct stat st;
    char *index_rep = xmalloc(strlen(path_rep) + strlen("/index")+1);
    snprintf(index_rep, strlen(path_rep) + strlen("/index")+1, "%s/index", path_rep);

    FILE *index_file = xfopen(index_rep, "wb");
    for(int i = 0; i<array.size; i++){
        fwrite(array.index_line[i], 1, sizeof(array.index_line[i])*sizeof(unsigned char *) + 1, index_file);
    }
    fclose(index_file);
}

int main(int nb_of_paths, char **paths){
    char *path_rep = find_git_rep();
    BlobsArray array;
    init_array(&array);
    for(int i = 1; i<nb_of_paths; i++){
        what_path(paths[i], path_rep, &array);
    }
    qsort((&array)->index_line, (&array)->size, sizeof(char *), cmp_str);
    write_index(array, path_rep);
    free(path_rep);
    free(array.blobs);
    return 0;
}
