#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_SIZE 64
#define DIR_NAME 48

#include "slist.h"
#include "inode.h"

typedef struct dirent {
    char* name;
    int name_len; //length of file name
    int inode_idx; // index in inodes array
    struct dirent* next;
} dirent;

typedef struct directory {
    dirent* entries; // array of everything in the directory
    inode*  node; // pointer to the inode that is this directory
} directory;

void directory_init(); // create root directory from already created inode at index 2 (always)
dirent* get_dirent(directory* dd, const char* name);
dirent* find_dirent(const char* path);
directory directory_from_pnum(int pnum);
int directory_lookup_idx(directory dd, const char* name);
int tree_lookup_pnum(const char* path);
directory directory_from_path(const char* path);
dirent* directory_put_ent(directory* dd, char* name, int idx);
directory* directory_make(int idx);
void directory_remove_ent(directory* parent, dirent* entry);
int directory_delete(const char* path);
slist* directory_list(const char* path);
void print_directory(directory* dd);


#endif

