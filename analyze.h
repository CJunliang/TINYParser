//
// Created by liang on 2020/6/23.
//

#ifndef TINY_ANALYZE_H
#define TINY_ANALYZE_H

#include "globals.h"

void traverse(TreeNode *t, void (*preProc)(TreeNode *), void (*postProc)(TreeNode *));

void insertNode(TreeNode *t);

void nullProc(TreeNode *t);

void buildSymTab(TreeNode *syntaxTree);

void typeCheck(TreeNode *syntaxTree);

#endif //TINY_ANALYZE_H
