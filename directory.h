#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_SIZE 64
#define DIR_NAME 48

#include "slist.h"
#include "inode.h"

typedef struct dirent {
    char   name[48];
    int name_len; //length of file name
    int inode_idx; // index in inodes array
    dirent* next;
} dirent;

typedef struct directory {
    dirent* entries; // array of everything in the directory
    inode*  node; // pointer to the inode that is this directory
} directory;

void directory_init(); // create root directory from already created inode at index 2 (always)

directory directory_from_pnum(int pnum);
int directory_lookup_idx(directory dd, const char* name);
int tree_lookup_pnum(const char* path);
directory directory_from_path(const char* path);
int directory_put_ent(directory dd, const char* name, int idx);
int directory_delete(directory dd, const char* name);
slist* directory_list(const char* path);
void print_directory(directory dd);

#endif

