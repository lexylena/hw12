#ifndef SLIST_H
#define SLIST_H

typedef struct slist {
    char* data;
    int   refs;
    struct slist* next;
} slist;

void print_slist(slist* list);
slist* s_cons(const char* text, slist* rest);
void   s_free(slist* xs);
slist* s_split(const char* text, char delim);

#endif

