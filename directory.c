#include "inode.h"
#include "directory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

directory* root;

void directory_init() {
    root = (directory*) malloc(sizeof(directory));
    root->entries = 0;
    inode* root_node = make_inode(0, 040755);
    root_node->data_blocks[0] = (void*) root; // point root node to root directory
    root->node = root_node; // set root directory's node
}

dirent* get_dirent(directory* dd, const char* name) {
    dirent* cur = dd->entries;
    while(cur != 0) {
        if(strcmp(cur->name, name)) {
            return cur;
        }
        cur = cur->next;
    }
    return 0;
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

int directory_lookup_idx(directory dd, const char* name) {
    //iterate through entries for dd
    //grab the entry with the name provided
    //return that entry's index
}


directory directory_from_path(const char* path){
    //Read first name in path up until a "/" character
    //Find that entry in the directory
    //If not found, error
    //Else, recurse this function on that entry with the remaining path
} 

int directory_put_ent(directory* dd, char* name, int idx) {
    //Create new entry with name "name" and index idx. 
    //Add this entry to the list of entries for directory dd 
    directory* ndir = (directory*) malloc(sizeof(directory));
    ndir->entries = 0;
    ndir->node = get_inode(idx);
    dirent* ndirent = (dirent*) malloc(sizeof(dirent));
    ndirent->name = name;
    ndirent->name_len = strlen(name);
    ndirent->inode_idx = idx;
    ndirent->next = dd->entries;
    dd->entries = ndirent;
}

void
directory_remove_ent(directory* parent, dirent* entry)
{
    dirent* cur = parent->entries;
    while (cur != 0 && cur->next != 0) {
        if (strcmp(cur->next->name, entry->name)) {
            cur->next = entry->next;
            return;
        }
        cur = cur->next;
    }
}

/*
can't call delete_inode from here because somehow need to also
update bitmap, so if successful, directory_delete returns the 
deleted directory's inode number so that the calling function
(most likely rmdir_help) can call delete_inode
*/
int
directory_delete(const char* path)
{
    slist* lpath = s_split(path, '/');
    inode* node = get_inode(0);
    directory* parent = node->data_blocks[0];
    dirent* cur = 0; // directory to delete
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
    if (S_ISREG(dir_node->mode)) {
        return -1; // path is to file
    }
    if (has_permissions(ret, 2) == 0) {
        return -1; // must have write permissions
    }
    directory* dir = dir_node->data_blocks[0];
    if (dir->entries != 0) {
        return -1; // dir not empty
    }

    directory_remove_ent(parent, cur); // remove entry from parent directory entries list
    free(cur);
    free(dir);

    return ret;
}

slist* directory_list(const char* path) {
    //Not sure what this does
    //Maybe return the entries in the directory given by the path?
    //If so, recurse like in directory_from_path
    //return the dirent* of the last directory
}

void print_directory(directory* dd) {
    //Print the node pointer
    //Iterate through the entries printing their info as it goes
}

