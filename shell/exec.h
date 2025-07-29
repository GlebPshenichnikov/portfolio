#ifndef EXEC_H
#define EXEC_H
#include "tree.h"


typedef struct backgrndList {
    int pid;
    struct backgrndList *next;
} intlist;
typedef intlist *bckgrnd;
void do_exec(tree tr);


#endif
