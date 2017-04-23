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
    /*
    - path is the file path, mask is mode (exists, read_ok, write_ok, execute_ok)
    - Check if a file exists using find_dirent
    - If not, return error code (-1)
    - If found, check its inode for permissions based on what mask asks for
    - If it has requested permissions, return 0
    - If not, return -1
    */
    int rv = access_help(path, mask);
    if (rv == -1) {
	return -1;
    }

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
    /*
    1.Find the first directory entry following the given offset.
    2.Optionally, create a struct stat that describes the file as for getattr (but FUSE only looks at st_ino and the file-type bits of st_mode).
    3.Call the filler function with arguments of buf, the null-terminated filename, the address of your struct stat (or NULL if you have none), and the offset of the next     directory entry.
    4. If filler returns nonzero, or if there are no more files, return 0.
    5. Find the next file in the directory.
    6. Go back to step 2.
    */
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
    /*
    -If the mode designates a file, make a file
    -If the mode designates a directory, call mkdir
    -rdev is only used if mode is a block device or character device
    */
    int rv = mknod_help(path, mode);
    assert (rv == 0);
    printf("mknod(%s, %04o)\n", path, mode);
    return 0;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    /*
    TODO: Create function that creates a directory (maybe modify directory_init?)
    (Unsure about this ordering)
    -Make inode for the directory
    -Make directory structure with that inode
    -Navigate to parent directory and make a dirent with the directory's name and inode index

     -OR-
     Make directory helper func (needs to make struct and dirent)
     Pass directory to make_inode

    Im putting all of this in a helper in storage.c
    */

    int rv = mkdir_help(path, mode);
    assert (rv == 0);
    printf("mkdir(%s)\n", path);
    return 0;
}

int
nufs_unlink(const char *path)
{
    /*
    Removes the given file
    -Navigate to the dirent using find_dirent
    -Get inode
    -If file, zero out the inode data
        -free dirent, return 0
    */
    int unlink_help(path);
    printf("unlink(%s)\n", path);
    return -1;
}

int
nufs_rmdir(const char *path)
{
    printf("rmdir(%s)\n", path);
    return rmdir_help(path);
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
    // assumes only one user, so no checking permissions for chmod...?
    printf("chmod(%s, %04o)\n", path, mode);
    return chmod_help(path, mode);
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
    return open_help(path);
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
    printf("write(%s, %ld bytes, @%ld)\n", path, size, offset);
    return write_help(path, buf, size, offset);
}

int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    printf("utimens(accessed:%lld.%.9ld, modified:%lld.%.9ld)\n",
        (long long)ts[0].tv_sec, ts[0].tv_nsec,
        (long long)ts[1].tv_sec, ts[1].tv_nsec);
    utimens_help(path, ts);
    return 0;
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
    assert(argc > 2);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

