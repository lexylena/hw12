
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

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
static char* root_path = 0;

void
storage_init(const char* path)
{
    printf("TODO: Store file system data in: %s\n", path);
    // potentially move this to helper function, have all helper functions in one file
    inode* inodes = inodes_init();
    void* datablocks = blocks_init(path);
    directory_init();
    sb = superblock_init(inodes, datablocks);
    root_path = malloc(sizeof(char));
    *root_path = '/';
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
        if (cur == 0) {
            return 0; // should be something about wrong dir in path?
        }
    	if(lpath->next != 0) {
        	node = get_inode(cur->inode_idx);
        	parent = node->data_blocks[0];
            lpath = lpath->next;
        } else {
    	    return cur;
        }
    }
    return 0;
}

int
get_stat(const char* path, struct stat* st)
{
    // need find_dirent function to find a dirent from given path
    dirent* entry = find_dirent(path);
    if (entry == 0 || streq(root_path, path) == 0) {
        return -1;
    }

    inode* node;
    if (streq(root_path, path)) {
        node = get_inode(0);
    } else {
        node = get_inode(entry->inode_idx);
    }

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
    dirent* entry = find_dirent(path);
    // permissions should be checked before this is called
    // probably with whatever function nufs_access uses
    return read_data(entry->inode_idx);
}

int
mkdir_help(const char* path, mode_t mode) 
{
    char* name;
    char* dir;
    size_t len = strlen(path);
    memcpy(name, path, len);
    memcpy(dir, path, len);
    //TODO: need to link this to an inode somehow
    int idx = get_free_inode();
    inode* node = make_inode(idx, mode);
    dirent* pdirent = find_dirent(dirname(dir));
    inode* pnode = get_inode(pdirent->inode_idx);
    directory* pdir = pnode->data_blocks[0];
    //TODO: needs to return a dirent* so that the make_inode function can mutate it
    directory_put_ent(pdir, basename(name), idx);
    return 0;
}
    
