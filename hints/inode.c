
#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#include "inode.h"
#include "util.h"

const int BLOCK_COUNT = 256;
const size_t INODE_SIZE  = sizeof(inode);

static inode* inodes_base =  0;


inode*
inodes_init()
{
    void* inodes_arr = mmap(0, INODE_SIZE * BLOCK_COUNT, PROT_READ | PROT_WRITE, MAP_SHARED, 0, 0);
    assert(inodes_arr != MAP_FAILED);
    inodes_base = (inode*) inodes_arr;
    return inodes_base;
}

void
inodes_free()
{
    int rv = munmap((void*)inodes_base, INODE_SIZE * BLOCK_COUNT);
    assert(rv == 0);
}

inode*
get_inode(int inode_num)
{
    return inodes_base + INODE_SIZE * inode_num;
}

void
print_inode(int inode_num)
{

}

