#include "inode.h"
#include "directory.h"
#include <stdio.h>
#include <stdlib.h>

directory* root;

void directory_init() {
    root = (directory*) malloc(sizeof(directory));
    root->entries = 0;
    root->node = get_inode(0);
}

directory directory_from_pnum(int pnum) {
    //dont think we need this
}

int directory_lookup_idx(directory dd, const char* name) {
    //iterate through entries for dd
    //grab the entry with the name provided
    //return that entry's index
}

int tree_lookup_pnum(const char* path) {
    //Honestly idk what this should do

}

directory directory_from_path(const char* path){
    //Read first name in path up until a "/" character
    //Find that entry in the directory
    //If not found, error
    //Else, recurse this function on that entry with the remaining path
} 

int directory_put_ent(directory* dd, const char* name, int idx) {
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

int directory_delete(directory dd, const char* name) {
    //iterate through entries for dd
    //grab the entry with the name provided
    //remove that entry from the list
    //return some int for success
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

dirent* get_dirent(directory* dd, const char* name) {
    dirent* cur = dd->entries;
    while(cur != 0) {
	if(cur->name == name) {
	    return cur;
	} else if (cur->next != 0) {
	    cur = cur->next;
	} else {
	    return 0;
	}
    }
    return 0;
}
