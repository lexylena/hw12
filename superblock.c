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
allocate_overflow_blocks(int inode_num, void* overflow, size_t size, off_t offset)
{
    inode* node = get_inode(inode_num);
    void** overflow_block = (void**)(node->data_blocks[14]);
    if (overflow_block == 0) { // all new data blocks for indirect pointers
        assert(offset == 0);
        overflow_block = get_block(get_free_block());
    }

    int overflow_blocks = node->blocks_count - 14;
    int beginning_block = (int)(offset / 4096) - 15;
    for (int ii = beginning_block; ii < overflow_blocks; ++ii) {
        void* dst = get_block(ii);
        if (dst == 0) { // new blocks are being added
            int block_num = get_free_block();
            dst = get_block(block_num);
        }
        if (ii == beginning_block) { // if first block
            memcpy(dst + offset % 4096, overflow, 4096 - (offset % 4096));
            offset += (4096 - (offset % 4096));
        } else if (ii == overflow_blocks - 1) { // if last block
            memcpy(dst, overflow + ii * 4096, size % 4096);
            offset += size % 4096;
        } else {
            memcpy(dst, overflow + ii * 4096, 4096);
            offset += 4096;
        }
        *(overflow_block + ii * sizeof(void*)) = dst;
    }
	node->data_blocks[14] = overflow_block;
    return offset;
}

// returns number of bytes written
int
write_data(int inode_num, const char* data, size_t size, off_t offset)
{
    inode* node = get_inode(inode_num);
    if (S_ISDIR(node->mode)) {
        return -1; // not a file
    }
    int ret;
    node->size += size; // update size
    node->blocks_count += (int) (size / 4096); // update blocks_count
    int beginning_block = (int) (offset / 4096) - 1;

    if (beginning_block < 14) {
        for (int ii = beginning_block; ii < node->blocks_count && ii < 14; ++ii) {
            void* dst = get_block(ii);
            if (dst == 0) { // new blocks are being added
                int block_num = get_free_block();
                dst = get_block(block_num);
            }

            if (ii == beginning_block) { // if first block
                memcpy(dst + offset % 4096, (void*)data, 4096 - (offset % 4096));
                offset += (4096 - (offset % 4096));
            } else if (ii == node->blocks_count - 1) { // if last block
                memcpy(dst, (void*)data + ii * 4096, size % 4096);
                offset += size % 4096;
            } else {
                memcpy(dst, (void*)data + ii * 4096, 4096);
                offset += 4096;
            }
            node->data_blocks[ii] = dst;
        }

        ret = offset;

        // now handle any data that requires more than 14 blocks
        if (node->blocks_count > 14) {
            void* overflow = (void*)data + 14 * 4096;
            // number of bytes written in overflow blocks
            int bytes_left = node->size - offset;
            ret = allocate_overflow_blocks(inode_num, overflow, bytes_left, 0);
        }
    } else { // beginning block is an indirect pointer to a data block
        ret = allocate_overflow_blocks(inode_num, (void*)data, size, offset);
    }

    return ret;
}

// deletes inode and all of its data blocks (if file)
void
delete_inode(int inode_num)
{
    inode* node = get_inode(inode_num);
    void* block;
    int block_num;

    if (S_ISREG(node->mode)) {
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
