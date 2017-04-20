#ifndef INODE_H
#define INODE_H

#include <stdio.h>
#include <sys/time.h>

typedef struct inode {
    int mode; // permission & type
    uid_t uid; // user id of the file owner
    int size; // bytes for file
    struct timespec time; // last accessed time
    struct timespec ctime; // creation time
    struct timespec mtime; // last modification time
    int links_count; // # of hard links pointing to this file
    int blocks_count; // # of data blocks allocated to this file
    enum Flags {FILE, DIRECTORY} flags;
    void* data_blocks[15]; // 0-13 direct pointers to data blocks; 14 indirect pointer for overflow
    // if DIRECTORY flag, data_blocks[0] points to directory struct instead of a data block
} inode;

inode* inodes_init();
void inodes_free();
inode* get_inode(int inode_num);
void print_inode(int inode_num);


#endif

