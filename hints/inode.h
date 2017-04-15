#ifndef INODE_H
#define INODE_H

#include <stdio.h>

// TODO: change fields to ones in lecture slides table
typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes for file
    int xtra; // more stuff can go here
} inode;

void   inodes_init(const char* path);
void   pages_free();
void*  pages_get_page(int pnum); // may delete?
inode* pages_get_node(int node_id); // may delete?
int    pages_find_empty();
void   print_node(inode* node);

#endif


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
