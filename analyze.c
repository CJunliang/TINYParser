//
// Created by liang on 2020/6/23.
//

#include <stdio.h>
#include "analyze.h"
#include "globals.h"
#include "symtab.h"

static int location = 0;

void insertNode(TreeNode *t) {
    switch (t->nodekind) {
        case StmtK:
            switch (t->kind.stmt) {
                case AssignK:
                case ReadK:
                    if (symTabLookUp(t->attr.name) == -1)
                        symTabInsert(t->attr.name, t->lineno, location++);
                    else
                        symTabInsert(t->attr.name, t->lineno, 0);
                    break;
                case WriteK:
                case IfK:
                case RepeatK:
                case WhileK:
                    break;
                case TypeK:
                    while (t->child[0] != NULL) {
                        t = t->child[0];
                        if (symTabLookUp(t->attr.name) == -1)
                            symTabInsert(t->attr.name, t->lineno, location++);
                        else
                            fprintf(listing, "The variable %s has been repeatedly defined at line %d.", t->attr.name,
                                    t->lineno);
                    }
                    break;
                default:
                    fprintf(listing, "This is an error nodeKind statement.");
                    break;
            }
            break;
        case ExpK:
            switch (t->kind.exp) {
                case IdK:
                    if (symTabLookUp(t->attr.name) == -1)
                        symTabInsert(t->attr.name, t->lineno, location++);
                    else
                        symTabInsert(t->attr.name, t->lineno, 0);
                    break;
                case OpK:
                case ConstNumK:
                case ConstStrK:
                case BoolK:
                    break;
                default:
                    fprintf(listing, "This is an error nodeKind expression.");
                    break;
            }
            break;
        default:
            fprintf(listing, "This is an error nodeKind.\n");
            break;
    }
}

void nullProc(TreeNode *t) {}

void typeError(TreeNode *t, char *message) {
    fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
    Error = TRUE;
}

void traverse(TreeNode *t, void (*preProc)(TreeNode *), void (*postProc)(TreeNode *)) {
    if (t != NULL) {
        preProc(t);
        for (int i = 0; i < MAXCHILDREN; i++)
            if (t->child[i] != NULL)
                traverse(t->child[i], preProc, postProc);
        postProc(t);
        if (t->sibling != NULL) {
            traverse(t->sibling, preProc, postProc);
        }
    }
}

/*void checkNode(TreeNode *t) {
    switch (t->nodekind) {
        case StmtK:
            switch (t->kind.stmt) {
                case ReadK:
                case WriteK:
                case IfK:
                case RepeatK:
                case WhileK:
                case TypeK:
                case AssignK:
                    break;
                default:
                    fprintf(listing, "This is an error nodeKind statement.");
                    break;
            }
        case ExpK:
            switch (t->kind.exp) {
                case IdK:
                    t->type = Integer;
                    break;
                case OpK:
                    if ((t->child[0]->type != Integer) || (t->child[1]->type != Integer))
                        typeError(t, "Op applied to non-integer");
                    switch (t->attr.op) {
                        case EQ:
                        case LT:
                        case GT:
                        case LTE:
                        case GTE:
                            t->type = Boolean;
                            break;
                        default:
                            t->type = Integer;
                            break;
                    }
                    break;
                case ConstNumK:
                case ConstStrK:
                    break;
                case BoolK:
                    t->type = Boolean;
                    break;
                default:
                    fprintf(listing, "This is an error nodeKind expression.");
                    break;
            };
        default:
            fprintf(listing, "This is an error nodeKind.\n");
            break;
    }
}*/

void buildSymTab(TreeNode *syntaxTree) {
    traverse(syntaxTree, insertNode, nullProc);
    if (TraceAnalyze) {
        fprintf(listing, "\nSymbol table:\n");
        printSymTab(listing);
    }
}

/*void typeCheck(TreeNode *syntaxTree) {
    traverse(syntaxTree, nullProc, checkNode);
}*/

