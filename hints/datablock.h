#ifndef DATABLOCK_H
#define DATABLOCK_H

void* blocks_init(); // return pointer to first data block
void blocks_free();
void* get_block(int block_num);
int find_empty_block(); // get first empty data block

#endif