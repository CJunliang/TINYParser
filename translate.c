//
// Created by liang on 2020/6/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "translate.h"
#include "util.h"

#define LENGTH 300
Quadruple quadruples[LENGTH];
static int curIndex = 0;
static int variableNum = 0;

/*初始化该结构体*/
void initRetStruct(RetStruct *retStruct) {
    retStruct->trueList = NULL;
    retStruct->falseList = NULL;
    retStruct->str = NULL;
};

/*将int转换成char**/
char *intToChar(int num) {
    char *str = malloc(10);
    sprintf(str, "%d", num);
    return str;
}

/*定义一个新的临时变量*/
char *newVar() {
    char *str = malloc(10);
    sprintf(str, "t%d", variableNum++);
    return str;
}

/*增加一个四元组*/
void addQuadruple(char *operator, char *arg1, char *arg2, char *result) {
    quadruples[curIndex].operator = copyString(operator);
    quadruples[curIndex].arg1 = copyString(arg1);
    quadruples[curIndex].arg2 = copyString(arg2);
    quadruples[curIndex].result = copyString(result);
    curIndex++;
}

/*输出四元组*/
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

/*新增加一个链表来记录要回填的信息*/
QuaLinkList *makeList(int i) {
    QuaLinkList *list = (QuaLinkList *) malloc(sizeof(QuaLinkList));
    list->index = i;
    list->next = NULL;
    return list;
}

/*合并两个链表*/
QuaLinkList *merge(QuaLinkList *list1, QuaLinkList *list2) {
    /*判断list1是否为空，如果为空直接返回list2*/
    if (list1 != NULL) {
        /*遍历到list1最后一个节点，
         * 让该节点的next为list2的首节点 */
        while (list1->next != NULL)
            list1 = list1->next;
        list1->next = list2;
        return list1;
    }
    return list2;
}

/*将链表进行回填*/
void backPatch(QuaLinkList *list, int target) {
    int index;
    QuaLinkList *temp;
    while (list != NULL) {
        index = list->index;
        /*将跳转地址回填到四元式的result中*/
        quadruples[index].result = intToChar(target);
        temp = list;
        list = list->next;
        /*释放节点*/
        free(temp);
    }
}

/*输出注释信息*/
void emitComment(char *c) {
    if (TraceCode)
        fprintf(code, "* %s\n", c);
}

/*遍历语法树来将四元式生成到代码文件*/
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

/*根据节点类型的不同来使用不同的函数来遍历语法树*/
RetStruct *cGen(TreeNode *tree) {
    RetStruct *ret = NULL;
    if (tree != NULL) {
        switch (tree->nodekind) {
            case StmtK:/*语句节点*/
                genStmt(tree);
                break;
            case ExpK:/*表达式节点*/
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

/*遍历语句节点*/
void genStmt(TreeNode *tree) {
    int true, false, end;
    RetStruct *re;
    QuaLinkList *endList;
    switch (tree->kind.stmt) {
        case IfK:
            /*子节点0是布尔表达式*/
            re = cGen(tree->child[0]);
            if (tree->child[2] != NULL) {/*当有else时*/
                /*当前的逻辑地址为布尔表达式为真时的跳转地址，此时回填真链*/
                backPatch(re->trueList, curIndex);
                /*接着遍历子节点1，并且新增一条结尾链表和一条跳转到if语句节点结束的无条件跳转语句，
                 * 因为此时逻辑地址为else的入口地址，所以此时可以回填假链*/
                cGen(tree->child[1]);
                endList = makeList(curIndex);
                addQuadruple("j", NULL, NULL, NULL);
                backPatch(re->falseList, curIndex);
                /*遍历子节点2，回填结尾链表，指示if为真时语句的出口*/
                cGen(tree->child[2]);
                end = curIndex;
                backPatch(endList, end);
            } else {/*当没有else时*/
                /*当前的逻辑地址为布尔表达式为真时的跳转地址，此时回填真链*/
                backPatch(re->trueList, curIndex);
                /*遍历子节点1，此时的逻辑地址为false时的出口，回填假链*/
                cGen(tree->child[1]);
                backPatch(re->falseList, curIndex);
            }
            free(re);
            break;
        case RepeatK:
        case WhileK:
            /*当前的逻辑地址为表达式为真时的入口*/
            true = curIndex;
            /*遍历子节点0，这是语句节点*/
            cGen(tree->child[0]);
            /*遍历子节点1，并且获取它的返回值*/
            re = cGen(tree->child[1]);
            /*用之前记录的表达式为真的逻辑地址回填真链*/
            backPatch(re->trueList, true);
            /*此时逻辑地址为false时的出口地址，回填假链*/
            backPatch(re->falseList, curIndex);
            free(re);
            break;
        case AssignK:
            /*遍历赋值语句的表达式节点*/
            re = cGen(tree->child[0]);
            /*当子节点的节点类型为布尔类型时,直接将值（true或false）赋给变量*/
            if (tree->child[0]->kind.exp == BoolK) {
                /*根据真链和假链是否为空来回填*/
                if (re->trueList != NULL)
                    backPatch(re->trueList, curIndex);
                else
                    backPatch(re->falseList, curIndex);
                /*插入赋值四元式*/
                addQuadruple(":=", re->str, NULL, tree->attr.name);
            } else if (re->falseList != NULL && re->trueList != NULL) {
                /*当真链和假链同时存在时，说明此时为布尔表达式*/
                /*此时的逻辑地址为布尔表达式为true时的出口,新增一条赋值四元式
                 * 将true赋给变量,新增一条无条件跳转语句到赋值语句的末尾*/
                backPatch(re->trueList, curIndex);
                addQuadruple(":=", "true", NULL, tree->attr.name);
                addQuadruple("j", NULL, NULL, intToChar(curIndex + 2));
                /*此时的逻辑地址为表达式为false时的出口，新增一条四元式将false赋给变量*/
                backPatch(re->falseList, curIndex);
                addQuadruple(":=", "false", NULL, tree->attr.name);
            } else
                /*其他语句直接将值赋给变量*/
                addQuadruple(":=", re->str, NULL, tree->attr.name);
            free(re);
            break;
        case ReadK:
            addQuadruple("IN", NULL, NULL, tree->attr.name);
            break;
        case WriteK:
            re = cGen(tree->child[0]);
            addQuadruple("OUT", NULL, NULL, re->str);
            free(re);
            break;
        case TypeK:
        default:
            break;
    }
}

/*遍历表达式节点*/
RetStruct *genExp(TreeNode *tree) {
    TreeNode *node1, *node2;/*两个子节点*/
    int index;/*记录逻辑地址*/
    RetStruct *re1, *re2;/*记录使用cGen函数后的返回值*/
    /*返回的结构*/
    RetStruct *retStruct = (RetStruct *) malloc(sizeof(RetStruct));
    initRetStruct(retStruct);/*初始化返回结构*/
    switch (tree->kind.exp) {
        case OpK:
            /*获取两颗子树*/
            node1 = tree->child[0];
            node2 = tree->child[1];
            /*获取两个子节点的返回信息，并记录第一个子节点执行完后的逻辑地址*/
            re1 = cGen(node1);
            index = curIndex;
            re2 = cGen(node2);
            /*根据节点的操作符选择不同的操作*/
            switch (tree->attr.op) {
                case OR:
                    /*操作符是or，需要回填第一个子节点的假链
                     * 并且将两个子节点的真链合并作为返回结果的真链
                     * 将第二个子节点的假链作为返回结果的假链，*/
                    backPatch(re1->falseList, index);
                    retStruct->trueList = merge(re1->trueList, re2->trueList);
                    retStruct->falseList = re2->falseList;
                    break;
                case AND:
                    /*操作符是and，需要回填第一个子节点的真链
                     * 并且将两个子节点的假链合并作为返回结果的假链
                     * 将第二个子节点的真链作为返回结果的假链*/
                    backPatch(re1->trueList, index);
                    retStruct->trueList = re2->trueList;
                    retStruct->falseList = merge(re1->falseList, re2->falseList);
                    break;
                case NOT:
                    /*操作符是not，将真链作为返回结果的假链，假链作为返回结果的真链*/
                    retStruct->trueList = re1->falseList;
                    retStruct->falseList = re1->trueList;
                    break;
                case EQ:/*操作符是=*/
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j=", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case LT:/*操作符是<*/
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j<", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case GT:/*操作符是>*/
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j>", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case LTE:/*操作符是<=*/
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j<=", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case GTE:/*操作符是>=*/
                    retStruct->trueList = makeList(curIndex);
                    retStruct->falseList = makeList(curIndex + 1);
                    addQuadruple("j>=", re1->str, re2->str, NULL);
                    addQuadruple("j", NULL, NULL, NULL);
                    break;
                case PLUS:
                    /*操作符是+ - * / 新增加一条四元式，以两个子节点的返回字符串
                     * 作为运算对象，将结果存储到新产生的临时变量中
                     * newVar函数的作用就是产生一个从未出现过的临时变量*/
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
            /*获取上一个逻辑地址的结果*/
            retStruct->str = quadruples[curIndex - 1].result;
            /*释放返回节点*/
            if (re1 != NULL)
                free(re1);
            if (re2 != NULL)
                free(re2);
            break;
        case ConstNumK:/*如果是int型数据将int转成字符串后返回*/
            retStruct->str = intToChar(tree->attr.val);
            break;
        case ConstStrK:/*如果是字符串类型直接返回字符串*/
            retStruct->str = tree->attr.string;
            break;
        case BoolK:/*如果是bool类型，需要创建链表以回填，同时还需要以字符串形式返回它的值*/
            if (strcmp(tree->attr.string, "true") == 0) {
                /*创建一条真链*/
                retStruct->trueList = makeList(curIndex);
                retStruct->str = "true";
            } else {
                /*创建一条假链*/
                retStruct->falseList = makeList(curIndex);
                retStruct->str = "false";
            }
            /*添加一条无条件跳转指令*/
            addQuadruple("j", NULL, NULL, NULL);
            break;
        case IdK:/*如果是id类型直接返回id*/
            retStruct->str = tree->attr.name;
            break;
        default:
            break;
    }
    return retStruct;
}