//
// Created by liang on 2020/6/26.
//
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "translate.h"
#include "util.h"
#include "string.h"

#define LENGTH 300
Quadruple quadruples[LENGTH];
static int curIndex = 0;
static int variableNum = 0;

void initRetStruct(RetStruct *retStruct) {
    retStruct->trueList = NULL;
    retStruct->falseList = NULL;
    retStruct->str = NULL;
};

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
    if (list1 != NULL) {
        while (list1->next != NULL)
            list1 = list1->next;
        list1->next = list2;
        return list1;
    }
    return list2;
}

void backPatch(QuaLinkList *list, int target) {
    int index;
    QuaLinkList *temp;
    while (list != NULL) {
        index = list->index;
        quadruples[index].result = intToChar(target);
        temp = list;
        list = list->next;
        free(temp);
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

RetStruct *cGen(TreeNode *tree) {
    RetStruct *ret = NULL;
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
    RetStruct *re;
    QuaLinkList *endList;
    switch (tree->kind.stmt) {
        case IfK:
            re = cGen(tree->child[0]);
            if (tree->child[2] != NULL) {
                backPatch(re->trueList, curIndex);
                cGen(tree->child[1]);
                addQuadruple("jump", NULL, NULL, NULL);
                endList = makeList(curIndex - 1);
//                false = curIndex;
                backPatch(re->falseList, curIndex);
                cGen(tree->child[2]);
                end = curIndex;
                backPatch(endList, end);
            } else {
                backPatch(re->trueList, curIndex);
                cGen(tree->child[1]);
                backPatch(re->falseList, curIndex);
            }
            break;
        case RepeatK:
            true = curIndex;
            cGen(tree->child[0]);
            re = cGen(tree->child[1]);
            backPatch(re->trueList, true);
            backPatch(re->falseList, curIndex);
            break;
        case AssignK:
            re = cGen(tree->child[0]);
            if (tree->child[0]->nodekind == BoolK) {
                addQuadruple(":=", re->str, NULL, tree->attr.name);
            } else if (re->falseList != NULL || re->trueList != NULL) {
                backPatch(re->trueList, curIndex);
                addQuadruple(":=", "true", NULL, tree->attr.name);
                addQuadruple("j", NULL, NULL, intToChar(curIndex + 2));
                backPatch(re->falseList, curIndex);
                addQuadruple(":=", "false", NULL, tree->attr.name);
            } else
                addQuadruple(":=", re->str, NULL, tree->attr.name);
            break;
        case ReadK:
            addQuadruple("IN", NULL, NULL, tree->attr.name);
            break;
        case WriteK:
            re = cGen(tree->child[0]);
            addQuadruple("OUT", NULL, NULL, re->str);
            break;
        case WhileK:
            true = curIndex;
            cGen(tree->child[0]);
            re = cGen(tree->child[1]);
            backPatch(re->trueList, true);
            backPatch(re->falseList, curIndex);
            break;
        case TypeK:
        default:
            break;
    }
}

RetStruct *genExp(TreeNode *tree) {
    TreeNode *node1, *node2;
    int index;
    RetStruct *re1, *re2;
    QuaLinkList *trueList, *falseList;
    RetStruct *retStruct = (RetStruct *) malloc(sizeof(RetStruct));
    initRetStruct(retStruct);
    switch (tree->kind.exp) {
        case OpK:
            node1 = tree->child[0];
            node2 = tree->child[1];
            re1 = cGen(node1);
            index = curIndex;
            re2 = cGen(node2);
            switch (tree->attr.op) {
                case OR:
                    backPatch(re1->falseList, index);
                    retStruct->trueList = merge(re1->trueList, re2->trueList);
                    retStruct->falseList = re2->falseList;
                    break;
                case AND:
                    backPatch(re1->trueList, index);
                    retStruct->trueList = re2->trueList;
                    retStruct->falseList = merge(re1->falseList, re2->falseList);
                    break;
                case NOT:
                    retStruct->trueList = re1->falseList;
                    retStruct->falseList = re1->trueList;
                    break;
                case EQ:
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j=", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case LT:
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j<", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case GT:
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j>", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case LTE:
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j<=", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case GTE:
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j>=", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case PLUS:
                    addQuadruple("plus", re1->str, re2->str, newVar());
                    break;
                case MINUS:
                    addQuadruple("minus", re1->str, re2->str, newVar());
                    break;
                case TIMES:
                    addQuadruple("times", re1->str, re2->str, newVar());
                    break;
                case OVER:
                    addQuadruple("over", re1->str, re2->str, newVar());
                    break;
                default:
                    break;
            }
            retStruct->str = quadruples[curIndex - 1].result;
            if (re1 != NULL)
                free(re1);
            if (re2 != NULL)
                free(re2);
            break;
        case ConstNumK:
            retStruct->str = intToChar(tree->attr.val);
            break;
        case ConstStrK:
            retStruct->str = tree->attr.string;
            break;
        case BoolK:
            if (strcmp(tree->attr.string, "true") == 0) {
                retStruct->trueList = makeList(curIndex);
                retStruct->str = "true";
            } else {
                retStruct->falseList = makeList(curIndex);
                retStruct->str = "false";
            }
            addQuadruple("j", NULL, NULL, NULL);
            break;
        case IdK:
            retStruct->str = tree->attr.name;
            break;
        default:
            break;
    }
    return retStruct;
}