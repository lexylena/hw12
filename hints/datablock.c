#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "datablock.h"
#include "util.h"

const int NUFS_SIZE  = 1024 * 1024; // 1MB
const int BLOCK_COUNT = 256; // 256 blocks, each block 4K = 4096 bytes

static int   pages_fd   = -1;
static void* blocks_base =  0;

void*
blocks_init(const char* path)
{
    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    blocks_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(blocks_base != MAP_FAILED);
    return blocks_base;
}


void
blocks_free()
{
    int rv = munmap(blocks_base, NUFS_SIZE);
    assert(rv == 0);
}

void*
get_block(int block_num)
{
    return blocks_base + 4096 * block_num;
}

// pnode*
// pages_get_node(int node_id)
// {
//     pnode* idx = (pnode*) pages_get_page(0);
//     return &(idx[pnum]);
// }

// int
// blocks_find_empty()
// {
//     int bnum = -1;
//     for (int ii = 2; ii < BLOCK_COUNT; ++ii) {
//         if (0) { // if page is empty
//             bnum = ii;
//             break;
//         }
//     }
//     return bnum;
// }

// void
// print_node(pnode* node)
// {
//     if (node) {
//         printf("node{refs: %d, mode: %04o, size: %d, xtra: %d}\n",
//                node->refs, node->mode, node->size, node->xtra);
//     }
//     else {
//         printf("node{null}\n");
//     }
// }


