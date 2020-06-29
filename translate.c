//
// Created by liang on 2020/6/26.
//
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "translate.h"
#include "util.h"

#define LENGTH 300
Quadruple quadruples[LENGTH];
static int curIndex = 0;
static int variableNum = 0;

char *intToChar(int num) {
    char *str = malloc(10);
    sprintf(str, "%d", num);
    return str;
}

char *newVar() {
    char *str = malloc(10);
    sprintf(str, "t%d", variableNum++);
    return str;
}

void addQuadruple(char *operator, char *arg1, char *arg2, char *result) {
    quadruples[curIndex].operator = copyString(operator);
    quadruples[curIndex].arg1 = copyString(arg1);
    quadruples[curIndex].arg2 = copyString(arg2);
    quadruples[curIndex].result = copyString(result);
    curIndex++;
}

void printQuadruple() {
    for (int i = 0; i < curIndex; i++) {
        fprintf(code, "%3d:  %5s  ", i, quadruples[i].operator);
        if (quadruples[i].arg1 == NULL)
            fprintf(code, "_,");
        else fprintf(code, "%s,", quadruples[i].arg1);
        if (quadruples[i].arg2 == NULL)
            fprintf(code, "_,%s\n", quadruples[i].result);
        else fprintf(code, "%s,%s\n", quadruples[i].arg2, quadruples[i].result);
    }
}

QuaLinkList *makeList(int i) {
    QuaLinkList *list = (QuaLinkList *) malloc(sizeof(QuaLinkList));
    list->index = i;
    list->next = NULL;
    return list;
}

QuaLinkList *merge(QuaLinkList *list1, QuaLinkList *list2) {
    while (list1->next != NULL)
        list1 = list1->next;
    list1->next = list2;
    return list1;
}

void backPatch(QuaLinkList *list, int target) {
    int index;
    while (list != NULL) {
        index = list->index;
        quadruples[index].result = intToChar(target);
        list = list->next;
    }
}

void emitComment(char *c) {
    if (TraceCode)
        fprintf(code, "* %s\n", c);
}

void codeGen(TreeNode *syntaxTree, char *codeFile) {
    char *s = malloc(strlen(codeFile) + 7);
    strcpy(s, "File: ");
    strcat(s, codeFile);
    emitComment("TINY Compilation to TM Code");
    emitComment(s);
    cGen(syntaxTree);
    addQuadruple("HALT", intToChar(0), intToChar(0), intToChar(0));
    printQuadruple();
    emitComment("End of execution.");
}

char *cGen(TreeNode *tree) {
    char *ret = NULL;
    if (tree != NULL) {
        switch (tree->nodekind) {
            case StmtK:
                genStmt(tree);
                break;
            case ExpK:
                ret = genExp(tree);
                break;
            default:
                fprintf(listing, "This is an error nodeKind expression at translate.");
                break;
        }
        cGen(tree->sibling);
    }
    return ret;
}

void genStmt(TreeNode *tree) {
    int true, false, end;
    char *result;
    QuaLinkList *endList, *falseList;
    switch (tree->kind.stmt) {
        case IfK:
            cGen(tree->child[0]);
            true = curIndex + 2;
            result = quadruples[curIndex - 1].result;
            addQuadruple("je", result, "true", intToChar(true));
            addQuadruple("jump", NULL, NULL, NULL);
            falseList = makeList(curIndex - 1);
            cGen(tree->child[1]);
            false = curIndex;
            if (tree->child[2] != NULL) {
                addQuadruple("jump", NULL, NULL, NULL);
                endList = makeList(curIndex - 1);
                false = curIndex;
                backPatch(falseList, false);
                cGen(tree->child[2]);
                end = curIndex;
                backPatch(endList, end);
            } else {
                backPatch(falseList, false);
            }
            break;
        case RepeatK:
            true = curIndex;
            cGen(tree->child[0]);
            cGen(tree->child[1]);
            result = quadruples[curIndex - 1].result;
            addQuadruple("je", result, "true", intToChar(true));
            break;
        case AssignK:
            result = cGen(tree->child[0]);
            addQuadruple(":=", result, NULL, tree->attr.name);
            break;
        case ReadK:
            addQuadruple("IN", NULL, NULL, tree->attr.name);
            break;
        case WriteK:
            result = cGen(tree->child[0]);
            addQuadruple("OUT", NULL, NULL, result);
            break;
        case WhileK:
            true = curIndex;
            cGen(tree->child[0]);
            cGen(tree->child[1]);
            result = quadruples[curIndex - 1].result;
            addQuadruple("je", result, "tree", intToChar(true));
            break;
        case TypeK:
        default:
            break;
    }
}

char *genExp(TreeNode *tree) {
    TreeNode *node1, *node2;
    char *arg1, *arg2;
    char *ret = NULL;
    switch (tree->kind.exp) {
        case OpK:
            node1 = tree->child[0];
            node2 = tree->child[1];
            arg1 = cGen(node1);
            arg2 = cGen(node2);
            switch (tree->attr.op) {
                case OR:
                    addQuadruple("or", arg1, arg2, newVar());
                    break;
                case AND:
                    addQuadruple("and", arg1, arg2, newVar());
                    break;
                case NOT:
                    addQuadruple("not", arg1, NULL, newVar());
                    break;
                case EQ:
                    addQuadruple("=", arg1, arg2, newVar());
                    break;
                case LT:
                    addQuadruple("<", arg1, arg2, newVar());
                    break;
                case GT:
                    addQuadruple(">", arg1, arg2, newVar());
                    break;
                case LTE:
                    addQuadruple("<=", arg1, arg2, newVar());
                    break;
                case GTE:
                    addQuadruple(">=", arg1, arg2, newVar());
                    break;
                case PLUS:
                    addQuadruple("plus", arg1, arg2, newVar());
                    break;
                case MINUS:
                    addQuadruple("minus", arg1, arg2, newVar());
                    break;
                case TIMES:
                    addQuadruple("times", arg1, arg2, newVar());
                    break;
                case OVER:
                    addQuadruple("over", arg1, arg2, newVar());
                    break;
                default:
                    break;
            }
            ret = quadruples[curIndex - 1].result;
            break;
        case ConstNumK:
            ret = intToChar(tree->attr.val);
            break;
        case ConstStrK:
        case BoolK:
            ret = tree->attr.string;
            break;
        case IdK:
            ret = tree->attr.name;
            break;
        default:
            break;
    }
    return ret;
}