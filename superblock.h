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

superblock* superblock_init(inode* inodes, void* data_blocks);
int get_free_inode();
int get_free_block();
int write_data(int inode_num, const char* data);
void delete_inode(int inode_num);
char* get_data(int inode_num);


#endif
