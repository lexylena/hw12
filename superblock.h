#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdio.h>
#include <stdint.h>
#include "inode.h"


typedef struct superblock {
	int root_idx; // = 0
	uint8_t inodes_bitmap[256];
	uint8_t blocks_bitmap[256];
	inode* inodes; // location of first inode
	void* data_blocks; // location of first data block
} superblock;

// TODO: better way to not include superblock as param every time?
superblock* superblock_init(inode* inodes, void* data_blocks);
int get_free_inode();
int get_free_block();
inode* make_inode(mode_t mode, char* data);


#endif
