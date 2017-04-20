
#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "inode.h"
#include "util.h"

const size_t INODE_SIZE  = sizeof(inode);

static inode* inodes_base =  0;


inode*
inodes_init()
{
    void* inodes_arr = mmap(0, INODE_SIZE * 256, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (inodes_arr == MAP_FAILED) {
        perror("ERROR");
    }
    inodes_base = (inode*) inodes_arr;
    return inodes_base;
}

void
inodes_free()
{
    int rv = munmap((void*)inodes_base, INODE_SIZE * 256);
    assert(rv == 0);
}

inode*
get_inode(int inode_num)
{
    return inodes_base + INODE_SIZE * inode_num;
}

// change inode->mode
// return 0 on success
// return -1 if flags don't match new mode
int
change_mode(int inode_num, mode_t mode)
{
    inode* node = get_inode(inode_num);
    if (S_ISREG(mode) != S_ISREG(node->mode)) { // not both files or directories
        return -1;
    }

    node->mode = mode;
    return 0;
}

int
update_timestamps(int inode_num, const struct timespec ts[2])
{
    inode* node = get_inode(inode_num);
    node->time = ts[0];
    node->mtime = ts[1];
    return 0;
}

void
print_inode(int inode_num)
{

}

