
#define _GNU_SOURCE
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
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
make_inode(mode_t mode)
{
    int inode_num = get_free_inode();
    inode* node = get_inode(inode_num);
    node->mode = (int)mode;
    node->uid = getuid();
    node->size = 0;

    struct timespec* ts = malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, ts);
    node->ctime.tv_sec = ts->tv_sec;
    node->ctime.tv_nsec = ts->tv_nsec;
    node->time.tv_sec = ts->tv_sec;
    node->time.tv_nsec = ts->tv_nsec;
    free(ts);
    //node->mtime isn't set because hasn't been modified yet?

    node->links_count = 1;
    node->blocks_count = 0;

    if (S_ISREG(mode)) {
        node->flags = FILE_FLAG;
    } else {
        node->flags = DIR_FLAG;
    }

    return node;
}

inode*
get_inode(int inode_num)
{
    return inodes_base + INODE_SIZE * inode_num;
}

int
get_inode_num(inode* node)
{
    uint diff = (uint)node - (uint)inodes_base;
    assert(diff % INODE_SIZE == 0);
    return ((uint)node - (uint)inodes_base) / INODE_SIZE;
}

/* 
change inode->mode
return 0 on success
return -1 if flags don't match new mode
*/
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

