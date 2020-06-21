//
// Created by liang on 2020/6/20.
//

#ifndef TINY_SYMTAB_H
#define TINY_SYMTAB_H

#define SIZE 211
typedef struct LineListRec {
    int lineno;
    struct LineListRec *next;
} *LineList;

typedef struct BucketListRec {
    char *name;
    LineList lines;
    int memloc;
    struct BucketListRec *next;
} *BucketList;

static BucketList hashTable[SIZE];


void symTabInsert(char *name, int lineno, int loc);

int symTabLookUp(char *name);

void printSymTab(FILE *listing);

#endif //TINY_SYMTAB_H
