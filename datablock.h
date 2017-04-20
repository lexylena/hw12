#ifndef DATABLOCK_H
#define DATABLOCK_H

void* blocks_init(); // return pointer to first data block
void blocks_free();
void* get_block(int block_num);

#endif