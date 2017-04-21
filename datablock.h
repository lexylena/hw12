#ifndef DATABLOCK_H
#define DATABLOCK_H

void* blocks_init(); // return pointer to first data block
void blocks_free();
void* get_block(int block_num);
int get_block_num(void* block);
// int truncate_block(void* block, size_t bytes); // truncate block to given bytes, return offset of block (bytes)

#endif