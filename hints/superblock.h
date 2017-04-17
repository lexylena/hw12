#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdio.h>
#include "inode.h"

const int COUNT = 256;

typedef struct superblock {
	int root_idx; // = 2
	int blocks_count; // = 256; also applies to inodes and bitmaps
	size_t block_size;
	int inodes_bitmap[COUNT];
	int blocks_bitmap[COUNT];
	inode inodes[COUNT]; // location of first inode
	void data_blocks[COUNT]; // location of first data block
	/* TODO: how to init/allocate fixed size of blocks if void pointers? 
			if datablock struct, it would just be like a wrapper...
	*/
} superblock;

/* TODO: 

typedef struct superblock {
	int* inodes;
	int* blocks;
	inode* actual inodes;
	blocks* datablocks;
	add # for all of those...^^^
	index of root inodes?;
} superblock;

*/

// TODO: better way to not include superblock as param every time?
superblock* init_superblock();
int get_free_inode(superblock* sb);
int get_free_block(superblock* sb);
int inode_is_free(superblock* sb, int inode_num); // may not be needed?
int block_is_free(superblock* sb, int block_num); // may not be needed?
void update_inodes_bitmap(superblock* sb, int inode_num, int free);
void update_blocks_bitmap(superblock* sb, int block_num, int free);




#endif