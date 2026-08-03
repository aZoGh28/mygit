#ifndef GIT_OBJECT_H
#define GIT_OBJECT_H

#define GIT_SHA1_HEX_SIZE 41

typedef struct {
    unsigned char **blobs;
    unsigned char **files;
    unsigned char **index_line;
    int size;
    int capacity;
} BlobsArray;



void init_array(BlobsArray *array);

void push_array(BlobsArray *array, unsigned char *blob,unsigned char *file, unsigned char *index_line);

void print_aray(BlobsArray array);

int cmp_str(const void *a, const void *b);

void write_git_blob_object(const char *source_path,char hex_out[GIT_SHA1_HEX_SIZE],const char *repo_path, BlobsArray* head);

#endif
