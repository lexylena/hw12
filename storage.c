
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "storage.h"
#include "superblock.h"
#include "directory.h"
#include "inode.h"
#include "datablock.h"
#include "util.h"
#include "slist.h"

typedef struct file_data {
    const char* path;
    int         mode;
    const char* data;
} file_data;

static file_data file_table[] = {
    {"/", 040755, 0},
    {"/hello.txt", 0100644, "hello\n"},
    {0, 0, 0},
};

static superblock* sb = 0;

/*
stat struct
    mode
    inode
    uid
    access time
    ctime
    mtime
    number of links
    size

*/

void
storage_init(const char* path)
{
    printf("TODO: Store file system data in: %s\n", path);
    // potentially move this to helper function, have all helper functions in one file
    inode* inodes = inodes_init();
    void* datablocks = blocks_init(path);
    // directory_init();
    // set return value of superblock_init to some global variable because only one superblock
    sb = superblock_init(inodes, datablocks);
}

dirent*
find_dirent(const char* path)
{
    slist* lpath = s_split(path, '/');
    inode* node = get_inode(0);
    directory* parent = node->data_blocks[0];
    dirent* cur = 0;
    while(lpath != 0) {
        cur = get_dirent(parent, lpath->data);
	if(lpath->next != 0) {
	    node = get_inode(cur->inode_idx);
	    parent = node->data_blocks[0];
            lpath = lpath->next;
        } else {
	    return cur;
        }
    }
    return cur;

    /*
        copy path
        dirname/basename
        copy that
 
        store basename
        add dir names into array until dirname(path) returns '.'
        start at root inode
        find first dir name in inode's directory's dirents
        go to that dirent's inode
        continue down dir names in array...
        find basename in dirents
        return that dirent
    */


int
get_stat(const char* path, struct stat* st)
{
    // need find_dirent function to find a dirent from given path
    dirent* entry = find_dirent(path);
    if (!entry) {
        return -1;
    }

    inode* node = get_inode(entry->inode_idx);

    memset(st, 0, sizeof(struct stat));
    st->st_uid  = getuid();
    st->st_mode = node->mode;
    st->st_size = node->size;
    st->st_nlink = node->links_count;
    st->st_ino = entry->inode_idx;
    st->st_atime = node->time.tv_sec;
    st->st_ctime = node->ctime.tv_sec;
    st->st_mtime = node->mtime.tv_sec;

    return 0;
}

const char*
get_data(const char* path)
{
    /*
        find dirent of file
        get inode_idx
        check if blocks_count > 14 (blocks will contain pointers to other blocks containing actual data)
        go through blocks and get data...
    */
}

