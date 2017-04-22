#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
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

// returns number of bytes written in overflow blocks
int
allocate_overflow_blocks(int inode_num, void* overflow)
{
	inode* node = get_inode(inode_num);
	void** overflow_block = get_block(get_free_block());
	int overflow_blocks = node->blocks_count - 14;
    int offset;
	for (int ii = 0; ii < overflow_blocks; ++ii) {
		int block_num  = get_free_block();
    	void* dst = get_block(block_num);
        if (ii == overflow_blocks - 1) { // if last block
            offset = (int) strlen(overflow + ii * 4096);
            memcpy(dst, overflow + ii * 4096, offset);
        } else {
        	memcpy(dst, overflow + ii * 4096, 4096);
        }
        *(overflow_block + ii * sizeof(void*)) = dst;
	}

	node->data_blocks[14] = overflow_block;
    if (offset == 0) { // data perfectly takes up x number of blocks
        offset = overflow_blocks * 4096;
    } else {
        offset += (overflow_blocks - 1) * 4096;
    }
    return offset;
}

// returns number of bytes written
int
write_data(int inode_num, const char* data)
{
    inode* node = get_inode(inode_num);
    assert(S_ISREG(node->mode));
    
    node->size = (int) strlen(data);
    node->blocks_count = (int)(node->size / 4096);
    assert(node->blocks_count * 4096 > node->size);
    int offset;

    for (int ii = 0; ii < node->blocks_count && ii < 14; ++ii) {
        int block_num  = get_free_block();
        void* dst = get_block(block_num);
        if (ii == node->blocks_count - 1) { // if last block
            offset = (int) strlen((char*)((void*)data + ii * 4096));
            memcpy(dst, (void*)data + ii * 4096, offset);
        } else {
            memcpy(dst, (void*)data + ii * 4096, 4096);
        }
        node->data_blocks[ii] = dst;
    }

    // handle data that requires more than 14 blocks
    if (node->blocks_count > 14) {
        void* overflow = (void*)data + 14 * 4096;
        offset = allocate_overflow_blocks(inode_num, overflow);
        offset += 14 * 4096;
    } else {
        if (offset == 0) { // data perfectly takes up < 14 blocks
            offset = node->blocks_count * 4096;
        } else {
            offset += (node->blocks_count - 1) * 4096;
        }
    }

    assert(offset == node->size);
    // TODO: update timestamps
    return offset;
}

// deletes inode and all of its data blocks
void
delete_inode(int inode_num)
{
    inode* node = get_inode(inode_num);
    void* block;
    int block_num;

    // delete data blocks of direct pointers
    for (int ii = 0; ii < node->blocks_count && ii < 14; ++ii) {
        block = node->data_blocks[ii];
        memset(block, 0, 4096);
        block_num = get_block_num(block);
        sb->blocks_bitmap[block_num] = 0;
    }

    // data blocks of indirect pointer
    void** overflow_blocks = (void**)node->data_blocks[14];
    if (overflow_blocks) {
        for (int ii = 0; ii < node->blocks_count - 14; ++ii) {
            block = *(overflow_blocks + ii * sizeof(void*));
            memset(block, 0, 4096);
            block_num = get_block_num(block);
            sb->blocks_bitmap[block_num] = 0;
        }
    }

    memset(node, 0, sizeof(inode));
    sb->inodes_bitmap[inode_num] = 0;
}

const char*
read_data(int inode_num)
{
    inode* node = get_inode(inode_num);
    void* buf = malloc(node->size);
    void* block;
    int offset;

    for (int ii = 0; ii < node->blocks_count && ii < 14; ++ii) {
        block = node->data_blocks[ii];
        if (ii == node->blocks_count - 1) { // if last block
            offset = (int) strlen(block);
            memcpy(buf + ii * 4096, block, offset);
        } else {
            memcpy(buf + ii * 4096, block, 4096);
        }
    }

    // get data blocks of indirect pointer
    void** overflow_blocks = (void**)node->data_blocks[14];
    if (overflow_blocks) {
        for (int ii = 0; ii < node->blocks_count - 14; ++ii) {
            block = *(overflow_blocks + ii * sizeof(void*));
            if (ii == node->blocks_count - 15) { // if last block
                offset = (int) strlen(block);
                memcpy(buf + 14 * 4096 + ii * 4096, block, offset);
            } else {
                memcpy(buf + 14 * 4096 + ii * 4096, block, 4096);
            }
        }
    }

    offset += (node->blocks_count - 1) * 4096;
    assert(offset == node->size);
    return (char*)buf;
    
}
