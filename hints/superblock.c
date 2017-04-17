#include <stdio.h>
#include "superblock.h"
#include "inode.h"


superblock*
superblock_init(inode* inodes, void* data_blocks)
{
	superblock* sb = malloc(sizeof(superblock));
	sb->root_idx = 0;
	sb->blocks_count = 256;
	sb->block_size = 4096;
	sb->inodes = inodes;
	sb->data_blocks = data_blocks;
	return sb;
}

int
get_free_inode(superblock* sb)
{
	int* inodes_bitmap = sb->inodes_bitmap;
	for (int ii = 0; ii < sb->blocks_count; ++ii) {
		if (inodes_bitmap[ii] == 0) {
			return ii;
		}
	}
	return -1; // no free inodes
}

int
get_free_block(superblock* sb)
{
	int* blocks_bitmap = sb->blocks_bitmap;
	for (int ii = 0; ii < sb->blocks_count; ++ii) {
		if (blocks_bitmap[ii] == 0) {
			return ii;
		}
	}
	return -1; // no free blocks
}

int
inode_is_free(superblock* sb, int inode_num) // may not be needed?
{
	return sb->inodes_bitmap[inode_num];
}

int
block_is_free(superblock* sb, int block_num) // may not be needed?
{
	return sb->blocks_bitmap[block_num];
}

void
update_inodes_bitmap(superblock* sb, int inode_num, int free)
{
	sb->inodes_bitmap[inode_num] = free;
	// print something if already set to free?
}

void
update_blocks_bitmap(superblock* sb, int block_num, int free)
{
	sb->blocks_bitmap[block_num] = free;
	// print something if already set to free?
}
