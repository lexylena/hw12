#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "superblock.h"
#include "inode.h"
#include "datablock.h"

static superblock* sb = 0;

superblock*
superblock_init(inode* inodes, void* data_blocks)
{
	superblock* superblock = malloc(sizeof(superblock));
	superblock->root_idx = 0;
	superblock->inodes = inodes;
	superblock->data_blocks = data_blocks;
	sb = superblock;
	return sb;
}

int
get_free_inode()
{
	uint8_t* inodes_bitmap = sb->inodes_bitmap;
	for (int ii = 0; ii < 256; ++ii) {
		if (inodes_bitmap[ii] == 0) {
			inodes_bitmap[ii] = 1;
			return ii;
		}
	}
	return -1; // no free inodes
}

int
get_free_block()
{
	uint8_t* blocks_bitmap = sb->blocks_bitmap;
	for (int ii = 0; ii < 256; ++ii) {
		if (blocks_bitmap[ii] == 0) {
			blocks_bitmap[ii] = 1;
			return ii;
		}
	}
	return -1; // no free blocks
}

void
allocate_overflow_blocks(int inode_num, void* overflow)
{
	inode* node = get_inode(inode_num);
	void** overflow_block = get_block(get_free_block());
	int overflow_blocks = node->blocks_count - 14;
	for (int ii = 0; ii < overflow_blocks; ++ii) {
		int block_num  = get_free_block();
        	void* dst = get_block(block_num);
        	memcpy(dst, overflow + ii * 4096, 4096);
        	*(overflow_block + ii * sizeof(void*)) = dst;
	}

	node->data_blocks[14] = overflow_block;
}

inode*
make_inode(mode_t mode, char* data)
{
    int inode_num = get_free_inode();
    inode* node = get_inode(inode_num);
    node->mode = (int)mode;
    node->uid = getuid();

    struct timespec* ts = malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, ts);
    node->ctime.tv_sec = ts->tv_sec;
    node->ctime.tv_nsec = ts->tv_nsec;
    node->time.tv_sec = ts->tv_sec;
    node->time.tv_nsec = ts->tv_nsec;
    free(ts);
    //node->mtime isn't set because hasn't been modified yet?

    node->links_count = 1;
    if (S_ISREG(mode)) {
        node->flags = FILE_FLAG;
    } else {
        node->flags = DIR_FLAG;
    }

    node->size = (int) strlen(data);
    node->blocks_count = (int)(node->size / 4096);
    assert(node->blocks_count * 4096 > node->size);

    for (int ii = 0; ii < node->blocks_count && ii < 14; ++ii) {
    	int block_num  = get_free_block();
        void* dst = get_block(block_num);
        memcpy(dst, (void*)data + ii * 4096, 4096);
        node->data_blocks[ii] = dst;
    }

    // handle data that requires more than 14 blocks
    if (node->blocks_count > 14) {
        void* overflow = (void*)data + 14 * 4096;
        allocate_overflow_blocks(inode_num, overflow);
    }

    return node;
}
