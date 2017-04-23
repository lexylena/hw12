#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void storage_init(const char* path);
int         get_stat(const char* path, struct stat* st);
const char* get_data(const char* path);
int mkdir_help(const char* path, mode_t mode);
int mknod_help(const char* path, mode_t mode);
int rmdir_help(const char* path);
int rename_help(const char* from, const char* to, int to_exists);
int chmod_help(const char* path, mode_t mode);
int write_help(const char* path, const char* buf, size_t size, off_t offset);
int utimens_help(const char* path, const struct timespec ts[2]);
int access_help(const char* path, int mask);
int unlink_help(const char* path);
slist* dir_list_help(const char* path);
#endif
