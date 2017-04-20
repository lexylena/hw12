#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "superblock.h"
#include "inode.h"
#include "datablock.h"

static superblock* sb = 0;

superblock*
superblock_init(inode* inodes, void* data_blocks)
{
	superblock* superblock = malloc(sizeof(superblock));
	superblock->root_idx = 0;
	// sb->blocks_count = 256;
	// sb->block_size = 4096;
	superblock->inodes = inodes;
	superblock->data_blocks = data_blocks;
	sb = superblock;
	return sb;
}

int
get_free_inode()
{
	int* inodes_bitmap = sb->inodes_bitmap;
	for (int ii = 0; ii < sb->blocks_count; ++ii) {
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
	int* blocks_bitmap = sb->blocks_bitmap;
	for (int ii = 0; ii < sb->blocks_count; ++ii) {
		if (blocks_bitmap[ii] == 0) {
			blocks_bitmap[ii] = 1;
			return ii;
		}
	}
	return -1; // no free blocks
}

int
inode_is_free(int inode_num) // may not be needed?
{
	return sb->inodes_bitmap[inode_num];
}

int
block_is_free(int block_num) // may not be needed?
{
	return sb->blocks_bitmap[block_num];
}

// void
// update_inodes_bitmap(int inode_num, int free)
// {
// 	sb->inodes_bitmap[inode_num] = free;
// 	// print something if already set to free?
// }

// void
// update_blocks_bitmap(int block_num, int free)
// {
// 	sb->blocks_bitmap[block_num] = free;
// 	// print something if already set to free?
// }

void
allocate_overflow_blocks(int inode_num, void* overflow)
{
	inode* node = get_inode(inode_num);
	void* overflow_block = get_block(get_free_block());
	int overflow_blocks = node->blocks_count - 14;
	for (int ii = 0; ii < overflow_blocks; ++ii) {
		int block_num  = get_free_block();
        void* dst = get_block(block_num);
        memcpy(dst, (void*)data + ii * 4096, 4096);
        overflow_block + ii * sizeof(void*) = dst;
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

    clock_gettime(CLOCK_REALTIME, node->ctime);
    node->time = node->ctime;
    node->mtime = 0; // hasn't been modified yet?

    node->links_count = 1;
    if (S_ISREG(mode)) {
        node->flags = FILE;
    } else {
        node->flags = DIRECTORY;
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
