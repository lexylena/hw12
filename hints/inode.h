#ifndef INODE_H
#define INODE_H

#include <stdio.h>

typedef struct inode {
    int mode; // permission & type
    uid_t uid; // user id of the file owner
    int size; // bytes for file
    time_t time; // last accessed time
    time_t ctime; // creation time
    time_t mtime; // last modification time
    time_t dtime; // deletion time
    gid_t gid; // group id of file - may not need?
    int links_count; // # of hard links pointing to this file
    int blocks_count; // # of data blocks allocated to this file
    enum Flags {FILE, DIRECTORY} flags;
    void* data_blocks[15]; // 0-13 direct pointers to data blocks; 14 indirect pointer to directory
} inode;

inode* inodes_init();
void inodes_free();
inode* get_inode(int inode_num);
void update_inode(); // params TBD..
void print_inode(int inode_num);


#endif

