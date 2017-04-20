#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"


// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
    printf("access(%s, %04o)\n", path, mask);
    return 0;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    printf("getattr(%s)\n", path);
    int rv = get_stat(path, st);
    if (rv == -1) {
        return -ENOENT;
    }
    else {
        return 0;
    }
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;

    printf("readdir(%s)\n", path);

    get_stat("/", &st);
    // filler is a callback that adds one item to the result
    // it will return non-zero when the buffer is full
    filler(buf, ".", &st, 0);

    get_stat("/hello.txt", &st);
    filler(buf, "hello.txt", &st, 0);

    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    printf("mknod(%s, %04o)\n", path, mode);
    return -1;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    printf("mkdir(%s)\n", path);
    return -1;
}

int
nufs_unlink(const char *path)
{
    printf("unlink(%s)\n", path);
    return -1;
}

int
nufs_rmdir(const char *path)
{
    /*
    REMOVE EMPTY DIRECTORY ONLY
        - find directory
        - if 0 dirents:
            - clear inode, update inode bitmap
            - remove directory from any dirent lists
            - free directory
        else: return -1?
    */
    printf("rmdir(%s)\n", path);
    return -1;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    /*
    - find FROM
    - nufs_access(TO) -> if TO already exists:
        - check that FROM and TO are both files or both directories
        - if not, return appropriate error message (EISDIR or ENOTDIR)
        - if both are directories:
            - check that FROM is not parent dir of TO; else [EINVAL]
            - check that TO is empty; else [ENOTEMPTY]
        - if bad permissions, [EACCESS]
        - if bad path, [ENOTDIR] or [ENOENT]
    - if good to go...
        - remove TO if it already exists
        - change FROM's dirent->name
        - return 0

    */
    printf("rename(%s => %s)\n", from, to);
    return -1;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    /*
    - find dirent
    - get corresponding inode
    - change mode

    TODO: Add chmod function in inode.c

    */
    printf("chmod(%s, %04o)\n", path, mode);
    return -1;
}

int
nufs_truncate(const char *path, off_t size)
{
    /*
    - find dirent, get inode
    - check permissions and verify that user can write to file (EISDIR if directory)
    - if size > inode->size: add appropriate # of blank blocks to inode->data_blocks and update size
    - else: remove and clear data_blocks if necessary or just remove x bytes from a block; update size

    TODO: add function to check permissions in storage.c?
    TODO: add truncate function in superblock.c
    */
    printf("truncate(%s, %ld bytes)\n", path, size);
    return -1;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    printf("open(%s)\n", path);
    return 0;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    /*
    - find dirent and inode
    - check that it's a file
    - check for read permissions
    - go through data blocks and append size bytes of data to buf?
    - return the number of bytes read as offset?

    TODO: add read function to superblock (or inode?)
    */
    printf("read(%s, %ld bytes, @%ld)\n", path, size, offset);
    const char* data = get_data(path);

    int len = strlen(data) + 1;
    if (size < len) {
        len = size;
    }

    strlcpy(buf, data, len);
    return len;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    /*
    - find dirent and inode
    - check that it's a file
    - check for write permissions
    - add data? or write over existing data? 
    - either way ^, get blocks, copy data from buf to blocks
    - update inode->data_blocks
    - return the new size of the data because that's the new offset?

    TODO: add write function to superblock (helper function in datablock?)
    */
    printf("write(%s, %ld bytes, @%ld)\n", path, size, offset);
    return -1;
}

int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    /*
    (NOT SURE IF PARAMS ARE CORRECT FOR THIS...)
    - find dirent, get inode
    - check for permissions or proper access ?
    - update timestamps (ts[0] = inode->time, ts[1] = inode->mtime)
    - return 0

    TODO: add utimens function to inode
    */
    return -1;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 5);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

