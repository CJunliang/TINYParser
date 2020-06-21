//
// Created by liang on 2020/6/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

static int hash(const char *key) {
    int i = 0;
    unsigned int h = 0, g;
    while (key[i] != '\0') {
        h = (h << 4u) + key[i];
        if ((g = h & 0xf0000000)) {
            h = h ^ (g >> 24u);
            h = h ^ g;
        }
    }
    return (int) h % SIZE;
}

void symTabInsert(char *name, int lineno, int loc) {
    int index = hash(name);
    BucketList bucketList = hashTable[index];
    /*寻找正确的表项*/
    while ((bucketList != NULL) && (strcmp(name, bucketList->name) != 0))
        bucketList = bucketList->next;
    if (bucketList == NULL) {
        bucketList = (BucketList) malloc(sizeof(struct BucketListRec));
        bucketList->name = name;
        bucketList->lines = (LineList) malloc(sizeof(struct LineListRec));
        bucketList->lines->lineno = lineno;
        bucketList->lines->next = NULL;
        bucketList->memloc = loc;
        bucketList->next = NULL;
        hashTable[index] = bucketList;
    } else {
        LineList lineList = bucketList->lines;
        while (lineList->next != NULL)
            lineList = lineList->next;
        lineList->next = (LineList) malloc(sizeof(struct LineListRec));
        lineList->next->lineno = lineno;
        lineList->next->next = NULL;
    }
}

int symTabLookUp(char *name) {
    int index = hash(name);
    BucketList bucketList = hashTable[index];
    while ((bucketList != NULL) && (strcmp(name, bucketList->name) != 0))
        bucketList = bucketList->next;
    if (bucketList == NULL)
        return -1;
    return bucketList->memloc;
}

void printSymTab(FILE *listing) {
    int i;
    fprintf(listing, "Variable Name  Location   Line Numbers\n");
    fprintf(listing, "-------------  --------   ------------\n");
    for (i = 0; i < SIZE; ++i) {
        if (hashTable[i] != NULL) {
            BucketList bucketList = hashTable[i];
            while (bucketList != NULL) {
                LineList lineList = bucketList->lines;
                fprintf(listing, "%-14s ", bucketList->name);
                fprintf(listing, "%-8d  ", bucketList->memloc);
                while (lineList != NULL) {
                    fprintf(listing, "%4d ", lineList->lineno);
                    lineList = lineList->next;
                }
                fprintf(listing, "\n");
                bucketList = bucketList->next;
            }
        }
    }
}