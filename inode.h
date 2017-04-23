#ifndef INODE_H
#define INODE_H

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

typedef struct inode {
    mode_t mode; // permission & type
    uid_t uid; // user id of the file owner
    int size; // bytes for file
    struct timespec time; // last accessed time
    struct timespec ctime; // creation time
    struct timespec mtime; // last modification time
    int links_count; // # of hard links pointing to this file
    int blocks_count; // # of data blocks allocated to this file
    enum Flags {FILE_FLAG, DIR_FLAG} flags;
    void* data_blocks[15]; // 0-13 direct pointers to data blocks; 14 indirect pointer for overflow
    // if DIRECTORY flag, data_blocks[0] points to directory struct instead of a data block
} inode;

inode* inodes_init();
void inodes_free();
inode* make_inode(int inode_num, mode_t mode);
inode* get_inode(int inode_num);
int change_mode(int inode_num, mode_t mode);
int update_timestamps(int inode_num, const struct timespec ts[2]);
int has_permissions(int inode_num, int permissions);
void print_inode(int inode_num);


#endif

