#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdio.h>
#include "inode.h"


typedef struct superblock {
	int root_idx; // = 0
	// int blocks_count; // = 256; also applies to inodes and bitmaps
	// size_t block_size;
	uint8_t inodes_bitmap[256];
	uint8_t blocks_bitmap[256];
	inode* inodes; // location of first inode
	void* data_blocks; // location of first data block
} superblock;

// TODO: better way to not include superblock as param every time?
superblock* superblock_init(inode* inodes, void* data_blocks);
int get_free_inode();
int get_free_block();
int inode_is_free(int inode_num); // may not be needed?
int block_is_free(int block_num); // may not be needed?




#endif