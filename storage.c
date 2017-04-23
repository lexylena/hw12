
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>

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

int
get_stat(const char* path, struct stat* st)
{
    inode* node;
    int ino;
    if (streq(root_path, path) == 0) {
        dirent* entry = find_dirent(path);
        if (entry == 0) {
            return -1;
        }
        ino = entry->inode_idx;
        node = get_inode(ino);
    } else {
        ino = 0;
        node = get_inode(0);
    }

    memset(st, 0, sizeof(struct stat));
    st->st_uid  = getuid();
    st->st_mode = node->mode;
    st->st_size = node->size;
    st->st_nlink = node->links_count;
    st->st_ino = ino;
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
    if (find_dirent(path) != 0) 
    {
        return -1;
    }
    char* name;
    char* dir;
    size_t len = strlen(path);
    memcpy(name, path, len);
    memcpy(dir, path, len);
    int idx = get_free_inode();
    inode* node = make_inode(idx, mode);
    dirent* pdirent = find_dirent(dirname(dir));
    inode* pnode = get_inode(pdirent->inode_idx);
    directory* pdir = pnode->data_blocks[0];
    directory_make(idx);
    directory_put_ent(pdir, basename(name), idx);
    return 0;
}

int 
mknod_help(const char* path, mode_t mode) 
{
    if (find_dirent(path) != 0) 
    {
        return -1;
    }
    char* name;
    char* dir;
    size_t len = strlen(path);
    memcpy(name, path, len);
    memcpy(dir, path, len);
    int idx = get_free_inode();
    inode* node = make_inode(idx, mode);
    dirent* pdirent = find_dirent(dirname(dir));
    inode* pnode = get_inode(pdirent->inode_idx);
    directory* pdir = pnode->data_blocks[0];
    directory_put_ent(pdir, basename(name), idx);
    return 0;
}    

int
rmdir_help(const char* path)
{
    int inode_num = directory_delete(path);
    if (inode_num == -1) {
        return -1; // somehow log errors
    }

    delete_inode(inode_num);
    return 0;
}

int
rename_help(const char* from, const char* to, int to_exists)
{
    dirent* old_entry = find_dirent(from);
    inode* node = get_inode(old_entry->inode_idx);
    if (has_permissions(old_entry->inode_idx, 2) == 0) {
        return -1;
    }

    if (to_exists) {
        dirent* new_entry = find_dirent(to);
        inode* to_node = get_inode(new_entry->inode_idx);
        if (S_ISREG(node->mode) != S_ISREG(to_node->mode)) {
            return -1;
        }
        if (S_ISDIR(node->mode)) { // renaming directores
            directory* old_dir = (directory*)node->data_blocks[0];
            if (get_dirent(old_dir, to) != 0) {
                return -1;
            }
            rmdir_help(to);
        } else { // renaming files
            char* to_copy;
            memcpy(to_copy, to, strlen(to));
            dirname(to_copy);
            dirent* pdirent = find_dirent(to_copy);
            directory* pdir = (directory*)(get_inode(pdirent->inode_idx))->data_blocks[0];
            directory_remove_ent(pdir, new_entry);
            delete_inode(new_entry->inode_idx);
        }
    }
    old_entry->name = (char*) to;
    old_entry->name_len = strlen(to);
    return 0;
}

int
chmod_help(const char* path, mode_t mode)
{
    dirent* entry = find_dirent(path);
    return change_mode(entry->inode_idx, mode);
}

int
write_help(const char* path, const char* buf, size_t size, off_t offset)
{
    dirent* entry = find_dirent(path);
    int ret = write_data(entry->inode_idx, buf, size, offset);
    assert(ret == size + offset);
    return 0;
}

int
utimens_help(const char* path, const struct timespec ts[2])
{
    dirent* entry = find_dirent(path);
    update_timestamps(entry->inode_idx, ts);
    return 0;
}

int
access_help(const char* path, int mask) 
{
    if(find_dirent(path) == 0) {
	return -1;
    }
    dirent* entry = find_dirent(path);
    inode* enode = get_inode(entry->inode_idx);
    if(enode->mode == mask) {
	return 0;
    } else {
	return -1;
    }
}

int
unlink_help(const char* path)
{

    slist* lpath = s_split(path, '/');
    inode* node = get_inode(0);
    directory* parent = node->data_blocks[0];
    dirent* cur = 0; // file to delete
    while(lpath != 0) {
        cur = get_dirent(parent, lpath->data);
        if (cur == 0) {
            return -1; // no directory found
        }
        if(lpath->next != 0) {
            node = get_inode(cur->inode_idx);
            parent = node->data_blocks[0];
            lpath = lpath->next;
        } else {
            break;
        }
    }

    int ret = cur->inode_idx;
    inode* dir_node = get_inode(cur->inode_idx);
    if (!(S_ISREG(dir_node->mode))) {
        return -1; // path is not to file
    }
    if (has_permissions(ret, 2) == 0) {
        return -1; // must have write permissions
    }
   
    
    //dirent* pdir = find_dirent(dirname(path));
    //inode* pnode = get_inode(pdir->inode_idx);
    //directory* parent = (directory*)pnode->data_blocks[0];
    directory_remove_ent(parent, cur); // remove entry from parent directory entries list
    free(cur);

    delete_inode(ret);
    return 0;
}

    