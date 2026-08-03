#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <zlib.h>
#include "git_object.h"
#include "utils.h"

#define GIT_OBJECTS_DIR_PREFIX "/objects/"
#define BLOB_HEADER_FIXED_SIZE 6
#define SHA1_DIGEST_SIZE 20
#define GIT_OBJECT_DIR_PATH_SIZE 12
#define GIT_OBJECT_FILENAME_SIZE 2 * (SHA1_DIGEST_SIZE - 1) + 1


void init_array(BlobsArray *array){
    array->capacity =8;
    array->size=0;
    array->blobs=malloc(array->capacity * sizeof(char *));
    array->files=malloc(array->capacity * sizeof(char *));
    array->index_line=malloc(array->capacity * sizeof(char *));
}

void push_array(BlobsArray *array, unsigned char *blob, unsigned char *file, unsigned char *index_line) {
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        unsigned char **tmp_blob = realloc(array->blobs, array->capacity * sizeof(unsigned char *));
        unsigned char **tmp_file = realloc(array->files, array->capacity * sizeof(unsigned char *));
        unsigned char **tmp_index_line = realloc(array->index_line, array->capacity * sizeof(unsigned char *));
        if (tmp_blob == NULL || tmp_file == NULL || tmp_index_line == NULL) {
            die("realloc error");
        }
        array->blobs = tmp_blob;
        array->files = tmp_file;
        array->index_line = tmp_index_line;
    }
    array->blobs[array->size] = blob;
    array->files[array->size] = file;
    array->index_line[array->size] = index_line;
    array->size++;
}

void print_aray(BlobsArray array){
    for(int i=0; i<array.size; i++){
        printf("index_line : %s\n",(array.index_line)[i]);
    }
}

int cmp_str(const void *a, const void *b) {
    const char * const *sa = a;
    const char * const *sb = b;
    return strcmp(*sa, *sb);
}


static void read_file_binary(const char *path, unsigned char **data, size_t *file_size){
    FILE *f = xfopen(path, "rb");
    fseek(f, 0, SEEK_END);
    *file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    *data = xmalloc(*file_size);
    fread(*data, 1, *file_size, f);
    fclose(f);
}


static unsigned char *dup_repo_relative_path(const char *source_path, const char *repo_path){ // cette fonction force un chemin normalisé (sans "//").
    size_t repo_root_len = strlen(repo_path); 
    const char *relative_path = source_path;

    repo_root_len -= strlen("/.git");
    

    if (strncmp(source_path, repo_path, repo_root_len) == 0 && source_path[repo_root_len] == '/') { //si source_path contient le chemin absolu du repo (sans .git/), alors le chemin relatif du fichier se déuidt facilement
        relative_path = source_path + repo_root_len + 1;
    } else if (strncmp(source_path, "./", 2) == 0) {
        relative_path = source_path + 2;
    }


    unsigned char *stored_path = xmalloc(strlen(relative_path) + 1);
    memcpy(stored_path, relative_path, strlen(relative_path) + 1);
    return stored_path;
}

static void build_blob(const unsigned char *file_data, size_t file_size, unsigned char **blob, size_t *blob_size, BlobsArray* array, const unsigned char *filename){
    int nb_digit = snprintf(NULL, 0, "%zu", file_size);
    unsigned char *size_char = xmalloc(nb_digit + 1);
    size_t filename_len = strlen((const char *)filename);
    size_t blob_header_len = BLOB_HEADER_FIXED_SIZE - 1 + nb_digit;
    *blob_size = BLOB_HEADER_FIXED_SIZE + nb_digit + file_size;
    *blob = xmalloc(*blob_size);
    snprintf(size_char, nb_digit + 1, "%zu", file_size);

    memcpy(*blob, "blob ", BLOB_HEADER_FIXED_SIZE - 1);
    memcpy(*blob + BLOB_HEADER_FIXED_SIZE - 1, size_char, nb_digit);
    (*blob)[BLOB_HEADER_FIXED_SIZE - 1 + nb_digit] = '\0';
    memcpy(*blob + BLOB_HEADER_FIXED_SIZE + nb_digit, file_data, file_size);

    unsigned char *stored_filename = xmalloc(filename_len + 1);
    unsigned char *index_line = xmalloc(filename_len + 1 + blob_header_len + 1);
    memcpy(stored_filename, filename, filename_len + 1);
    memcpy(index_line, stored_filename, filename_len);
    index_line[filename_len] = ' ';
    memcpy(index_line + filename_len + 1, *blob, blob_header_len);
    index_line[filename_len + 1 + blob_header_len] = '\0';
    push_array(array, *blob, stored_filename, index_line);

    free(size_char);
}

static void compute_sha1_hex(const unsigned char *data, size_t len, char hex[GIT_SHA1_HEX_SIZE]){
    unsigned char buf_hash[SHA1_DIGEST_SIZE];
    SHA1(data, len, buf_hash);
    to_hex_str(buf_hash, SHA1_DIGEST_SIZE, hex);
}

static void build_object_paths_from_rep(const char *path_rep, const char *hex, char *dir, char *file_path){
    size_t repo_len = strlen(path_rep);

    memcpy(dir, path_rep, repo_len);
    memcpy(dir + repo_len, GIT_OBJECTS_DIR_PREFIX, GIT_OBJECT_DIR_PATH_SIZE - 3);
    memcpy(dir + repo_len + GIT_OBJECT_DIR_PATH_SIZE - 3, hex, 2);
    dir[repo_len + GIT_OBJECT_DIR_PATH_SIZE - 1] = '\0';

    memcpy(file_path, dir, repo_len + GIT_OBJECT_DIR_PATH_SIZE - 1);
    file_path[repo_len + GIT_OBJECT_DIR_PATH_SIZE - 1] = '/';
    memcpy(file_path + repo_len + GIT_OBJECT_DIR_PATH_SIZE, hex + 2,GIT_OBJECT_FILENAME_SIZE);
}

static void compress_blob_data(const unsigned char *blob, size_t blob_size, unsigned char **compressed, uLong *compressed_size){
    *compressed_size = compressBound(blob_size);
    *compressed = xmalloc(*compressed_size);

    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    
    strm.next_in = (Bytef *)blob;
    strm.avail_in = (uInt)blob_size;

    strm.next_out = *compressed;
    strm.avail_out = (uInt)(*compressed_size);

    deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    deflate(&strm, Z_FINISH);
    *compressed_size = strm.total_out;
    deflateEnd(&strm);

}

static void write_binary_file(const char *file_path, const unsigned char *data, size_t size){
    FILE *blob_file = xfopen(file_path, "wb");
    fwrite(data, 1, size, blob_file);
    fclose(blob_file);
}

static void build_blob_from_file(const char *source_path, unsigned char **blob, size_t *blob_size, BlobsArray* array, const unsigned char *filename){
    size_t file_size = 0;
    unsigned char *data = NULL;
    read_file_binary(source_path, &data, &file_size);
    
    build_blob(data, file_size, blob, blob_size, array, filename);

    free(data);
}

static void store_git_object(const char hex_out[GIT_SHA1_HEX_SIZE],const unsigned char *blob, size_t blob_size,const char *path_rep){
    size_t repo_len = strlen(path_rep);
    char dir[GIT_OBJECT_DIR_PATH_SIZE + repo_len];
    char file_path[GIT_OBJECT_DIR_PATH_SIZE + GIT_OBJECT_FILENAME_SIZE + repo_len + 1];
    uLong compressed_size = 0;
    unsigned char *compressed = NULL;

    build_object_paths_from_rep(path_rep, hex_out, dir, file_path);
    xmkdir(dir, 0777);

    compress_blob_data(blob, blob_size, &compressed, &compressed_size);
    write_binary_file(file_path, (unsigned char *) compressed, compressed_size);

    free(compressed);
}

void write_git_blob_object(const char *source_path, char hex_out[GIT_SHA1_HEX_SIZE],const char *repo_path, BlobsArray* array){
    unsigned char *blob = NULL;
    size_t blob_size = 0;
    unsigned char *relative_path = dup_repo_relative_path(source_path, repo_path);

    build_blob_from_file(source_path, &blob, &blob_size, array, relative_path);
    compute_sha1_hex(blob, blob_size, hex_out);
    store_git_object(hex_out, blob, blob_size, repo_path);

    free(relative_path);
}
